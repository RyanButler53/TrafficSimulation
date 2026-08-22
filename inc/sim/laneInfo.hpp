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

enum class Direction : int8_t{
    RIGHT = -1,
    LEFT = 1
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
    
    double bias_ = 0.2;
    double changePressure_ = 0.4;
    // X threshold where biases break down to force a lane change. 
    double switchThreshold_ = 1600;

    std::optional<LaneBoundary> getLane(double x, size_t ilane);


    public:
    LaneInfo() = default;

    LaneInfo(double bias, double changePressure, double switchThreshold);
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


    /**
     * @brief Calculates the lane change bias for a car looking to change lanes
     * @warning This function has no error handing and can only be called if x exists in both the current and target lanes
     * @param x X position of the current car
     * @param ilane CURRENT lane the car is in
     * @param dir Direction the car is trying to lane change.
     * @return Bias for this x position, lane position and lane chang direction
     */
    double calculateBias(double x, size_t ilane, Direction dir);

    /**
     * @brief Returns if the segment x is in is at the end of the lane (encapsulates this computation)
     * 
     * @param x : X position to check
     * @param ilane lane index
     * @return true if x is in the last segment of its lane. Do true free road acceleration and bias calculation
     * @return false if x is in a segment that will end. Acceleration and bias calculations must account for this. 
     */
    bool lastSegment(double x, size_t ilane);
    double endOfRoad() const;
};

