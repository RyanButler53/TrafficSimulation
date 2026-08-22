#include <unordered_map>
#include <ranges>
#include <utility>
#include <algorithm>
#include <map>
#include <ranges>
#include <numeric>
#include <print>

#include "sim/highway.hpp"


CpuHighway::CpuHighway(size_t numLanes, std::vector<std::pair<size_t, FlowGenerator>>flows,
                       std::unique_ptr<LaneInfo> lanes):flowGenerators_{flows}, 
    lanes_{std::vector<std::set<Car>>(numLanes)},laneInfo_{std::move(lanes)}, nLanes_{numLanes}{}


std::optional<std::string> CpuHighway::getAccelerationCache(std::vector<std::unordered_map<double, double>>& accelerationCache, double dt){

    for (std::set<Car>& cars : lanes_){
        accelerationCache.push_back({});
        std::unordered_map<double, double>& cache = accelerationCache.back();
        if (cars.empty()){
            continue;
        }

        // Iterate over all cars in the lane, calculate acceleration and add to cache
        std::set<Car>::const_iterator current = cars.begin();
        std::set<Car>::const_iterator next = ++cars.begin();
        // Store the end of the current segment to check if the lead car is in another road segment
        double endOfCurrentSegment = laneInfo_->endOfSegment(current->getPosition(), accelerationCache.size() -1).value();
        while (next != cars.end()){
            Car lead = *next;
            // Set lead car to still car at end of segment and update the end of current segment to 
            if (lead.getPosition() > endOfCurrentSegment){
                lead = Car::stoppedCar(endOfCurrentSegment);
                if (auto curSegment = laneInfo_->endOfSegment(next->getPosition(), accelerationCache.size() - 1)){
                    endOfCurrentSegment = curSegment.value();
                } else {
                    return std::make_optional(std::format("Error calculating acceleration cacle for car {}. Lead car {} is out of bounds:{}", current->getId(), lead.getId(), curSegment.error()));
                }
            }
            std::expected<void, std::string> insert = current->acceleration(lead, dt).transform([&cache, current](double accel){
                cache.insert({current->getPosition(), accel});
            });
            if (!insert.has_value()) {
                return std::make_optional(insert.error());
            }
            // Next pair
            ++current;
            ++next;
        }
        // Lead car
        std::expected<double, std::string> a = leadCarAcceleration(*current, accelerationCache.size() -1, dt);
        if (!a){
            return std::make_optional(a.error());
        }

        cache.insert({current->getPosition(), a.value()});
    }
    return std::nullopt;
}

std::expected<double, std::string> CpuHighway::leadCarAcceleration(const Car& c, size_t ilane, double dt) {
    std::expected<double, std::string> laneEnd = laneInfo_->endOfLane(ilane);
    std::expected<double, std::string> endOfCurrentSegment = laneInfo_->endOfSegment(c.getPosition(), ilane);
    if (!laneEnd or !endOfCurrentSegment){
        return laneEnd;
    }
    // True free road acceleration only occurs if the car is in the last segment that ends at the end of the road
    if (*endOfCurrentSegment == *laneEnd && *endOfCurrentSegment == laneInfo_->endOfRoad()){
        return c.acceleration(dt);
    } else { // Otherwise a stopped car is in front. 
        return c.acceleration(Car::stoppedCar(*endOfCurrentSegment), dt);
    }
}

void CpuHighway::moveVehicles(std::vector<std::unordered_map<double, double>>& accelerationCache, double dt){
    for (size_t i : std::views::iota(0UL, nLanes_)){
        std::unordered_map<double, double>& cache = accelerationCache[i];
        for (auto it = lanes_[i].rbegin(); it != lanes_[i].rend();++it){
            auto& car = *it;
            double a = cache[car.getPosition()];
            auto nh = lanes_[i].extract(car);
            nh.value().update(a, dt);
            lanes_[i].insert(std::move(nh));
        }
        std::erase_if(lanes_[i], [this, i](const Car& c) {
            return !laneInfo_->laneValid(c.getPosition(), i);
        });
    }
}

