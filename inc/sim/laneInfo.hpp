/**
 * @file laneBounds.hpp
 * @author  Ryan Butler (rmbutler@outlook.com)
 * @brief Class holding lane boundaries
 * @version 0.1
 * @date 2026-07-01
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#pragma once
#include "dataStructures/intervalTree.hpp"
#include <expected>
#include <optional>
#include <vector>

/// @brief Lane boundary class to implement IntervalTree's Interval interface
struct LaneBoundary : public SimpleInterval<double>{
    size_t position_;

    using SimpleInterval<double>::SimpleInterval;
    LaneBoundary(double low, double high, size_t p):
        SimpleInterval<double>(low, high), position_{p}{}

    size_t lane() const {return position_;}
};    

/**
 * @class Class to hold and answer queries about lanes. Particularly start
 * and ends of lanes and segments of lanes. 
 * 
 */
class LaneInfo {

    IntervalTree<LaneBoundary> iTree_;
    std::vector<std::optional<double>> laneEnds_;
    double endOfRoad_ = 0;

    std::optional<LaneBoundary> getLane(double x, size_t ilane);


    public:
    LaneInfo() = default;
    /**
     * @brief Adds many segmemnts to a lane. Checks that they are all valid and there are no overlaps
     * 
     * @param start 
     * @param end 
     * @param position 
     */
    bool addSegment(double start, double end, size_t position);

    /**
     * @brief Check if a lane exists at a specific x value
     */
    bool laneValid(double x, size_t lane);

    /**
     * @brief Returns the x position where the current lane ends. 
     * @return Returns an error string if the lane is not present at the x value. 
     */
    std::expected<double, std::string> endOfSegment(double x, size_t lane);

    /**
     * @brief Returns the end of the lane. 
     * @details Used to determine if a car can use true free road acceleration or a stopped car
     * 
     */
    std::expected<double, std::string> endOfLane(size_t ilane);

    double endOfRoad() const;
};

