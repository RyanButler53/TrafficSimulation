// Algorithm test for Traffic Simulator
#include <gtest/gtest.h>
#include <expected>
#include <filesystem>
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <ranges>
#include <type_traits>
#include "sim/simulator.hpp"
#include "yaml-cpp/yaml.h"

#include "sim/parser.hpp"
#include "sim/parserFactory.hpp"
#include "sim/simulator.hpp"
#include "api/DBManager.hpp"

// Test Cases
#include "testUtil.hpp"
#include "AlgTest.hpp"

class NameGenerator {
    public:
      template <typename T>
      static std::string GetName(int) {
         if constexpr (std::is_same_v<T, Test3Lane>) return "Test3Lane";
         if constexpr (std::is_same_v<T, TestZeroFlow>) return "TestZeroFlow";
         if constexpr (std::is_same_v<T, TestLaneClosure>) return "TestLaneClosure";
      }
  };
  


template <typename TestCase>
class AlgorithmTest : public testing::Test {

    public:
    static std::vector<RawData> rawData_;

    static void SetUpTestSuite() {

        TestCase c;
        c.generateInput();
        TestUtil::clearDB();

        auto simResult = Traffic::Simulate(c.filename());
        ASSERT_TRUE(simResult.has_value()) << "Simulator Failed: " << simResult.error();
        DBManager reader(true);
        std::expected<std::vector<RawData>, std::string> raw = reader.queryData(c.testName());
        ASSERT_TRUE(raw.has_value()) << raw.error();
        rawData_ = *raw;
    }

    static void TearDownTestSuite() {
        TestUtil::clearDB();
        std::filesystem::path input = TestCase().filename();
        if (std::filesystem::exists(input)) std::filesystem::remove(input);
    }

};

using testing::Types;

typedef Types<Test3Lane,
              TestZeroFlow,
              TestLaneClosure> Implementations;
     
TYPED_TEST_SUITE(AlgorithmTest, Implementations, NameGenerator);

template <typename TestCase>
std::vector<RawData> AlgorithmTest<TestCase>::rawData_;

TYPED_TEST(AlgorithmTest, ForwardMovement){
    TypeParam().forwardMovement(AlgorithmTest<TypeParam>::rawData_);
}

TYPED_TEST(AlgorithmTest, LaneChanges){
    TypeParam().laneChanges(AlgorithmTest<TypeParam>::rawData_);
}

TYPED_TEST(AlgorithmTest, InBounds){
    TypeParam().inBounds(AlgorithmTest<TypeParam>::rawData_);
}

TYPED_TEST(AlgorithmTest, FlowGeneration){
    TypeParam().flowGeneration(AlgorithmTest<TypeParam>::rawData_);
}



