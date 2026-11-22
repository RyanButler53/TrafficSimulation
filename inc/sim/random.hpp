/**
 * @file random.hpp
 * @author  Ryan Butler (rmbutler@outlook.com)
 * @brief Interface for a random number generator. Used for mocking in flow generation tests
 * @version 0.1
 * @date 2026-08-22
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#pragma once
#include <random>
#include <variant>
#include <memory>
#include <sstream>
#include <charconv>

/**
 * @brief Wrapper around std::mt19937 to count actual engine rolls for reproducibility
 * 
 */
struct RngEngine {
    using result_type = std::mt19937::result_type;
    static constexpr result_type min() { return std::mt19937::min(); }
    static constexpr result_type max() { return std::mt19937::max(); }

    result_type operator()();

    std::mt19937 eng;
    std::uint64_t draws = 0;

    bool operator==(const RngEngine& other) const { return eng == other.eng;}

    void discard(size_t n);

    uint64_t getState() { return draws;}
};

/**
 * @brief Abstract class for randomness generators. 
 * 
 * @pure Derived classes must implement getValue() to get the next value
 * 
 */
struct RandomGenerator {

    using ptr = std::shared_ptr<RandomGenerator>;

    /**
     * @brief Gets the next random value from the generator. This is the only pure virtual function
     * 
     * @param rng random number generator
     * @return double value
     */
    virtual double getValue(RngEngine& rng) = 0;

    /**
     * @brief Dumps the state of the generator to a string
     * 
     * @return std::string 
     */
    virtual std::string getState() = 0;

};

/**
 * @brief Class for a uniform random distribution. 
 * @details Used for flow generation to ensure that the actual rate is correct. 
 * 
 */
class UniformDistribution : public RandomGenerator {
    std::uniform_real_distribution<double> dist_;

    public:

    /**
     * @brief Construct a new Uniform Distribution 
     * 
     * @param min Minimum value
     * @param max Maximum value
     */
    UniformDistribution(double min, double max);

    /**
     * @brief Construct a new Uniform Distribution from a serialized string
     * 
     * @param state 
     */
    UniformDistribution(std::string state);

    double getValue(RngEngine& rng) override;

    std::string getState() override;
};

/**
 * @brief Constructs a normal distribution with a mean and standard deviaton. 
 * Handles the implementation defined case of a standard deviation of zero
 */
class NormalDistribution : public RandomGenerator {

    std::variant<std::normal_distribution<double>, double> dist_;
    using Dist_t = std::normal_distribution<double>;
    public:

    NormalDistribution(double mean, double stdev);

    NormalDistribution(std::string state);

    double getValue(RngEngine& rng) override;

    std::string getState() override;

    double operator()(RngEngine& rng){ return getValue(rng); }
};