std::expected<std::vector<CarData>, std::string> CpuHighway::update(double dt){
        
    // Phase 1: Iterate over all cars in all lanes to fill acceleration cache
    std::vector<std::unordered_map<double, double>> accelerationCache;
    auto result = getAccelerationCache(accelerationCache, dt);
    if (result){
        return std::unexpected(result.value());
    }

    // Phase 2: Run Lane Change algorithm to find all possible lane changes. 
    std::vector<std::tuple<std::set<Car>::const_iterator, int, int>> laneChanges;
    for (int ilane : std::views::iota(0, static_cast<int>(nLanes_))){
        std::set<Car>& cars = lanes_[ilane];

        // Lane changes are only possible if there are more than 3 cars in the lane. 
        if (cars.size() < 3 or nLanes_ == 1){ continue;}
        
        // Iterate over all cars in the lane, find f_hat and l_hat and compute utility for all possible lane changes
        std::set<Car>::const_iterator follow = cars.begin();
        std::set<Car>::const_iterator alpha = ++cars.begin();
        std::set<Car>::const_iterator lead = ++(++cars.begin());

        // End of segment Alpha is in. 
        double endOfCurrentSegment = laneInfo_->endOfSegment(alpha->getPosition(),ilane).value();

        while (alpha != cars.end()){
        
            // Utility value for left or right lane change
            double left = -1000.0;
            double right = -1000.0;


            // Find the "lead car" for the timestep. L
            std::unique_ptr<Car> leadCar;
            if (lead != cars.end()){
                if (lead->getPosition() > endOfCurrentSegment){
                    leadCar = std::make_unique<Car>(Car::stoppedCar(endOfCurrentSegment));
                    endOfCurrentSegment = laneInfo_->endOfSegment(lead->getPosition(), ilane).value();
                } else {
                    leadCar = std::make_unique<Car>(*lead);
                }
            } else { // lead == cars.end() implies alpha is the front car in the 
                if (endOfCurrentSegment == laneInfo_->endOfRoad()){
                    ++alpha;
                    continue;
                } else {
                    leadCar = std::make_unique<Car>(Car::stoppedCar(endOfCurrentSegment));
                }
            }

            // Calculate the lane change:
            auto calculateUtility = [&](int curLane, int newLane, double bias){
                double a_alpha = accelerationCache[curLane].at(alpha->getPosition());
                const Car& a = *alpha;
                std::set<Car>::const_iterator l_hat = std::upper_bound(lanes_[newLane].begin(), lanes_[newLane].end(), a);
                std::expected<double, std::string> a_alphaChange;
                if (l_hat == lanes_[newLane].end()){ // No new lead car. Acceleration is "free road" acceleration
                    a_alphaChange = leadCarAcceleration(*alpha, newLane, dt);
                } else {
                    // If new leader is past the end of the segment alpha would move into, compare to stopped car
                    double newEndOfSegment = laneInfo_->endOfSegment(a.getPosition(), newLane).value();
                    if (l_hat->getPosition() > newEndOfSegment ){
                        a_alphaChange = alpha->acceleration(Car::stoppedCar(newEndOfSegment), dt);
                    } else {
                        if (alpha->getId() == 5){
                            // std::println("breakpoint");
                        }
                        a_alphaChange = alpha->acceleration(*l_hat, dt);
                    }
                }
                // Current Follower Terms
                double a_f = accelerationCache[curLane].at(follow->getPosition());
                // Acceleration for the follower if the lane change happens. Uses the lead car (stopped car or real car)
                std::expected<double, std::string> a_fChange = follow->acceleration(*leadCar, dt);
                
                // New Follower Terms
                std::set<Car>::const_iterator f_hat = std::lower_bound(lanes_[newLane].begin(), lanes_[newLane].end(), *alpha);
                double a_fHat = 0;
                std::expected<double, std::string> a_fHatChange = 0.0;

                if (f_hat != lanes_[newLane].begin()) {
                    --f_hat;
                    // Ensure that the new follower is actually following alpha and that the new follower exists
                    if (f_hat != lanes_[newLane].end() && f_hat->getPosition() < a.getPosition()){
                        a_fHat = accelerationCache[newLane].at(f_hat->getPosition());
                        a_fHatChange = f_hat->acceleration(*alpha, dt).value_or(-1000);
                    }
                }

                if (!a_alphaChange || ! a_fChange || !a_fHatChange){
                    if (alpha->getId() == 16){
                        std::println("Something is not defined: {}, {}, {}",!a_alphaChange, ! a_fChange, !a_fHatChange);
                    }
                    return -1000.0;
                }
    
                // Safety Criterion: If either acceleration is more negative than -bMax, return minimum
                double estimatedMaxBraking = alpha -> braking(); // use same value for all 
                std::array<double, 3> accelerations = {*a_alphaChange, *a_fChange, *a_fHatChange};
                if (*std::ranges::min_element(accelerations) < estimatedMaxBraking){
                    if (alpha->getId() == 16){
                        std::println("Safety criterion not passed: {}, {}, {} at x = ",*a_alphaChange, *a_fChange, *a_fHatChange, alpha->getPosition());
                    }
                    return -1000.0;
                }

                // Safety criterion satisfied, apply incentive criterion. 
                double p = alpha->politeness();
                return (*a_alphaChange - a_alpha) +  p * (*a_fHatChange - a_fHat + *a_fChange - a_f) + bias;
            };

            // If in leftmost lane, don't do left lane change computation 
            if (laneInfo_->laneValid(alpha->getPosition(), ilane - 1)){
                right = calculateUtility(ilane, ilane - 1, laneInfo_->calculateBias(alpha->getPosition(), ilane, Direction::RIGHT ));
            } 
            if (laneInfo_->laneValid(alpha->getPosition(), ilane + 1)) { // rightmost lane, only change left
                left = calculateUtility(ilane, ilane + 1,  laneInfo_->calculateBias(alpha->getPosition(), ilane, Direction::LEFT));
                if (alpha->getId() == 16){
                    std::println("Utility to go left: {}", left);
                }
            }
            // Map the utility to old lane, new lane
            if (right > left and right > changeThreshold_){
                laneChanges.push_back({alpha ,ilane, ilane-1});
            } else if (left > right and left > changeThreshold_){
                if (alpha->getId() == 16){
                    std::println("Going left!");
                }
                laneChanges.push_back({alpha, ilane, ilane+1});
            }

            if (lead != cars.end()) {++lead;};
            ++alpha;
            ++follow;
        }
    }

    // Phase 3: Move the lane changed cars to their new lanes
    for (auto [carIter, oldlane, newlane] : laneChanges){
        // if (carIter->getId() == 0){
        //     std::println("Car 0 moving from {} to {}", oldlane, newlane);
        // }
        auto nh = lanes_[oldlane].extract(carIter);
        lanes_[newlane].insert(std::move(nh));
    }

    // Phase 4: Compute the acceleration for all cars in all the lanes. 
    accelerationCache.clear();
    result = getAccelerationCache(accelerationCache, dt);
    if (result){
        return std::unexpected(result.value());
    }

    // Phase 5: Apply acceleration to each vehicle 
    moveVehicles(accelerationCache, dt);

    // Phase 6: Generate flow for each lane 
    std::vector<CarData> newCars;
    for (auto& [l, gen] : flowGenerators_){
        std::set<Car>::iterator nextCar = lanes_[l].upper_bound(Car::compare(gen.position()));
        std::optional<Car> c = std::nullopt;
        if (nextCar == lanes_[l].end()){
            c = gen.generateFlow(dt);
        } else {
            c = gen.generateFlow(dt, nextCar->getRearPosition(), nextCar->getVelocity());
        }

        if (c){ 
            lanes_[l].insert(*c);
            newCars.push_back({c->data()});
        }
    }
    return newCars;
}

void CpuHighway::log(double t, std::vector<CarSnapshot>& snapshots){
    for (auto ilane : std::views::iota(0UL, nLanes_)){
        for (auto car : lanes_[ilane]){
            snapshots.push_back(car.snapshot(t, ilane));
        }
    }
}
