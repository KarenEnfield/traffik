

#include "tfk_clients/tfk_clients.h"
#include "tfk_servers/tfk_servers.h"

#include <uv.h>
#include <iostream>

int main(int argc, char* argv[]){

    uv_loop_t *lp = uv_default_loop();

    // Parse command-line arguments
    if (argc>1){

        std::string name = "default";
        std::string url = "";
        int portNumber = -1;
        std::string serverType="message";
        std::string message = "Hello_World!";
        int dataLength = 50;
        int errorCode = 200;
        std::string duration = "continuous";
        int timeout = -1;
        int maxSends = -1;
        int rate = 1;
        std::string log_level = "";

        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--port") {
                portNumber = std::stoi(argv[++i]); // Convert string to integer
            } else if (arg == "--server_type") {
                serverType = argv[++i];
            } else if (arg == "--message") {
                message = argv[++i];
            } else if (arg == "--timeout") {
                timeout = std::stoi(argv[++i]);
            } else if (arg == "--data_length") {
                dataLength = std::stoi(argv[++i]);
            } else if (arg == "--url") {
                url = argv[++i];
            } else if (arg == "--name") {
                name = argv[++i];
            } else if (arg == "--error_code") {
                errorCode = std::stoi(argv[++i]);
            } else if (arg == "--max_sends") {
                maxSends = std::stoi(argv[++i]);
            } else if (arg == "--rate") {
                rate = std::stoi(argv[++i]);
            } else if (arg == "--log_level") {
                log_level = argv[++i];
            } else {
                std::cerr << "Invalid argument: " << arg << std::endl;
                return 1;
            }       
        }

        // Create Client
        if (url.length()>0){
            std::string configData = tfk_clients::CreateConfigString(name, url, message, rate, maxSends);
            std::cerr << configData<<std::endl;
            tfk_clients c(lp, configData.c_str(), log_level.c_str());
            
        }
        // Create Server
        if (portNumber>0){
            std::string configData = tfk_servers::CreateConfigString(name, portNumber, serverType, message, dataLength, errorCode, duration, timeout);
            tfk_servers s(lp, configData.c_str(), log_level.c_str());
            
        }

        uv_run(lp, UV_RUN_DEFAULT); // loop run can be explicity called

    }

    else {
        // specifying a uv loop is optional, 
        uv_loop_t *lp = uv_default_loop();

        tfk_clients c(lp);
        tfk_servers s(lp);
        uv_run(lp, UV_RUN_DEFAULT); // loop run can be explicity called

    }
    
}
