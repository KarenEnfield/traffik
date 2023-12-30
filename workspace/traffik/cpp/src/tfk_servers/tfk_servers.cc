#include "tfk_servers.h"
#include "../tfk_common.h"
#include "../tfk_logger/tfk_logger.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <format>
#include <chrono>
#include <unistd.h>

#include <uv.h>
#include <nlohmann/json.hpp>


using json = nlohmann::json;

// Structure to hold server configuration
struct ServerConfig {
    uv_loop_t *loop;
    std::string name;       // Unique name for the server
    int port;
    std::string type;       // "message", "random", "error"
    std::string message;    // Used for "message" type
    size_t dataLength;      // Used for "random" type
    int errorCode;          // Used for "error" type
    std::string duration;   // "continuous", "timed", "timeout"
    int timeoutSeconds;     // Used for "timed" and "timeout" types
    bool isTimeoutExpired;  // Used to track the inactivity timeout
    uv_tcp_t* serverHandle; // Pointer to the server handle
    bool isServerRunning;   // Flag t
    std::unique_ptr<tfk_logger> log;
    uv_timer_t timeoutTimer;// Timer for timeout
    BufferData bufferData;  // Buffer data for reading http requests
    std::string httpResponse; // Response string  
};

void on_server_closed(uv_handle_t* handle);

// Callback for handling HTTP requests
void on_http_request(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    
    // Get server configuration from handle data
    ServerConfig* config = static_cast<ServerConfig*>(uv_handle_get_data(reinterpret_cast<uv_handle_t*>(stream)));

    if (nread > 0) {
        
        try {
            // Get server configuration from handle data
            //ServerConfig* config = static_cast<ServerConfig*>(uv_handle_get_data(reinterpret_cast<uv_handle_t*>(stream)));
        
            // Append received data to the buffer
            config->bufferData.data.insert(config->bufferData.data.end(), buf->base, buf->base + nread);
            
            // Look for the end of an HTTP request (e.g., double newline)
            const char* double_newline = "\r\n\r\n";
            auto end_of_request = std::search(config->bufferData.data.begin(), config->bufferData.data.end(),
                                          double_newline, double_newline + strlen(double_newline));
            
            if (end_of_request != config->bufferData.data.end()) {
                // Process the HTTP request
                

            // Handle different server types
            uv_buf_t response;
            //std::string http_response;

            if (config->type == "message") {
                // Return a standard message
                // Your HTTP processing logic here   

                config->httpResponse =   "HTTP/1.1 200 OK\r\n"
                                    "Content-Type: text/plain\r\n"
                                    "Content-Length: " + std::to_string(config->message.length()) + "\r\n"
                                    "\r\n" + config->message;

                response = uv_buf_init(const_cast<char*>(config->httpResponse.c_str()), config->httpResponse.length());

            } else if (config->type == "random") {
                // Return continuously randomized data
                // Range check
                if ((config->dataLength<0)||(config->dataLength>1000))
                    config->dataLength = 50;
                std::srand(static_cast<unsigned>(std::time(0)));    
                std::string s(config->dataLength,'*');
                for (int i=0; i<config->dataLength; i++)
                    s[i] = 32 + std::rand() % 94;

                config->httpResponse =   "HTTP/1.1 200 OK\r\n"
                                    "Content-Type: text/plain\r\n"
                                    "Content-Length: " + std::to_string(config->dataLength) + "\r\n"
                                    "\r\n" + s;
    
                response = uv_buf_init(const_cast<char*>(config->httpResponse.c_str()), config->httpResponse.length());

            } else if (config->type == "error") {
                // Return an HTTP error code
                config->httpResponse =   "HTTP/1.1 "+ std::to_string(config->errorCode)+ "\r\n"
                                    "Content-Type: text/plain\r\n"
                                    "Content-Length: 0\r\n"
                                    "\r\n";
                response = uv_buf_init(const_cast<char*>(config->httpResponse.c_str()), config->httpResponse.length());
                
           } else {
                //std::cerr << "Unknown server type: " << config->type << std::endl;
                config->log->logError("unknown server type {}", config->type);
                uv_close(reinterpret_cast<uv_handle_t*>(stream), nullptr);
                free(buf->base);
                return;
            }

            
            // Write the response back to the client
            uv_write_t* write_req = new uv_write_t;
            uv_write(write_req, stream, &response, 1, nullptr);
            
            // Handle server duration
            if (config->duration == "timed") {
                // Close the server after a specified duration
                uv_timer_t* timer = static_cast<uv_timer_t*>(malloc(sizeof(uv_timer_t)));
                uv_timer_init(config->loop, timer);
                uv_timer_start(timer, [](uv_timer_t* timer) {
                    uv_close(reinterpret_cast<uv_handle_t*>(timer), nullptr);
                }, config->timeoutSeconds * 1000, 0);
            } else if (config->duration == "timeout") {
                // Set an inactivity timeout for the server
                uv_timer_t* timer = static_cast<uv_timer_t*>(malloc(sizeof(uv_timer_t)));
                uv_timer_init(config->loop, timer);
                uv_timer_start(timer, [](uv_timer_t* timer) {
                    ServerConfig* serverConfig = static_cast<ServerConfig*>(uv_handle_get_data(reinterpret_cast<uv_handle_t*>(timer)));
                    if (!serverConfig->isTimeoutExpired) {
                        serverConfig->isTimeoutExpired = true;
                        uv_close(reinterpret_cast<uv_handle_t*>(timer), nullptr);
                    }
                }, config->timeoutSeconds * 1000, 0);
                uv_handle_set_data(reinterpret_cast<uv_handle_t*>(timer), &config);
            }

            // Clear the buffer for the next request
            config->bufferData.data.erase(config->bufferData.data.begin(), end_of_request + strlen(double_newline));
            // config->bufferData.data.clear();
            }
        }
        catch(const std::exception& e) {
           //  config parsing error, handle as needed
            std::cerr << "Config parsing error: " << e.what() << std::endl;
            config->log->logError("config parsig error {}", e.what());
            uv_close(reinterpret_cast<uv_handle_t*>(stream), on_server_closed);
            
            //delete  bufferData;
        }

    } else if (nread < 0) {
        // An error or EOF occurred
        if (nread != UV_EOF) {
            std::cerr << "Read error: " << uv_strerror(nread) << std::endl;
            config->log->logError("read {}({})", uv_strerror(nread), nread);
        }
        uv_close(reinterpret_cast<uv_handle_t*>(stream), nullptr);
    }

    // Free the buffer since it's no longer needed
    free(buf->base);
}

