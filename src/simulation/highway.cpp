#include <unordered_map>
#include <ranges>
#include <utility>
#include <algorithm>
#include <map>
#include <ranges>
#include <numeric>
#include <print>

#include "sim/highway.hpp"


CpuHighway::CpuHighway(size_t numLanes, std::vector<FlowGenerator> flows, double roadEnd):flowGenerators_{flows}, 
    lanes_{std::vector<std::set<Car>>(numLanes)}, nLanes_{numLanes}, roadEnd_{roadEnd}{}


std::optional<std::string>CpuHighway::getAccelerationCache(std::vector<std::unordered_map<double, double>>& accelerationCache, double dt){

    for (std::set<Car>& cars : lanes_){
        accelerationCache.push_back({});
        std::unordered_map<double, double>& cache = accelerationCache.back();
        if (cars.empty()){
            continue;
        }

        // Iterate over all cars in the lane, calculate acceleration and add to cache
        std::set<Car>::const_iterator current = cars.begin();
        std::set<Car>::const_iterator next = ++cars.begin();
        while (next != cars.end()){
            const Car& lead = *next;
            std::expected<double, std::string> a = current->acceleration(lead, dt);
            // TODO Clean up with monads? 
            if (!a.has_value()) {
                std::cout << "Error calculating acceleraton! "  << a.error() << std::endl;
                return std::make_optional(a.error());
            } else {
                cache.insert({current->getPosition(), a.value()});
            }
            // Next pair
            ++current;
            ++next;
        }
        // Lead car
        cache.insert({current->getPosition(), current->acceleration(dt)});
    }
    return std::nullopt;
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
        // Remove all cars past roadEnd_
        Car end{0, roadEnd_, 0, 0, 0, {}};
        auto it = lanes_[i].upper_bound(end);
        lanes_[i].erase(it, lanes_[i].end());
    }
}

std::expected<std::vector<CarData>, std::string> CpuHighway::update(double dt){
    
    // Cache of accelerations of each car given that no lane changing occurs. x->a
    
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

        while (lead != cars.end()){
        
            // Utility value for left or right lane change
            double left = -1000.0;
            double right = -1000.0;

            // Calculate the lane change:
            auto calculateUtility = [&](int curLane, int newLane, double bias){
                double a_alpha = accelerationCache[curLane].at(alpha->getPosition());
                const Car& a = *alpha;
                std::set<Car>::const_iterator l_hat = std::upper_bound(lanes_[newLane].begin(), lanes_[newLane].end(), a);
                std::expected<double, std::string> a_alphaChange;
                if (l_hat == lanes_[newLane].end()){ // No new lead car. Acceleration is free road acceleration
                    a_alphaChange = alpha->acceleration(dt);
                } else {
                    a_alphaChange = alpha->acceleration(*l_hat, dt);
                }
                // Current Follower Terms
                double a_f = accelerationCache[curLane].at(follow->getPosition());
                std::expected<double, std::string> a_fChange = follow->acceleration(*lead, dt);
                
                // New Follower Terms
                std::set<Car>::const_iterator f_hat = std::lower_bound(lanes_[newLane].begin(), lanes_[newLane].end(), *alpha);
                double a_fHat = 0;
                std::expected<double, std::string> a_fHatChange = 0.0;
                // If the new follower is non existent, can never gaurantee the safety criterion is satisfied
                if (f_hat != lanes_[newLane].begin()) {
                    --f_hat;
                } else {
                    return -1000.0;
                }

                // Ensure that the new follower is actually following alpha and that the new follower exists
                if (f_hat != lanes_[newLane].end() && f_hat->getPosition() < a.getPosition()){
                    a_fHat = accelerationCache[newLane].at(f_hat->getPosition());
                    a_fHatChange = f_hat->acceleration(*alpha, dt).value_or(-1000);
                }

                if (!a_alphaChange || ! a_fChange || !a_fHatChange){
                    return -1000.0;
                }
    
                // Safety Criterion: If either acceleration is more negative than -bMax, return minimum
                double estimatedMaxBraking = alpha -> braking(); // use same value for all 
                std::array<double, 3> accelerations = {*a_alphaChange, *a_fChange, *a_fHatChange};
                if (*std::ranges::min_element(accelerations) < estimatedMaxBraking){
                    return -1000.0;
                }

                // Safety criterion satisfied, apply incentive criterion. 
                double p = alpha->politeness();
                return (*a_alphaChange - a_alpha) +  p * (*a_fHatChange - a_fHat + *a_fChange - a_f) + bias;
            };

            // If in leftmost lane, don't do left lane change computation 
            if (ilane == nLanes_ - 1){
                right = calculateUtility(ilane, ilane - 1, a_bias);
            } else if (ilane == 0) { // rightmost lane, only change left
                left = calculateUtility(ilane, ilane + 1, -1 * a_bias);
            } else { // both are available. 
                right = calculateUtility(ilane, ilane - 1, a_bias);
                left = calculateUtility(ilane, ilane + 1, -1 * a_bias);
            }
            // Map the utility to old lane, new lane
            if (right > left and right > changeThreshold_){
                // std::println(" R Lane change: Car{} (x = {:.2f}, v = {:.2f}) from lane {} to {}. Utilities: R:{:.3f} L:{:.3f} Stay:{:.3f}", alpha->getId(), alpha->getPosition(), alpha->getVelocity(), ilane, ilane - 1, right, left, changeThreshold_);
                laneChanges.push_back({alpha ,ilane, ilane-1});
            } else if (left > right and left > changeThreshold_){
                // std::println(" L Lane change: Car{} (x = {:.2f}, v = {:.2f}) from lane {} to {}. Utilities: R:{:.3f} L:{:.3f} Stay:{:.3f}", alpha->getId(), alpha->getPosition(), alpha->getVelocity(),ilane, ilane + 1, right, left, changeThreshold_);
                laneChanges.push_back({alpha, ilane, ilane+1});
            }

            ++lead;
            ++alpha;
            ++follow;
        }
    }

    // Phase 3: Move the lane changed cars to their new lanes
    for (auto [carIter, oldlane, newlane] : laneChanges){
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
    for (size_t i : std::views::iota(0UL, nLanes_)){
        double lastcar = roadEnd_;
        if (!lanes_[i].empty()){
            auto car = lanes_[i].begin();
            lastcar = car->getPosition() - car->getLength(); 
        }
        auto c = flowGenerators_[i].generateFlow(dt);
        if (c){ 
            lanes_[i].insert(*c);
            newCars.push_back({c->data()});
        }

    }
    return newCars;
}

std::vector<CarSnapshot> CpuHighway::log(double t){

    // Calculate number of cars
    size_t ncars = 0;
    for (auto ilane : std::views::iota(0UL, nLanes_)){
        ncars += lanes_[ilane].size();
    }

    // Allocate data memory up front and collect data. 
    std::vector<CarSnapshot> data;
    data.reserve(ncars);
    for (auto ilane : std::views::iota(0UL, nLanes_)){
        for (auto car : lanes_[ilane]){
            data.push_back(car.snapshot(t, ilane));
        }
    }
    return data;
}
