/**
 * @file AlgTest.hpp
 * @author Ryan Butler
 * @brief Outlines the class for an Algorithm Test
 * @version 0.1
 * @date 2026-07-14
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#pragma once

#include <concepts>
#include <filesystem>
#include <vector> 
#include "api/structs.hpp"
#include "yaml-cpp/yaml.h"


template <class A>
concept AlgTestCase = requires(A test,
    std::filesystem::path file
){
    file = test.filename();
    test.generateInput();
    test.forwardMovement();
    test.laneChanges();
    test.inBounds();
    test.flowGeneration();
};

/**
 * @brief Base class for Algorithm Tests
 * 
 */
class AlgTest {

    protected:

    void completeLaneBoundsCheck(const std::vector<RawData>& raw, float roadEnd);
    void simpleFlowGenerationCheck(const std::vector<RawData>& raw, size_t n);

    public:
    virtual std::string testName() = 0;
    virtual std::filesystem::path filename() = 0;
    virtual void generateInput() = 0;
    virtual size_t nlanes() const = 0;
    void forwardMovement(const std::vector<RawData>& raw);
    virtual void laneChanges(const std::vector<RawData>& raw);
    virtual void inBounds(const std::vector<RawData>& raw) = 0;
    virtual void flowGeneration(const std::vector<RawData>& raw) = 0;
};


class Test3Lane : public AlgTest{

    public:
    std::string testName() override;
    std::filesystem::path filename() override;
    void generateInput() override;
    size_t nlanes() const override;
    void inBounds(const std::vector<RawData>& raw) override;
    void flowGeneration(const std::vector<RawData>& raw) override;
};

class TestZeroFlow : public AlgTest {

    public:
    std::string testName() override;
    std::filesystem::path filename() override;
    size_t nlanes() const override;
    void generateInput() override;
    void inBounds(const std::vector<RawData>& raw) override;
    void flowGeneration(const std::vector<RawData>& raw) override;
};

class TestLaneClosure : public AlgTest{
    public:
    std::string testName() override;
    std::filesystem::path filename() override;
    size_t nlanes() const override;
    void generateInput() override;
    void inBounds(const std::vector<RawData>& raw) override;
    void flowGeneration(const std::vector<RawData>& raw) override;
};