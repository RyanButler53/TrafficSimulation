#include "sim/parserFactory.hpp"

ParserFactory::ParserFactory(std::filesystem::path cfgpath):cfgpath_{cfgpath}{

}

std::expected<std::unique_ptr<Parser>, std::string>ParserFactory::makeParser(){
    try {
        cfg_ = YAML::LoadFile(cfgpath_);
    } catch(const std::exception& e) {
        return std::unexpected(e.what());
    }  

    std::string simtype = "";
    if (cfg_["type"]){
        simtype = cfg_["type"].as<std::string>();
    } else {
        return std::unexpected("type not found");
    }
    
    
    if (simtype == "continuous"){
        return std::make_unique<ContinuousParser>(cfg_, cfgpath_);
    } else if (simtype == "discrete"){
        return std::make_unique<DiscreteParser>(cfg_, cfgpath_);
    } else {
        return std::unexpected("\"type\" must be either continuous or discrete");
    }

}