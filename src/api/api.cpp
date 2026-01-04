#include <memory>

#include "api/api.hpp"
#include "api/appComponent.hpp"
#include "api/Controller/controller.hpp"
#include "database/databaseInit.hpp"
#include "oatpp/network/Server.hpp"


void TrafficApi::run() {

    if (!initDB::initDB(useTestDB_)){
        std::cerr << "Cannot initialize database!" << std::endl;
        return;
    }
    // Registers these as COMPONENTS! (CRTP?)
    AppComponent components;
    // Makes a local var called "router" 
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    auto controller = std::make_shared<Controller>();
    controller->setDBManager(useTestDB_);
    router->addController(controller);

    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);
  
    oatpp::network::Server server(connectionProvider, connectionHandler);

    /* Print info about server port */
    char* x = (char*)connectionProvider->getProperty("port").getData();
    OATPP_LOGI("Traffic API", "Server running on port %s", x );

    /* Run server */
    server.run([this](){return serverOn_.load();});
}
    
void TrafficApi::closeServer(){
    serverOn_.store(false);
}
