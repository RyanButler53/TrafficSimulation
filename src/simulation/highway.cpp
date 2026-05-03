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
                if (a.value() < current->braking()){
                    std::println("Calculated Acceleration less than max braking. If no lane change in next timestep, crash");
                }
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
        for (auto it = lanes_[i].begin(); it != lanes_[i].end();++it){
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

std::expected<void, std::string> CpuHighway::update(double dt){
    
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
            double left = std::numeric_limits<double>::min();
            double right = std::numeric_limits<double>::min();

            // Calculate the lane change:
            auto calculateUtility = [&](int curLane, int newLane, double bias){
                double a_alpha = accelerationCache[curLane].at(alpha->getPosition());
                const Car& a = *alpha;
                std::set<Car>::const_iterator l_hat = std::upper_bound(lanes_[newLane].begin(), lanes_[newLane].end(), a);
                double a_alphaChange;
                if (l_hat == lanes_[newLane].end()){ // No new lead car. Acceleration is free road acceleration
                    a_alphaChange = alpha->acceleration(dt);
                } else {
                    alpha->acceleration(*l_hat, dt).value_or(std::numeric_limits<double>::min());
                }
                // Current Follower Terms
                double a_f = accelerationCache[curLane].at(follow->getPosition());
                double a_fChange = follow->acceleration(*lead, dt).value_or(std::numeric_limits<double>::min()); // should never be -1000
                
                // New Follower Terms
                std::set<Car>::const_iterator f_hat = std::lower_bound(lanes_[newLane].begin(), lanes_[newLane].end(), *alpha);
                if (f_hat != lanes_[newLane].begin()){ --f_hat;}

                // Ensure that the new follower is actually following alpha and that the new follower exists
                double a_fHat = 0;
                double a_fHatChange = 0;
                if (f_hat != lanes_[newLane].end() && f_hat->getPosition() < a.getPosition()){
                    a_fHat = accelerationCache[newLane].at(f_hat->getPosition());
                    a_fHatChange = f_hat->acceleration(*alpha, dt).value_or(std::numeric_limits<double>::min());
                }
    
                // Safety Criterion: If either acceleration is more negative than -bMax, return minimum
                double estimatedMaxBraking = alpha -> braking(); // use same value for all 
                std::array<double, 3> accelerations = {a_alphaChange, a_fChange, a_fHatChange};
                if (*std::ranges::min_element(accelerations) < estimatedMaxBraking){
                    std::println("Min element: {:.3f}", *std::ranges::min_element(accelerations));
                    return std::numeric_limits<double>::min();
                } else {
                    std::println("Min element: {:.3f}", *std::ranges::min_element(accelerations));

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
                std::println(" R Lane change: Car{} (x = {:.2f}, v = {:.2f}) from lane {} to {}. Utilities: R:{:.3f} L:{:.3f} Stay:{:.3f}", alpha->getId(), alpha->getPosition(), alpha->getVelocity(), ilane, ilane - 1, right, left, changeThreshold_);
                laneChanges.push_back({alpha ,ilane, ilane-1});
            } else if (left > right and left > changeThreshold_){
                std::println(" L Lane change: Car{} (x = {:.2f}, v = {:.2f}) from lane {} to {}. Utilities: R:{:.3f} L:{:.3f} Stay:{:.3f}", alpha->getId(), alpha->getPosition(), alpha->getVelocity(),ilane, ilane + 1, right, left, changeThreshold_);
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
        // std::cout << std::format("Lane Change: Car {}(x = {:.2f}) from lane {} to {}", carIter->getId(), carIter->getPosition(), oldlane, newlane)<< std::endl;
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
    for (size_t i : std::views::iota(0UL, nLanes_)){
       auto c = flowGenerators_[i].generateFlow(dt);
        if (c){ lanes_[i].insert(*c); }

    }
    return {};
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

// How to track timestep 42, lane change for car 4. Lane change should be 