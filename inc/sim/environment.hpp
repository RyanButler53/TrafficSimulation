/**
 * @file environment.hpp
 * @author Ryan Butler
 * @brief Defines the Environment struct
 * @version 0.1
 * @date 2026-08-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

 #pragma once
 #include <vector>

/**
 * @brief Defines the envrionment struct to be passed to the logger
 * @details This struct is similar to the data in the input file but is only
 * about the road conditions. This struct is created by the Highway class
 * and is sent to the logger and sent to file or disk. 
 */
struct Environment {

    struct LaneSegment{
        double start;
        double end;
        double rate;
    };

    struct EmptySegment{
        double start;
        double end;
    };

    std::vector<LaneSegment> segments_;
    std::vector<EmptySegment> emptySegments_;
    double x0;
    double xf;
    size_t nlanes;
};
