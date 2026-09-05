#include "gtest/gtest.h"
#include <string>
#include "yaml-cpp/yaml.h"
#include "sim/simulator.hpp"
#include "sim/parserFactory.hpp"
#include "api/DBManager.hpp"

#include "testUtil.hpp"
#include "AlgTest.hpp"

template<typename T>
void extract(std::function<std::expected<T, std::string>()> f, T& out, std::string errmsg){
    std::expected<T, std::string> result = f();
    EXPECT_TRUE(result.has_value()) << "Unable to " << errmsg;
    out = result.value();
}

class EnvironmentComparison : public ::testing::Test {

    protected:
    /**
     * @brief GetEnvironment will run the parsing step of the simulation to return the logger and highway. 
     * The highway will get the environment and send it to the logger to write to the sink. 
     * @note This sidesteps actually running the simulation and is extrememly fast. 
     * @param cfg Path to the config file to parse and write to the file. 
     */
    void getEnvironment(std::string cfg){
        ParserFactory parserFac(cfg);
        // TODO : Monads!
        std::expected<SimulatorInputs, std::string> parseResult = parserFac.makeParser().and_then(std::mem_fn(&Parser::parse)).value();
        ASSERT_TRUE(parseResult.has_value()) << "Unable to parse config: " << cfg << " " << parseResult.error();
        SimulatorInputs inputs = parseResult.value();
        auto result = inputs.logger_->logEnvironment(inputs.highway_->environment());
        ASSERT_TRUE(result.has_value()) << "Unable to generate environment for config " << cfg << result.error();
    }

    Environment fromYaml(std::string path){
        YAML::Node env = YAML::LoadFile(path);
        Environment e;

        e.nlanes = size_t(env["nlanes"].as<int>());
        e.x0 = env["x0"].as<double>();
        e.xf = env["xf"].as<double>();
        for (YAML::Node n : env["road-segments"]){
            e.segments_.push_back(Environment::LaneSegment{
                n["start"].as<double>(),
                n["end"].as<double>(),
                n["rate"].as<double>(),
                n["position"].as<int>()
            });
        }
        for (YAML::Node n : env["empty-segments"]){
            e.emptySegments_.push_back(Environment::EmptySegment{
                n["start"].as<double>(),
                n["end"].as<double>(),
                n["position"].as<int>()
            });
        }
        return e;
    }

    void SetUp() {
        TestUtil::clearDB();
        TestLaneClosure testcase;
        testcase.generateInput();
        std::filesystem::path dbfile = testcase.filename();

        YAML::Node fileConfig = YAML::LoadFile(testcase.filename().string());
        fileConfig["logtype"] = "file";
        fileConfig["logdir"] = "environment-comparison/";
        TestUtil::configToFile(fileConfig,"envTest_filelog.yaml" );

        getEnvironment("envTest_filelog.yaml");
        getEnvironment(dbfile);
    }

    void TearDown() {
        TestUtil::conditionalFileCleanup({"envTest_filelog.yaml", TestLaneClosure().filename()});
        TestUtil::conditionalFolderCleanup(std::filesystem::path("environment-comparison/"));
    }
    
};

// Compare File vs DB
TEST_F(EnvironmentComparison, FileDBComparison){
    DBManager reader(true);

    Environment db;
    extract<Environment>([&reader](){return reader.queryEnvironment(TestLaneClosure().testName());}, db, "Query DB environment");
    Environment file = fromYaml("environment-comparison/environment.yml");

    EXPECT_DOUBLE_EQ(file.x0, db.x0);
    EXPECT_DOUBLE_EQ(file.xf,  db.xf);
    EXPECT_EQ(file.nlanes,  db.nlanes);
    
    for (auto [f,d] : std::views::zip(file.segments_, db.segments_)){
        EXPECT_EQ(f.start, d.start);
        EXPECT_EQ(f.end, d.end);
        EXPECT_EQ(f.rate, d.rate);
        EXPECT_EQ(f.position, d.position);
    }

    for (auto [f,d] : std::views::zip(file.emptySegments_, db.emptySegments_)){
        EXPECT_EQ(f.start, d.start);
        EXPECT_EQ(f.end, d.end);
        EXPECT_EQ(f.position, d.position);
    }

};

class EnvironmentTest : public EnvironmentComparison {

