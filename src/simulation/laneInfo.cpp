#include "sim/laneInfo.hpp"
#include <ranges>
#include <algorithm>
#include <array>
#include <format>
#include <algorithm>
#include <ranges>
#include <functional>
#include <iterator>

std::optional<LaneBoundary> LaneInterval::getLaneSegment(size_t ilane, double x){
    if (ilane >= lanes_.size()){
        return std::nullopt;
    } 
    const std::set<double>& lane = lanes_[ilane];

    if (x < *lane.begin() || x > *lane.rbegin()){
        return std::nullopt;
    }
    // Get the index
    auto iter = std::lower_bound(lane.begin(), lane.end(), x);
    // On a boundary. Always in the segment. Which segment? If index is even, then (iter, iter+1), otherwise, iter-1, iter
    if (*iter == x){
        if (std::distance(lane.begin(), iter) % 2 == 1){
            --iter;
        }
        return LaneBoundary(*iter, *(++iter));
    }
    // Not on a boundary. Lower_bound will always give me the upper bound of the interval
    if (std::distance(lane.begin(), iter) % 2 == 1){
        --iter;
        return LaneBoundary(*iter, *(++iter));
    } else {
        return std::nullopt;
    }

}
// what if it was just a set if numbers? If the index we find is odd or even it is in a lane ir not
// 0, 20, 55,60, 85,100 -> 30,40 lower_bound is the same for both AND lower bound is at an odd index

bool LaneInterval::insert(double start, double end, size_t position){
    if (position >= lanes_.size()){
        lanes_.resize(position + 1);
        lanes_[position] = {start, end};
        return true;
    } else if (lanes_[position].empty()){
        lanes_[position] = {start, end};
        return true;
    } else {
        const std::set<double>& lane = lanes_[position];
        auto lower = std::lower_bound(lane.begin(), lane.end(), start);
        auto higher = std::lower_bound(lane.begin(), lane.end(), end);
        // auto diff = std::distance(lane.begin(), lower);
        if (lower == higher && lower != lane.end()){
            auto dist = std::distance(lane.begin(), lower);
            if (dist % 2 == 1){
                lanes_[position].insert_range(std::array{start, end});
            }
            return dist % 2 == 1;
        } else if (lower == lane.end()) {
            lanes_[position].insert_range(std::array{start, end});
            return true;
        } else {
            return false;
        }
    }
}

Environment LaneInterval::getEnv(double start, double end){
    Environment e;
    e.nlanes = lanes_.size();
    e.x0 = start;
    e.xf = end;

    for (size_t ilane = 0; ilane < e.nlanes; ++ilane){
        // Convert into a vector for indexing syntax
        std::vector<double> data(lanes_[ilane].begin(), lanes_[ilane].end());

        if (data.front() != start){
            e.emptySegments_.emplace_back(start, data.front(), int(ilane));
        }
        if (data.back() != end){
            e.emptySegments_.emplace_back(data.back(), end, int(ilane));
        }
        for (size_t i = 0; i < data.size() - 1; ++i){
            // If i is even, the interval between lanes[i]
            if (i % 2 == 0) {
                e.segments_.emplace_back(data[i], data[i+1], -1.0, int(ilane));
            } else {
                e.emptySegments_.emplace_back(data[i], data[i+1], int(ilane));
            }
        }
    }
    return e;
}

// LANE INFO  FUNCTIONS

LaneInfo::LaneInfo(double bias, double changePressure, double switchThreshold):
    bias_{bias}, changePressure_{changePressure}, switchThreshold_{switchThreshold}{}


bool LaneInfo::addSegment(double start, double end, size_t position){
    if (lanes_.insert(start, end, position)){

        // Insert into the laneEnds lookup table
        if (laneEnds_.size() < position+1){
            laneEnds_.resize(position + 1);
        }

        // Update end of road and corresponding lane end. 
        startOfRoad_ = std::min(start, startOfRoad_);
        endOfRoad_ = std::max(end, endOfRoad_);

        laneEnds_[position] = laneEnds_[position].transform([end](double cur)
                                                            { return std::max(cur, end); })
                                                 .or_else([end]()
                                                            { return std::make_optional(end); });
        return true;
    } else {
        return false;
    }
}

std::optional<LaneBoundary> LaneInfo::getLane(double x, size_t ilane){
    return lanes_.getLaneSegment(ilane, x);
}

bool LaneInfo::laneValid(double x, size_t ilane){
    return getLane(x, ilane).has_value();
}

std::expected<double, std::string> LaneInfo::endOfSegment(double x, size_t ilane){
    // TODO: Monads
    // return getLane(x, ilane).transform([](const LaneBoundary& l){return std::expected<double, std::string>(l.high());})
    //                         .or_else([x, ilane](){return std::unexpected(std::format("Lane {} is not present at x = {}", ilane, x));});
    std::optional<LaneBoundary> end = getLane(x, ilane);
    if (end){
        return end->high_;
    } else {
        return std::unexpected(std::format("Lane {} is not present at x = {}", ilane, x));
    }
}

std::expected<double, std::string> LaneInfo::endOfLane(size_t ilane){
    if (ilane < laneEnds_.size() && laneEnds_[ilane]){
        return laneEnds_[ilane].value();
    } else {
        return std::unexpected(std::format("Lane {} has no segments and the end of the lane is undefined!", ilane));
    }
}

double LaneInfo::endOfRoad() const {
    return endOfRoad_;
}

bool LaneInfo::lastSegment(double x, size_t ilane) {
    std::expected<double, std::string> segEnd = endOfSegment(x, ilane);
    return segEnd.has_value() && segEnd.value() == endOfRoad_;
}

double LaneInfo::calculateBias(double x, size_t ilane, Direction dir){

    auto lane = std::make_optional<size_t>(ilane);
    std::vector<double> laneEnds(laneEnds_.size());

    // Figure out the of lane segments for ALL lanes. 
    for (size_t laneIndex : std::views::iota(0UL, laneEnds_.size())){
        if (laneValid(x, laneIndex)){
            laneEnds[laneIndex] = endOfSegment(x, laneIndex).value();
        } else {
            laneEnds[laneIndex] = endOfRoad_;
        }
    }

    // Find the lane index of the closest lane to ending 
    auto closest = std::min_element(laneEnds.begin(), laneEnds.end());
    double closestEndOfLane = *closest;
    // If distance * direction is positive, then lane change in direction dir represents moving away from lane going to end. 
    // Zero if the current lane will end. 
    // Negative if moving towards the lane that will end.  
    auto distance = std::distance(laneEnds.begin() + ilane, closest);

    std::function<double()> bias = [this, dir](){return double(dir) * bias_;};
    // If none of the lanes end (closest is the end of the road), return bias()
    if (closestEndOfLane == endOfRoad_ or closestEndOfLane > x+ switchThreshold_){
        return bias();
    } 
    double xCrit = closestEndOfLane - switchThreshold_;
    int d(distance);
    if (distance == 0){ // the current lane is the one that needs to end. 
        return bias() + ((changePressure_/switchThreshold_) * (x - xCrit));
    } else {
        // If moving away from the lane that ends, increment/decrement the distance to scale the bias effect. 
        bool movingAway = (double(dir) * 1.0/(d)) > 0;
        if (movingAway){
            d = d > 0 ? ++d : --d;
        }
        return bias() + double(dir) * 1.0/(d) * ((changePressure_/switchThreshold_) * (x - xCrit));
    }
}

Environment LaneInfo::getEnv(){
    return lanes_.getEnv(startOfRoad_, endOfRoad_);
}