// Callback for handling new connections
void on_connection(uv_stream_t* server, int status) {
    ServerConfig* serverConfig = static_cast<ServerConfig*>(uv_handle_get_data(reinterpret_cast<uv_handle_t*>(server)));

    if (status < 0) {
        std::cerr << "Connection error: " << uv_strerror(status) << std::endl;
        serverConfig->log->logError("connection {}({})", uv_strerror(status), status);
        return;
    }

    //std::cerr<< "Server " << std::setw(10) << std::left << serverConfig->name << " got pinged!\n";
    serverConfig->log->logDebug("got pinged");
    uv_tcp_t* client = static_cast<uv_tcp_t*>(malloc(sizeof(uv_tcp_t)));
    uv_tcp_init(serverConfig->loop, client);

    if (uv_accept(server, reinterpret_cast<uv_stream_t*>(client)) == 0) {
        // Set up the buffer data for this client
        BufferData* bufferData = new BufferData{ client, {} };
        
        //uv_handle_set_data(reinterpret_cast<uv_handle_t*>(client), bufferData);
        uv_handle_set_data(reinterpret_cast<uv_handle_t*>(client), server->data);

        // Set up the read callback
        uv_read_start(reinterpret_cast<uv_stream_t*>(client), [](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
            *buf = uv_buf_init(new char[suggested_size], static_cast<unsigned int>(suggested_size));
        }, on_http_request);
        //uv_read_start(reinterpret_cast<uv_stream_t*>(client), alloc_buffer, on_http_request);
    } else {
        uv_close(reinterpret_cast<uv_handle_t*>(client), nullptr);
    }
}