    void SetUp() override {
        // Algorithm Test environment
        TestLaneClosure testcase;
        testcase.generateInput();
        std::filesystem::path dbfile = testcase.filename();

        YAML::Node fileConfig = YAML::LoadFile(testcase.filename().string());
        fileConfig["logtype"] = "file";
        fileConfig["logdir"] = "algorithm/";
        TestUtil::configToFile(fileConfig, "algorithm.yaml");


        // Passing lane environment
        YAML::Node cfg;
        cfg["jobname"] = "passing-lane";
        cfg["type"] = "continuous";
        YAML::Node rightlane;
        cfg["lanes"][0]["flow"]["rate"] = 1500;
        cfg["lanes"][0]["start"] = 0;
        cfg["lanes"][0]["end"] = 2000;
        cfg["lanes"][0]["position"] = 0; // right lane

        cfg["lanes"][1]["flow"]["rate"] = 0;
        cfg["lanes"][1]["start"] = 300;
        cfg["lanes"][1]["end"] = 1700;
        cfg["lanes"][1]["position"] = 1; // right lane

        TestUtil::configToFile(cfg, "passing-lane.yaml");
        getEnvironment("algorithm.yaml");
        getEnvironment("passing-lane.yaml");
    }

    void TearDown() override {
        // Algorithm config file is generated from lane closure algorithm test which needs to also be cleaned up
        TestUtil::conditionalFileCleanup(std::vector<std::string>{"passing-lane.yaml", "algorithm.yaml", TestLaneClosure().filename()});
        TestUtil::conditionalFolderCleanup(std::vector<std::filesystem::path>{"algorithm", "passing-lane"});
    }

};

TEST_F(EnvironmentTest, LaneClosure){
        std::vector<Environment::LaneSegment> segments({{0, 2000, 200, 0}, 
                                                        {5000, 10000, 200, 0},
                                                        {0, 10000, 400, 1},
                                                        {0, 10000, 600, 2},
                                                        {6000, 10000, 0, 3}});

        std::vector<Environment::EmptySegment> empty({{2000, 5000, 0}, 
                                                      {0, 6000, 3}}); 

        Environment env = EnvironmentTest::fromYaml("algorithm/environment.yml");
        
        EXPECT_DOUBLE_EQ(env.x0, 0);
        EXPECT_DOUBLE_EQ(env.xf, 10000);
        EXPECT_EQ(env.nlanes, 4);
        
        ASSERT_EQ(env.segments_.size(), 5) << "Number of road segments found: " << env.segments_.size();
        for (auto i : std::views::iota(0,5)){
            EXPECT_EQ(env.segments_[i].start, segments[i].start);
            EXPECT_EQ(env.segments_[i].end, segments[i].end);
            EXPECT_EQ(env.segments_[i].rate, segments[i].rate);
            EXPECT_EQ(env.segments_[i].position, segments[i].position);
        }

        ASSERT_EQ(env.emptySegments_.size(), 2) << "Number of empty segments found: " << env.emptySegments_.size();
        for (auto i : std::views::iota(0,2)){
            EXPECT_EQ(env.segments_[i].start, segments[i].start);
            EXPECT_EQ(env.segments_[i].end, segments[i].end);
            EXPECT_EQ(env.segments_[i].rate, segments[i].rate);
            EXPECT_EQ(env.segments_[i].position, segments[i].position);
        }
}


TEST_F(EnvironmentTest, PassingLane){

    std::vector<Environment::LaneSegment> segments({{0, 2000, 1500, 0}, 
                                                    {300, 1700, 0, 1},
                                                    });

    std::vector<Environment::EmptySegment> empty({{0, 300, 1}, 
                                                  {1700, 2000, 1}}); 

    Environment env = EnvironmentTest::fromYaml("passing-lane/environment.yml");
    
    EXPECT_DOUBLE_EQ(env.x0, 0);
    EXPECT_DOUBLE_EQ(env.xf, 2000);
    EXPECT_EQ(env.nlanes, 2);
    
    ASSERT_EQ(env.segments_.size(), 2) << "Number of road segments found: " << env.segments_.size();
    for (auto i : std::views::iota(0,2)){
        EXPECT_EQ(env.segments_[i].start, segments[i].start);
        EXPECT_EQ(env.segments_[i].end, segments[i].end);
        EXPECT_EQ(env.segments_[i].rate, segments[i].rate);
        EXPECT_EQ(env.segments_[i].position, segments[i].position);
    }

    ASSERT_EQ(env.emptySegments_.size(), 2) << "Number of empty segments found: " << env.emptySegments_.size();
    for (auto i : std::views::iota(0,2)){
        EXPECT_EQ(env.segments_[i].start, segments[i].start);
        EXPECT_EQ(env.segments_[i].end, segments[i].end);
        EXPECT_EQ(env.segments_[i].rate, segments[i].rate);
        EXPECT_EQ(env.segments_[i].position, segments[i].position);
    }
}
