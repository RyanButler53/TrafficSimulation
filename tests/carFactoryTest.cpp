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

    void SetUp() override {
        factory_ = makeCarFactory();
    }
};

TEST_F(CarFactoryTest, SkipRestoreState){

}

TEST_F(CarFactoryTest, RestoreState){
    // Make 100 cars
    for (auto c : std::views::iota(0, 100)){
        factory_->makeCar(0, 0, c);
    }

    auto [rngRolls, id] = factory_->state();
    EXPECT_GT(rngRolls, 0); // Must have run rng engine at least once
    ASSERT_EQ(id, 100); // 100 will be the NEXT car id

    std::unique_ptr<CarFactory> restoredFactory = makeCarFactory();
    restoredFactory->restore(rngRolls, 100);

    for (auto _ : std::views::iota(0,5)){
        Car c = factory_->makeCar(100, 20, 25);
        Car c2 = restoredFactory->makeCar(100, 20, 25);
        EXPECT_EQ(c, c2);
    }
}