// Callback for when the server handle is closed
void on_server_closed(uv_handle_t* handle) {
    ServerConfig* serverConfig = static_cast<ServerConfig*>(uv_handle_get_data(handle));

    // std::cerr << "Server " << serverConfig->name << " has stopped." << std::endl;
    serverConfig->log->logInfo("stopped");
    serverConfig->isServerRunning = false; // Update the flag
    
    // Stop the timeout timer if it's still active
    uv_timer_stop(&serverConfig->timeoutTimer);

    free(serverConfig);
}


// Callback for the timeout timer
void on_timeout(uv_timer_t* timer) {
    ServerConfig* serverConfig = static_cast<ServerConfig*>(uv_handle_get_data(reinterpret_cast<uv_handle_t*>(timer)));

    if (serverConfig) {
        // std::cerr << "Server " << serverConfig->name << " timed out and stopped listening." << std::endl;
        serverConfig->log->logInfo("server timed out and stopped listening");
        // Close the server handle
        uv_close(reinterpret_cast<uv_handle_t*>(serverConfig->serverHandle), on_server_closed);

        // Stop the timeout timer
        uv_timer_stop(timer);
    }
    else {
        //std::cerr << "Error: Server configuration is missing for the timeout timer." << std::endl;
        serverConfig->log->logError("server configuration is missing for the timeout timer");
    }
}

// Callback for handling a SIGINT signal (e.g., Ctrl+C)
void on_signal_server(uv_signal_t* handle, int signum) {

    ServerConfig *serverConfig =  static_cast<ServerConfig*>(uv_handle_get_data(reinterpret_cast<uv_handle_t*>(&handle)));
    std::cerr << "Received Ctrl+C. Cleaning up servers and exiting..." << std::endl;
    serverConfig->log->logInfo("Received Ctrl+C. Cleaning up servers and exiting...");
    //std::string message = "Received Ctrl+C. Cleaning up and exiting...\n";
    //write(STDERR_FILENO, message.c_str(), message.length());
    
    // Close the server handle and stop the event loop
    // uv_close(reinterpret_cast<uv_handle_t*>(&handle), on_server_closed);

    
    // moved it here from below
    uv_signal_stop(handle);

    // Stop the event loop when a SIGINT signal is received
    uv_stop(handle->loop);

}


