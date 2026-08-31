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

class EnvironmentTest : public ::testing::Test {

    protected:
    static void getEnvironment(std::string cfg){
        ParserFactory parserFac(cfg);
        std::expected<SimulatorInputs, std::string> parseResult = parserFac.makeParser().and_then(std::mem_fn(&Parser::parse)).value();
        ASSERT_TRUE(parseResult.has_value()) << "Unable to parse config: " << cfg << " " << parseResult.error();
        SimulatorInputs inputs = parseResult.value();
        auto result = inputs.logger_->logEnvironment(inputs.highway_->environment());
        ASSERT_TRUE(result.has_value()) << "Unable to generate environment for config " << cfg << result.error();
    }

    static Environment fromYaml(std::string path){
        YAML::Node env = YAML::LoadFile(path);
        Environment e;

        e.nlanes = size_t(env["number-of-lanes"].as<int>());
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

    static void SetUpTestSuite() {
        TestUtil::clearDB();
        TestLaneClosure testcase;
        testcase.generateInput();
        std::filesystem::path dbfile = testcase.filename();

        YAML::Node fileConfig = YAML::LoadFile(testcase.filename().string());
        fileConfig["logtype"] = "file";
        fileConfig["logdir"] = "environmentTest/";
        TestUtil::configToFile(fileConfig,"envTest_filelog.yaml" );

        getEnvironment("envTest_filelog.yaml");
        getEnvironment(dbfile);
    }

    static void TearDownTestSuite() {
        TestUtil::conditionalFileCleanup({"envTest_filelog.yaml", TestLaneClosure().filename()});
        TestUtil::conditionalFolderCleanup("environmentTest");
    }
    
};

// Compare File vs DB
TEST_F(EnvironmentTest, Comparison){
    DBManager reader(true);

    Environment db;
    extract<Environment>([&reader](){return reader.queryEnvironment(TestLaneClosure().testName());}, db, "Query DB environment");
    Environment file = EnvironmentTest::fromYaml("environmentTest/environment.yml");

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

// TEST_F(EnvironmentTest, LaneClosure){

// }
