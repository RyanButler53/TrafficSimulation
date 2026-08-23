#include <gtest/gtest.h>
#include <sim/flowGenerator.hpp>
#include <ranges>
#include "sim/random.hpp"


struct AlwaysGenerate : public RandomGenerator {
    double getValue(std::mt19937) override {return 0.0;}
};



struct MockCarFactory: public CarFactory {

    MockCarFactory(): CarFactory(0.2, 0){}
    ~MockCarFactory(){};

    Car makeCar(double x0, double v0, double vdes, double t) override {
        return Car(0, x0, v0, t, 0.2, {});
    }
};


class FlowGeneratorTests : public ::testing::Test {
    protected:
    std::unique_ptr<FlowGenerator> gen = nullptr;
    std::mt19937 rng_{70};

    void setRng(RandomGenerator::ptr generationDist){
        gen->setRng(std::make_shared<NormalDistribution>(20, 0), 
                    std::make_shared<NormalDistribution>(22, 0),
                    generationDist
        );
    }

    void SetUp() override{
        auto factory = std::make_shared<MockCarFactory>();
        gen = std::make_unique<FlowGenerator>(600, 0, factory, 0.2, std::make_shared<std::mt19937>(rng_));
    }
};


TEST_F(FlowGeneratorTests, generateAllCars){
    std::vector<Car> cars;
    setRng(std::make_shared<UniformDistribution>(0, 1));

    for (auto t : std::views::iota(0, 18000)){
        if (std::optional<Car> c = gen->generateFlow( 2000, 30)){
            cars.push_back(*c);
            EXPECT_EQ(c->getVelocity(), 20);
        }
    }

    EXPECT_EQ(cars.size(), 600);

}

TEST_F(FlowGeneratorTests, generateWithRoadblock){
    std::vector<Car> cars;
    setRng(std::make_shared<UniformDistribution>(0, 1));


    for (auto t : std::views::iota(0, 9000)){
        // next car = -1 implies car is behind the flow generator and no flow can be generated
        if (std::optional<Car> c = gen->generateFlow(-1, 30)){
            FAIL() << "Car was not supposed to be generated!";
        }
    }

    for (auto t : std::views::iota(0, 9000)){
        if (std::optional<Car> c = gen->generateFlow(200, 30)){
            cars.push_back(*c);
            EXPECT_EQ(c->getVelocity(), 20);
        }
    }
    EXPECT_EQ(cars.size(), 600);
}

TEST_F(FlowGeneratorTests, adjustedV0){

    setRng(std::make_shared<AlwaysGenerate>());

    EXPECT_EQ(gen->generateFlow(95, 15)->getVelocity(), 15);
    EXPECT_EQ(gen->generateFlow(100, 15)->getVelocity(), 20); // Equal case doesn't slow down
    EXPECT_EQ(gen->generateFlow(115, 15)->getVelocity(), 20);

}