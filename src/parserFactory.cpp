#include "parserFactory.hpp"

ParserFactory::ParserFactory(std::filesystem::path cfgpath){
    try {
        cfg_ = YAML::LoadFile(cfgpath);
    } catch(const std::exception& e) {
        std::cout << "Error loading file: " << e.what() << std::endl; 
    }   
}

std::unique_ptr<Parser> ParserFactory::makeParser(){
    std::string simtype = "";
    try {
        std::string simtype = cfg_["type"].as<std::string>();
    } catch(const std::exception& e) {
        std::cerr << "Simulation type must be defined " << '\n';
    }
    
    if (simtype == "continuous"){
        return std::make_unique<ContinuousParser>(cfg_);
    } else if (simtype == "discrete"){
        return std::make_unique<DiscreteParser>(cfg_);
    } else {
        throw InvalidConfigError("\"type\" must me either continuous or discrete");
    }

}