#include <memory>

#include "api/appComponent.hpp"
#include "api/Controller/controller.hpp"
#include "oatpp/network/Server.hpp"


void run() {

    // Registers these as COMPONENTS! (CRTP?)
    AppComponent components;
    // Makes a local var called "router" 
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    auto controller = std::make_shared<Controller>();
    router->addController(controller);

    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);
  
    oatpp::network::Server server(connectionProvider, connectionHandler);

    /* Print info about server port */
    char* x = (char*)connectionProvider->getProperty("port").getData();
    OATPP_LOGI("MyApp", "Server running on port %s", x );

    /* Run server */
    server.run();
}

int main(){
    /* Init oatpp Environment */
    oatpp::base::Environment::init();

    /* Run App */
    run();

    /* Destroy oatpp Environment */
    oatpp::base::Environment::destroy();
    return 0;
}