tfk_servers::tfk_servers(uv_loop_t* dl):loop(dl==nullptr?uv_default_loop():dl) {

    std::string filename = "../data/input/tfk_servers_config.json";
    // Read server configurations from a JSON file
    std::ifstream config_file(filename.c_str());
    if (!config_file) {
        std::cerr << "Error opening config file: " << filename<< std::endl;
   
        return;
        //return 1;
    }

    json server_configs;
    try {
        config_file >> server_configs;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return;
    }

    // Create HTTP servers based on configurations
    for (const auto& config : server_configs["servers"]) {
        
        // Required keys
        std::string name = config["name"];
        int port = config["port"];

        // Set default key values
        std::string type = "message";//config["type"];            // "message", "random", "error"
        std::string message = name;//config["message"];      // Used for "message" type
        unsigned int dataLength = 50;//config["dataLength"];// Used for "random" type
        int errorCode = 200; //config["errorCode"];          // Used for "error" type
        std::string duration = "continuous";//config["duration"];    // "continuous", "timed", "timeout"
        int timeoutSeconds = -1;            //config["timeoutSeconds"];// Used for "timed" and "timeout" types
        
        std::unique_ptr<tfk_logger> log_ptr = std::make_unique<tfk_logger>(name);
           
        
        // Safely check for the existence of other keys
        if (auto it = config.find("type"); it != config.end() && it->is_string()) {
            type = *it;
        } else {
            //std::cerr << "Warning: Missing or invalid 'type' key in server configuration.  Using '" << type << "'." << std::endl;
            log_ptr->logWarn( "missing or invalid 'type' key in server configuration; using default {}", type);
        }
        
        if (type=="message"){
            if (auto it = config.find("message"); it != config.end() && it->is_string()) {
                message = *it;
            } else {
                //std::cerr << "Warning: Missing or invalid 'message' key in server '"<< name <<"' configuration: " << type << ". Using '" << message << "'." << std::endl;
                log_ptr->logWarn("missing or invalid 'message' key in server configuration; using default '{}'", message );
            }
        }

        if (type=="random"){
            if (auto it = config.find("dataLength"); it != config.end() && it->is_number()) {
                dataLength = *it;
            } else {   
                //std::cerr << "Warning: Missing or invalid 'dataLength' key in server '"<< name <<"' configuration: "<< type  << ". Using '" << dataLength << "'." << std::endl;
                log_ptr->logWarn("missing or invalid 'dataLength' key in server configuration; using default {}");
            }
        }

        if (type=="error"){
            if (auto it = config.find("errorCode"); it != config.end() && it->is_number()) {
                errorCode = *it;
            } else {
                //std::cerr << "Warning: Missing or invalid 'errorCode' key in server '"<< name <<"' configuration: "<< type  << ". Using '" << errorCode << "'." << std::endl;
                log_ptr->logWarn("missing or invalid errorCode: using default {}", errorCode);
            }
        }
        

        if (auto it = config.find("duration"); it != config.end() && it->is_string()) {
            duration = *it;
        } else {
            //std::cerr << "Warning: Missing or invalid 'duration' key in server '"<< name <<"' configuration: "<< type  << ".  Using '" << duration << "'." << std::endl;
            log_ptr->logWarn("missing of invalid 'duration' key; using default {}", duration);
            //continue;
        }

        if (duration!="continuous")
        {
            if (auto it = config.find("timeoutSeconds"); it != config.end() && it->is_number()) {
                timeoutSeconds = *it;
            } else {
                //std::cerr << "Warning: Missing or invalid 'timeoutSeconds' key in server '"<< name <<"' configuration: "<< type  << ".  Using '" << timeoutSeconds << "'." << std::endl;
                log_ptr->logWarn("missing or invalid 'timeoutSeconds' key; uing default {}",timeoutSeconds );
                //continue;
            }
        }

        // Create server only if all required keys are present
        uv_tcp_t* server = static_cast<uv_tcp_t*>(malloc(sizeof(uv_tcp_t)));
        uv_tcp_init(loop, server);

        struct sockaddr_in bind_addr;
        uv_ip4_addr("0.0.0.0", port, &bind_addr);

        int bind_result = uv_tcp_bind(server, reinterpret_cast<struct sockaddr*>(&bind_addr), 0);
        if (bind_result ==0)
        {
            // Detailed Server is running message
            log_ptr->logInfo("is listening on port {}, running {}, responding with {} {}", port, duration, type, type=="error" ? std::to_string(errorCode) : (type=="random" ? std::to_string(dataLength):""));
            
            // Store server configuration in the server handle
            ServerConfig* serverConfig = new ServerConfig{ loop, name, port, type, message, dataLength, errorCode, duration, timeoutSeconds, false, server, true, std::move(log_ptr), nullptr}; 
            uv_handle_set_data(reinterpret_cast<uv_handle_t*>(server), serverConfig);

            // Set up timeout timer if necessary
            if (duration == "timeout" || duration == "timed") {
                // Duration is in seconds
                timeoutSeconds = config["timeoutSeconds"];
                serverConfig->timeoutSeconds = timeoutSeconds;

                // Initialize and start the timer
                uv_timer_init(loop, &serverConfig->timeoutTimer);
                uv_handle_set_data(reinterpret_cast<uv_handle_t*>(&serverConfig->timeoutTimer), serverConfig);
                uv_timer_start(&serverConfig->timeoutTimer, on_timeout, timeoutSeconds * 1000, 0);
            }

            uv_listen(reinterpret_cast<uv_stream_t*>(server), SOMAXCONN, on_connection);
        }
        else
        {
            std::cerr << "Error binding server " << name << " to port " << port << ": " << uv_strerror(bind_result) << std::endl;
            uv_close(reinterpret_cast<uv_handle_t*>(server), nullptr);
            free(server);
        }
    }
}