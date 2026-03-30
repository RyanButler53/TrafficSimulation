#include <unordered_map>
#include <ranges>
#include <utility>
#include <algorithm>
#include <map>
#include <numeric>

#include "highway.hpp"


CpuHighway::CpuHighway(size_t numLanes, std::vector<FlowGenerator> flows):flowGenerators_{flows}, 
    lanes_{std::make_shared<std::set<Car>[]>(numLanes)}, nLanes_{numLanes}{}


std::optional<std::string>CpuHighway::getAccelerationCache(std::vector<std::unordered_map<double, double>>& accelerationCache, double dt){
    accelerationCache.resize(nLanes_);
    for (size_t i : std::views::iota(0UL, nLanes_)){
        std::set<Car>& cars = lanes_[i];
        std::unordered_map<double, double>& cache = accelerationCache[i];

        // Iterate over all cars in the lane, calculate acceleration and add to cache
        std::set<Car>::const_iterator current = cars.begin();
        std::set<Car>::const_iterator next = ++cars.begin();
        while (next != cars.end()){
            const Car& lead = *next;
            std::expected<double, std::string> a = current->acceleration(lead, dt);
            // TODO Clean up with monads? 
            if (!a.has_value()) {
                return std::make_optional(a.error());
            } else {
                cache.insert({current->getPosition(), a.value()});
            }
            // Next pair
            ++current;
            ++next;
        }
        cache.insert({current->getPosition(), current->acceleration(dt)});
    }
    return std::nullopt;
}

void CpuHighway::moveVehicles(std::vector<std::unordered_map<double, double>>& accelerationCache, double dt){
    for (size_t i : std::views::iota(0UL, nLanes_)){
        std::unordered_map<double, double>& cache = accelerationCache[i];
        for (auto car : lanes_[i]){
            double a = cache[car.getPosition()];
            car.update(a, dt);
        }
    }
}

std::expected<void, std::string> CpuHighway::update(double dt){
    
    // Cache of accelerations of each car given that no lane changing occurs. x->a
    
    // Phase 1: Iterate over all cars in all lanes to fill acceleration cache
    std::vector<std::unordered_map<double, double>> accelerationCache;
    if (auto result = getAccelerationCache(accelerationCache, dt)){
        return std::unexpected(result.value());
    }


    // Phase 2: Run Lane Change algorithm to find all possible lane changes. 
    std::vector<std::tuple<std::set<Car>::const_iterator, int, int>> laneChanges;
    for (int ilane : std::views::iota(0, static_cast<int>(nLanes_))){
        std::set<Car>& cars = lanes_[ilane];

        // Lane changes are only possible if there are more than 3 cars in the lane. 
        if (cars.size() < 3 or nLanes_ == 1){ continue;}
        
        // Iterate over all cars in the lane, find f_hat and l_hat and compute utility. 
        std::set<Car>::const_iterator follow = cars.begin();
        std::set<Car>::const_iterator alpha = ++cars.begin();
        std::set<Car>::const_iterator lead = ++(++cars.begin());

        while (lead != cars.end()){
        
            double left = std::numeric_limits<double>::min();
            double right = std::numeric_limits<double>::min();

            // Calculate the lane change:
            auto calculateUtility = [&](int curLane, int newLane, double bias){
                double a_alpha = accelerationCache[curLane].at(alpha->getPosition());
                std::set<Car>::const_iterator l_hat = std::upper_bound(lanes_[newLane].begin(), lanes_[newLane].end(), *alpha);
                double a_alphaChange = alpha->acceleration(*l_hat, dt).value_or(std::numeric_limits<double>::min());
                // Current Follower Terms
                double a_f = accelerationCache[curLane].at(follow->getPosition());
                double a_fChange = follow->acceleration(*lead, dt).value_or(std::numeric_limits<double>::min()); // should never be -1000
                
                // New Follower Terms
                std::set<Car>::const_iterator f_hat = std::lower_bound(lanes_[newLane].begin(), lanes_[newLane].end(), *alpha);
                double a_fHat = accelerationCache[newLane].at(f_hat->getPosition());
                double a_fHatChange = f_hat->acceleration(*alpha, dt).value_or(std::numeric_limits<double>::min());
    
                // Safety Criterion: If either acceleration is more negative than -bMax, return minimum
                double estimatedMaxBraking = alpha -> braking(); // use same value for all 
                std::array<double, 3> accelerations = {a_alphaChange, a_fChange, a_fHatChange};
                if (*std::ranges::min_element(accelerations) < estimatedMaxBraking){
                    return std::numeric_limits<double>::min();
                }

                // Safety criterion satisfied, apply incentive criterion. 
                double p = alpha->politeness();
                return (a_alphaChange - a_alpha) +  p * (a_fHatChange - a_fHat + a_fChange - a_f) + bias;
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
                laneChanges.push_back({alpha ,ilane, ilane-1});
            } else if (left > right and left > changeThreshold_){
                laneChanges.push_back({alpha, ilane, ilane+1});
            } 
        }
    }

    // Phase 3: Move the lane changed cars to their new lanes
    for (auto [carIter, oldlane, newlane] : laneChanges){
        lanes_[newlane].insert(lanes_[oldlane].extract(carIter));
    } 

    // Phase 4: Compute the acceleration for all cars in all the lanes. 
    accelerationCache.clear();
    if (auto result = getAccelerationCache(accelerationCache, dt)){
        return std::unexpected(result.value());
    }

    // Phase 5: Apply acceleration to each vehicle 
    moveVehicles(accelerationCache, dt);

    // Phase 6: Generate flow for each lane 
    for (size_t i : std::views::iota(0UL, nLanes_)){
        flowGenerators_[i].generateFlow(dt).transform([this, i](Car c){lanes_[i].insert(c);});
    }
    return {};
}

