#include <unordered_map>
#include <ranges>
#include <utility>
#include <map>
#include <numeric>

#include "highway.hpp"


CpuHighway::CpuHighway(size_t numLanes):nLanes_{numLanes},
    lanes_{std::make_shared<std::set<Car>[]>(numLanes)}{}

std::expected<void, std::string> CpuHighway::update(double dt){
    
    // Cache of accelerations of each car given that no lane changing occurs. x->a
    std::vector<std::unordered_map<double, double>> accelerationCache(nLanes_);
    
    // Iterate over all cars in all lanes to fill acceleration cache
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
                return std::unexpected(a.error());
            } else {
                cache.insert({current->getPosition(), a.value()});
            }
            // Next pair
            ++current;
            ++next;
        }
        cache.insert({current->getPosition(), current->acceleration(dt)});
    }

    // Run Lane Change algorithm to find all possible lane changes. 
    std::map<std::pair<double, int>, int> laneChanges;
    for (int ilane : std::views::iota(0, static_cast<int>(nLanes_))){
        std::set<Car>& cars = lanes_[ilane];

        // Lane changes are only possible if there are more than 3 cars in the lane. 
        if (cars.size() < 3){
            continue;
        }
        
        // Iterate over all cars in the lane, find f_hat and l_hat and compute utility. 
        std::set<Car>::const_iterator follow = cars.begin();
        std::set<Car>::const_iterator alpha = ++cars.begin();
        std::set<Car>::const_iterator lead = ++(++cars.begin());
        
        while (lead != cars.end()){
        

            double left = std::numeric_limits<double>::min();
            double right = std::numeric_limits<double>::min();
            double stay = a_;
            // std::array<double, 3> utilities {}

            // Calculate the lane change;
            auto calculateUtility = [&](int curLane, int newLane){
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
    
                // Return Utility function. 
                return (a_alphaChange - a_alpha) + alpha->politeness() * (a_fHatChange - a_fHat + a_fChange - a_f);
            };




            // If in leftmost lane, don't do left lane change computation 
            if (ilane == nLanes_ - 1){
                right = calculateUtility(ilane, ilane - 1);

            } else if (ilane == 0) { // rightmost lane

            } else {

            }

        }

        
    }
    return {};
}

