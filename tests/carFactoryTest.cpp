#include <gtest/gtest.h>
#include <ranges>
#include <memory>

#include "sim/carFactory.hpp"
#include "sim/strategy.hpp"
#include "sim/random.hpp"


class CarFactoryTest : public ::testing::Test {
    
    protected:

    std::unique_ptr<CarFactory> factory_ = nullptr;
    size_t seed_{131};

    std::unique_ptr<CarFactory> makeCarFactory(){
        return std::make_unique<CarFactoryImpl<decltype(&makeGippsUpdateFunc)>>(
            2.5, -2, -3.5, 0.2, 
            0.1, 0.1, 0.1, 0.1, 
            seed_, makeGippsUpdateFunc);
    }

    void compareGenerators(std::unique_ptr<CarFactory>& f1, std::unique_ptr<CarFactory>& f2, int n){
        for (auto _ : std::views::iota(0,n)){
            Car c = f1->makeCar(100, 20, 25);
            Car c2 = f2->makeCar(100, 20, 25);
            ASSERT_EQ(c, c2);
        }
    }

    void generateCars(int n){
        for (auto _ : std::views::iota(0, n)){
            factory_->makeCar(0, 0, 0);
        }
    }

    void SetUp() override {
        factory_ = makeCarFactory();
        generateCars(100);

        auto [rngRolls, id] = factory_->getState();
        EXPECT_GT(rngRolls, 0); // Must have run rng engine at least once
        ASSERT_EQ(id, 100); // 100 will be the NEXT car id
    }
};

TEST_F(CarFactoryTest, SkipRestoreState){
    std::unique_ptr<CarFactory> unrestoredFactory = makeCarFactory();
    // Cars should be not equal when not restoring the state
    for (auto _ : std::views::iota(0,5)){
        Car c = factory_->makeCar(100, 20, 25);
        Car c2 = unrestoredFactory->makeCar(100, 20, 25);
        EXPECT_NE(c, c2);
    }
}

TEST_F(CarFactoryTest, RestoreState){

    auto [rngRolls, _] = factory_->getState();
    std::unique_ptr<CarFactory> restoredFactory = makeCarFactory();
    restoredFactory->restore(rngRolls, 100);

    compareGenerators(factory_, restoredFactory, 100);
    auto s1 = factory_->getState();
    auto s2 = restoredFactory->getState();
    EXPECT_EQ(s1.first, s2.first);
    EXPECT_EQ(s1.second, s2.second);
}

TEST_F(CarFactoryTest, RestoreStateSavedCars){
    auto [rngRolls, id] = factory_->getState();
    std::unique_ptr<CarFactory> restoredFactory = makeCarFactory();

    // Make 100 cars without calling the randomness generator
    for (auto i : std::views::iota(0, 100)){
        restoredFactory->makeCar(i, 0, 0, 0, 5.0, 0, 0, 0, 0.2);
    }

    restoredFactory->restore(rngRolls, 100);

    compareGenerators(factory_, restoredFactory, 100);
    auto s1 = factory_->getState();
    auto s2 = restoredFactory->getState();
    EXPECT_EQ(s1.first, s2.first);
    EXPECT_EQ(s1.second, s2.second);
}
