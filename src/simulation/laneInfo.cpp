#include "sim/laneInfo.hpp"
#include <ranges>
#include <algorithm>
#include <format>
#include <ranges>
#include <iterator>

bool LaneInfo::addSegment(double start, double end, size_t position){

    std::vector<LaneBoundary> overlaps = iTree_.findOverlaps({start, end});
    auto iter = std::ranges::find_if(overlaps, [position](const LaneBoundary& l){return l.position_ == position;});
    if (iter == overlaps.end()){
        iTree_.insert({start, end, position});
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