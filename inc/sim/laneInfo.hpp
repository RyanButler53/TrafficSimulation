/**
 * @file laneInfo.hpp
 * @author  Ryan Butler (rmbutler@outlook.com)
 * @brief Class holding lane boundaries
 * @version 0.1
 * @date 2026-07-01
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#pragma once
#include <expected>
#include <optional>
#include <vector>
#include <set>
#include <numeric>
#include "shared/environment.hpp"

/// @brief Lane boundary class to store the high and low boundaries of a segment
struct LaneBoundary {
    double low_;
    double high_;

    bool operator==(const LaneBoundary&) const = default;

};    

/// @brief Small enum representing lane change direction
enum class Direction : int8_t{
    LEFT = -1,
    RIGHT = 1
};

/**
 * @brief Class to hold and answer queries about lanes. Particularly start
 * and ends of lanes and segments of lanes and environment
 * 
 */
class LaneInterval {

    std::vector<std::set<double>> lanes_;

    public: 
    /**
     * @brief Get the Lane Segment that x would fall in if it exists. If the lane
     * doesn't exist, returns std::nullopt. 
     * 
     * @param ilane Position of the lane
     * @param x X position
     * @return std::optional<LaneBoundary> Returns the low, high and position the x value lands in. 
     * 
     */
    std::optional<LaneBoundary> getLaneSegment(size_t ilane, double x);

    /**
     * @brief Inserts a lane segment in the correct lane if it can fit there. 
     * @details checks getLaneSegment() to see if one already exists before adding it. 
     * 
     * @param start Start of the lane (meters)
     * @param end End of the lane (meters)
     * @param position Lane position (index)
     * @return true if a lane was added, false if not. 
     */
    bool insert(double start, double end, size_t position);

    /**
     * @brief Returns the environment represented by this laneInterval object. 
     * 
     * @param start start of the road (m)
     * @param end end of the road (m)
     * @return Environment struct with the data for lane segments populated
     */
    Environment getEnv(double start, double end);
};

/**
 * @brief Class storing lane info. 
 * @details This class stores data about each lane. Used to quickly determine
 * what segment an x position is in and other properties about the road. See the listed
 * public methods
 * 
 */
 class LaneInfo {

    /// @brief Underlying representation of the lanes. 
    LaneInterval lanes_;

    /**
     * @brief Vector storing the lane ends for all known lanes. 
     * @note This stores optionals since the index is used as a lookup key. If resizing 
     * the vector, initializing to a nullopt ensures that the the maximum value is set correctly.
     * 
     */
    std::vector<std::optional<double>> laneEnds_;

    /**
     * @brief  The maximum x value for all lanes in the simulation. 
     * @note Initialized to zero to ensure all the end is continuously updated correctly 
     * 
     */
    double endOfRoad_ = 0;

    /**
     * @brief Start of road. The minimum x value for all lanes in the simulation. This probably will be zero
     * @note Initialized to the max to ensure that the start is continuously updated correctly. 
     */
    double startOfRoad_ = std::numeric_limits<double>::max();

    /**
     * @brief Keep right bias for the highway. This is defaulted to 0.2 (per Traffic Flow Dynamics textbook)
     * @details This bias is used when all lanes in a lane change are open. The bias is added to right lane changes
     * and subtracted for left lane changes
     * @note This is configureable in the input parameter
     */
    double bias_ = 0.2;

    /**
     * @brief Scalar value for adding "pressure" to move away from a lane that is going to end. 
     * @details The bias is changed by a factor of changePressure_/switchThreshold for every meter past
     * switchThreshold_. This encourages cars to move away from a lane that is about to end
     */
    double changePressure_ = 0.4;

    /**
     * @brief X threshold where the keep right bias begins to break down to encourage a lane change. 
     * @details See \ref changePressure to see how this is used mathematically 
     * 
     */
    double switchThreshold_ = 1600;

    /**
     * @brief Utility function to get the lane segment (start and end) that x is in.
     * 
     * @param x x position to check
     * @param ilane Lane index
     * @return std::optional<LaneBoundary> Returns a Lane Boundary if the lane exists at the given x position, nullopt if not. 
     */
    inline std::optional<LaneBoundary> getLane(double x, size_t ilane);

    public:

    /** 
     * @brief Constructs a default LaneInfo. This initially has no lane segments added. 
     * @note This constructor doesn't set changePressure, switchThreshold and bias
     */
    LaneInfo() = default;

    /**
     * @brief Constructs a default LaneInfo. This initially has no lane segments added. 
     * 
     * @param bias Keep right bias. See \ref bias_
     * @param changePressure Factor to encourage lane change. See \ref changePressure
     * @param switchThreshold Threhold number of meters before cars start trying to move over to avoid lane changes. See \ref switchThreshold_
     */
    LaneInfo(double bias, double changePressure, double switchThreshold);

    /**
     * @brief Add a lane segment to a lane. Checks that there is no overlap with existing lane segments
     * 
     * @param start Start position of the new lane
     * @param end End position of the new lane. 
     * @param position Index of the lane. Index of zero means rightmost lane. Higher indexes mean further left lanes. 
     * @return True if the lane was added, false if it had an overlap with another lane. 
     */
    bool addSegment(double start, double end, size_t position);

    /**
     * @brief Check if a lane exists at a specific x value. Thin wrapper around LaneInfo::getLane
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
     * @return Returns the x position (m) of the end of lane if it exists, error string otherwise
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

    /**
     * @brief Returns the largest x value of any lane in the simulation
     * 
     * @return double X position in meters
     */
    double endOfRoad() const;

    /**
     * @brief Gets the envrionment associated with the underlying LaneIntervals 
     * 
     * @return Populated environment struct with the lane intervals. Passes down to lanes_.getEnd
     */
    inline Environment getEnv(){
        return lanes_.getEnv(startOfRoad_, endOfRoad_);
    }
};

