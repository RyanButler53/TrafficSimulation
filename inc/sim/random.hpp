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

/**
 * @brief Wrapper around std::mt19937 to count actual engine rolls for reproducibility
 * 
 */
struct RngEngine {
    using result_type = std::mt19937::result_type;
    static constexpr result_type min() { return std::mt19937::min(); }
    static constexpr result_type max() { return std::mt19937::max(); }

    result_type operator()() {
        ++draws;
        return eng();
    }

    std::mt19937 eng;
    std::uint64_t draws = 0;

    bool operator==(const RngEngine& other) const {
        return eng == other.eng;
    }

    void discard(size_t n){
        eng.discard(n);
        draws += n;
    }
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
    UniformDistribution(double min, double max):
        dist_{min, max}{}

    double getValue(RngEngine& rng) override {
        return dist_(rng);
    }
};

/**
 * @brief Constructs a normal distribution with a mean and standard deviaton. 
 * Handles the implementation defined case of a standard deviation of zero
 */
class NormalDistribution : public RandomGenerator {

    std::variant<std::normal_distribution<double>, double> dist_;

    public:

    NormalDistribution(double mean, double stdev) {
        if (stdev == 0.0){
            dist_ = mean;
        } else {
            dist_ = std::normal_distribution<double>(mean, stdev);
        }
    }

    double getValue(RngEngine& rng) override {
        if (std::holds_alternative<double>(dist_)){
            return std::get<double>(dist_);
        } else {
            auto& dist = std::get<std::normal_distribution<double>>(dist_);
            return dist(rng);

        }
    }

    double operator()(RngEngine& rng){
        return getValue(rng);
    }

};
