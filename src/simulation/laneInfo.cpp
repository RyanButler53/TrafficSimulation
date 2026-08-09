#include "sim/laneInfo.hpp"
#include <ranges>
#include <algorithm>
#include <format>
#include <ranges>
#include <iterator>

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

double LaneInfo::endOfRoad() const{
    return endOfRoad_;
}
