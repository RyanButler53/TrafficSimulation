#include "sim/laneInfo.hpp"
#include <ranges>
#include <algorithm>
#include <format>
#include <ranges>
#include <functional>
#include <iterator>

LaneInfo::LaneInfo(double bias, double changePressure, double switchThreshold):
    bias_{bias}, changePressure_{changePressure}, switchThreshold_{switchThreshold}{}


bool LaneInfo::addSegment(double start, double end, size_t position){

    std::vector<LaneBoundary> overlaps = iTree_.findOverlaps(LaneBoundary{start, end});
    auto iter = std::ranges::find_if(overlaps, [position](const LaneBoundary& l){return l.position_ == position;});
    if (iter == overlaps.end()){
        iTree_.insert({start, end, position});

        // Insert into the laneEnds lookup table
        if (laneEnds_.size() < position+1){
            laneEnds_.resize(position + 1);
        }

        // Update end of road and corresponding lane end. 
        endOfRoad_ = std::max(end, endOfRoad_);

        laneEnds_[position] = laneEnds_[position].transform([end](double cur)
                                                            { return std::max(cur, end); })
                                                 .or_else([end]()
                                                            { return std::make_optional(end); });
    }
    return iter == overlaps.end();
}

std::optional<LaneBoundary> LaneInfo::getLane(double x, size_t ilane){
    std::vector<LaneBoundary> overlaps = iTree_.findOverlaps(x);
    auto iter = std::ranges::find_if(overlaps, [ilane](const LaneBoundary& l){return l.position_ == ilane;});
    if (iter != overlaps.end()){
        return std::make_optional(*iter);
    } else {
        return std::nullopt;
    }
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
        return end->high();
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
