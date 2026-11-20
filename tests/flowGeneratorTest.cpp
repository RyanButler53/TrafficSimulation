#include <gtest/gtest.h>
#include <sim/flowGenerator.hpp>
#include <ranges>
#include "sim/random.hpp"


struct AlwaysGenerate : public RandomGenerator {
    double getValue(std::mt19937&) override {return 0.0;}
};


struct MockCarFactory: public CarFactory {

    MockCarFactory(): CarFactory(0.2, 0){}
    ~MockCarFactory(){};

    Car makeCar(double x0, double v0, double, double t) override {
        return Car(0, x0, v0, t, 0.2, {});
    }

    Car makeCar(size_t id, double x0, double v0, double, double t0, double, double, double, double p) override {
        return Car(id, x0, v0, t0, p, {});
    }
};


class FlowGeneratorTest : public ::testing::Test {
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


TEST_F(FlowGeneratorTest, generateAllCars){
    std::vector<Car> cars;
    setRng(std::make_shared<UniformDistribution>(0, 1));

    // Generate 1 hour of cars at dt = 0.2
    for (auto t : std::views::iota(0, 18000)){
        if (std::optional<Car> c = gen->generateFlow(2000, 30)){
            cars.push_back(*c);
            EXPECT_EQ(c->getVelocity(), 20);
        }
    }
    EXPECT_EQ(gen->numRngRolls(), 19200); // 1 per timestep, 2 per generation
    EXPECT_EQ(cars.size(), 600);

}

TEST_F(FlowGeneratorTest, generateWithRoadblock){
    std::vector<Car> cars;
    setRng(std::make_shared<UniformDistribution>(0, 1));


    for (auto t : std::views::iota(0, 9000)){
        // next car = -1 implies car is behind the flow generator and no flow can be generated
        if (std::optional<Car> c = gen->generateFlow(-1, 30)){
            FAIL() << "Car was not supposed to be generated!";
        }
    }
    EXPECT_EQ(gen->numRngRolls(), 9000);
    for (auto _ : std::views::iota(0, 9000)){
        if (std::optional<Car> c = gen->generateFlow(200, 30)){
            cars.push_back(*c);
            EXPECT_EQ(c->getVelocity(), 20);
        }
    }
    EXPECT_EQ(gen->numRngRolls(), 19200);
    EXPECT_EQ(cars.size(), 600);
}

TEST_F(FlowGeneratorTest, adjustedV0){

    setRng(std::make_shared<AlwaysGenerate>());

    EXPECT_EQ(gen->generateFlow(95, 15)->getVelocity(), 15);
    EXPECT_EQ(gen->generateFlow(100, 15)->getVelocity(), 20); // Equal case doesn't slow down
    EXPECT_EQ(gen->generateFlow(115, 15)->getVelocity(), 20);
    EXPECT_EQ(gen->numRngRolls(), 9);
}

TEST_F(FlowGeneratorTest, rngRollCount){
    size_t ncars = 0;
    setRng(std::make_shared<UniformDistribution>(0, 1));

    for (auto t : std::views::iota(0, 36000)){
        if (gen->generateFlow(200, 30)){
            ++ncars;
        }
        EXPECT_EQ(gen->numRngRolls(), 2*ncars + t + 1);
    }
    EXPECT_EQ(gen->numRngRolls(), 38400);
}

TEST_F(FlowGeneratorTest, restoreState){
    setRng(std::make_shared<UniformDistribution>(0, 1));
    for (auto _ : std::views::iota(0, 18000)){
        gen->generateFlow(200, 30);
    }
    ASSERT_EQ(gen->numRngRolls(), 19200);

    // Create another flow generator and skip it ahead an hour of rng rolls. 
    std::mt19937 rng2(70);
    auto factory = std::make_shared<MockCarFactory>();
    std::unique_ptr<FlowGenerator> gen2 = std::make_unique<FlowGenerator>(600, 0, factory, 0.2, std::make_shared<std::mt19937>(rng2));

    gen2->setRng(std::make_shared<NormalDistribution>(20, 0), 
        std::make_shared<NormalDistribution>(22, 0),
        std::make_shared<UniformDistribution>(0, 1)
    );
    gen2->discardRng(19200);

    for (auto _ : std::views::iota(0, 10)){
        std::optional<Car> c1 = gen->generateFlow(200, 30);
        std::optional<Car> c2 = gen2->generateFlow(200, 30);

        if (c1){
            ASSERT_TRUE(c2.has_value());
            EXPECT_EQ(c1->getPosition(), c2->getPosition());
            EXPECT_EQ(c1->getVelocity(), c2->getVelocity());
        } else if (c2){
            ASSERT_TRUE(c1.has_value());
            EXPECT_EQ(c1->getPosition(), c2->getPosition());
            EXPECT_EQ(c1->getVelocity(), c2->getVelocity());
        }
    }
    EXPECT_EQ(gen->numRngRolls(), gen2->numRngRolls());
}

// Ensure the rng is properly modifying  
TEST(RngTest, DifferentValues){
    UniformDistribution dist(0, 1);
    std::mt19937 rng{105};
    double sum = 0;
    for (size_t i : std::views::iota(0, 100)){
        double v = dist.getValue(rng);
        sum += dist.getValue(rng);
    }
    EXPECT_NEAR(sum, 50, 5);
}