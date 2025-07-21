/**
 * @file traffic.cpp
 * @author Ryan Butler
 * @brief  Driver code to run the simulation
 * @version 0.1
 * @date 2025-07-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */
// Main driver code to run a simulation 
#include "parser.hpp"
#include "parserFactory.hpp"
#include "simulator.hpp"
#include <iostream>

int main(int argc, char** argv){

    if (argc != 2){
        std::cerr << "No config file set. usage: ./traffic <config.yaml>" << std::endl;
        return 1;
    }
    
    std::string configfile(argv[1]);
    ParserFactory parserFac(configfile);
    std::shared_ptr<Parser> parser = parserFac.makeParser();
    SimulatorInputs inputs = parser->parse();

    Simulator s(inputs);

    s.run();
}