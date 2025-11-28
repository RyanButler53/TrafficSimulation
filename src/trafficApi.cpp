#include <iostream>
#include "api/api.hpp"  
#include "oatpp/network/Server.hpp"


int main(){
    /* Init oatpp Environment */
    oatpp::base::Environment::init();

    /* Run Api */
    TrafficApi().run();
    /* Destroy oatpp Environment */
    oatpp::base::Environment::destroy();
}