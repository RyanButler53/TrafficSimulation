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
    // If there is no lane end of lane 
    size_t newLane = (dir == Direction::LEFT) ? ilane + 1 : ilane - 1;
    double endOfCurSegment = endOfSegment(x, ilane).value();
    double endOfNewSegment = endOfSegment(x, newLane).value();

    bool curLaneEnds = !lastSegment(x, ilane) && (endOfCurSegment < x + switchThreshold_);
    bool newLaneEnds = !lastSegment(x, newLane) && (endOfNewSegment < x + switchThreshold_);

    // If neither lane ends within switchThreshold meters
    std::function<double()> bias = [this, dir](){return -1 * double(dir) * bias_;};
    if (!curLaneEnds && !newLaneEnds){
        // -1 to flip left to negative bias 
        return bias();
    }

    double xCrit = std::min(endOfNewSegment, endOfCurSegment) - switchThreshold_;
    if (curLaneEnds && !newLaneEnds) {
        // Increase bias to encourage a lane change to the new lane
        return bias() + ((changePressure_/switchThreshold_) * (x - xCrit));
    } else if (!curLaneEnds && newLaneEnds){
        return bias() - ((changePressure_/switchThreshold_) * (x - xCrit));
    } else { // both lanes end. Return positive bias to move towards finding an open lane. 
        return bias_;
    }

}