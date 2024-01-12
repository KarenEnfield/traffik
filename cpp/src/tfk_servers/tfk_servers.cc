#include "tfk_servers.h"
#include "../tfk_logger/tfk_logger.h"

#include <iostream>
#include <chrono>
#include <fmt/core.h>  // For fmtlib
#include <fstream>
#include <memory>
#include <unistd.h>

#include <uv.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

const char * kDefaultServer = R"(
    {       
        "servers":[
            {
                "name": "server",
                "port": 8080,
                "type": "message",
                "message": "Hello World!",
                "data_length" : -1,
                "error_code": 200,
                "duration": "continuous",
                "timeout_seconds": -1
            }
        ]
    }
)";

const char * kServerTemplate = R"(
    {{ 
        "servers":[
            {{
                "name": "{}",
                "port": {},
                "type": "{}",
                "message": "{}",
                "data_length": {},
                "error_code": {},
                "duration": "{}",
                "timeout_seconds": {}
            }}
        ]
    }}
)";


std::string tfk_servers::CreateConfigString(std::string name, int port, std::string serverType, std::string message, int dataLength, int errorCode, std::string duration, int timeout)
{
    std::string retval = fmt::format(kServerTemplate, name, port, serverType, message, dataLength, errorCode, duration, timeout);    
    return retval;
}

// Structure to hold server configuration
struct BufferData {
    uv_tcp_t* client;
    std::vector<char> data;
};
struct ServerConfig {
    uv_loop_t *loop;        // for libuv event looping
    std::string name;       // Unique name for the server
    int port;               // port the server runs on
    std::string type;       // "message", "random", "error"
    std::string message;    // Used for "message" type
    size_t data_length;     // Used for "random" type
    int error_code;         // Used for "error" type
    std::string duration;   // "continuous", "timed", "timeout"
    int timeout_seconds;    // Used for "timed" and "timeout" types
    bool isTimeoutExpired;  // Used to track the inactivity timeout
    uv_tcp_t* serverHandle; // Pointer to the server handle
    bool isServerRunning;   // Flag t
    std::unique_ptr<tfk_logger> log; // logging output
    uv_timer_t timeoutTimer;// Timer for timeout
    BufferData bufferData;  // Buffer data for reading http requests
    std::string httpResponse;   // Response string  
};

// Callback for when the server handle is closed
void on_server_closed(uv_handle_t* handle) {
    ServerConfig* serverConfig = static_cast<ServerConfig*>(uv_handle_get_data(handle));

    serverConfig->log->logInfo("Server stopped");
    serverConfig->isServerRunning = false; // Update the flag
    
    // Stop the timeout timer if it's still active
    uv_timer_stop(&serverConfig->timeoutTimer);

    free(serverConfig);
}

// Callback for handling HTTP requests
void on_http_request(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    
    // Get server configuration from handle data
    ServerConfig* serverConfig = static_cast<ServerConfig*>(uv_handle_get_data(reinterpret_cast<uv_handle_t*>(stream)));

    if (nread > 0) {
        try {
            // Append received data to the buffer
            serverConfig->bufferData.data.insert(serverConfig->bufferData.data.end(), buf->base, buf->base + nread);
            
            // Look for the end of an HTTP request (e.g., double newline)
            const char* double_newline = "\r\n\r\n";
            auto end_of_request = std::search(serverConfig->bufferData.data.begin(), serverConfig->bufferData.data.end(),
                                          double_newline, double_newline + strlen(double_newline));
            
            if (end_of_request != serverConfig->bufferData.data.end()) {
                // Process the HTTP request              
                // Handle different server types
                uv_buf_t response;

                if (serverConfig->type == "message") {
                    // Return a standard message
                    serverConfig->httpResponse =   "HTTP/1.1 200 OK\r\n"
                                        "Content-Type: text/plain\r\n"
                                        "Content-Length: " + std::to_string(serverConfig->message.length()) + "\r\n"
                                        "\r\n" + serverConfig->message;

                    response = uv_buf_init(const_cast<char*>(serverConfig->httpResponse.c_str()), serverConfig->httpResponse.length());

                } else if (serverConfig->type == "random") {
                    // Return continuously randomized data
                    // Range check
                    if ((serverConfig->data_length<0)||(serverConfig->data_length>1000))
                        serverConfig->data_length = 50;
                    std::srand(static_cast<unsigned>(std::time(0)));    
                    std::string s(serverConfig->data_length,'*');
                    for (int i=0; i<serverConfig->data_length; i++)
                        s[i] = 32 + std::rand() % 94;

                    serverConfig->httpResponse =   "HTTP/1.1 200 OK\r\n"
                                        "Content-Type: text/plain\r\n"
                                        "Content-Length: " + std::to_string(serverConfig->data_length) + "\r\n"
                                        "\r\n" + s;
        
                    response = uv_buf_init(const_cast<char*>(serverConfig->httpResponse.c_str()), serverConfig->httpResponse.length());

                } else if (serverConfig->type == "error") {
                    // Return an HTTP error code
                    serverConfig->httpResponse =   "HTTP/1.1 "+ std::to_string(serverConfig->error_code)+ "\r\n"
                                        "Content-Type: text/plain\r\n"
                                        "Content-Length: 0\r\n"
                                        "\r\n";
                    response = uv_buf_init(const_cast<char*>(serverConfig->httpResponse.c_str()), serverConfig->httpResponse.length());
                    
                } else {
                    //std::cerr << "Unknown server type: " << serverConfig->type << std::endl;
                    serverConfig->log->logError("server type {} unhandled", serverConfig->type);
                    uv_close(reinterpret_cast<uv_handle_t*>(stream), nullptr);
                    free(buf->base);
                    return;
                }

                // Write the response back to the client
                uv_write_t* write_req = new uv_write_t;
                uv_write(write_req, stream, &response, 1, nullptr);
                
                // Handle server duration
                if (serverConfig->duration == "timed") {
                    // Close the server after a specified duration
                    uv_timer_t* timer = static_cast<uv_timer_t*>(malloc(sizeof(uv_timer_t)));
                    uv_timer_init(serverConfig->loop, timer);
                    uv_timer_start(timer, [](uv_timer_t* timer) {
                        uv_close(reinterpret_cast<uv_handle_t*>(timer), nullptr);
                    }, serverConfig->timeout_seconds * 1000, 0);
                } else if (serverConfig->duration == "timeout") {
                    // Set an inactivity timeout for the server
                    uv_timer_t* timer = static_cast<uv_timer_t*>(malloc(sizeof(uv_timer_t)));
                    uv_timer_init(serverConfig->loop, timer);
                    uv_timer_start(timer, [](uv_timer_t* timer) {
                        ServerConfig* serverConfig = static_cast<ServerConfig*>(uv_handle_get_data(reinterpret_cast<uv_handle_t*>(timer)));
                        if (!serverConfig->isTimeoutExpired) {
                            serverConfig->isTimeoutExpired = true;
                            uv_close(reinterpret_cast<uv_handle_t*>(timer), nullptr);
                        }
                    }, serverConfig->timeout_seconds * 1000, 0);
                    uv_handle_set_data(reinterpret_cast<uv_handle_t*>(timer), &serverConfig);
                }

                // Clear the buffer for the next request
                serverConfig->bufferData.data.erase(serverConfig->bufferData.data.begin(), end_of_request + strlen(double_newline));
            }
        }
        catch(const std::exception& e) {
           //  config parsing error, handle as needed
            std::cerr << "Config parsing error: " << e.what() << std::endl;
            serverConfig->log->logError("server config parsig error {}", e.what());
            uv_close(reinterpret_cast<uv_handle_t*>(stream), on_server_closed);
        }

    } else if (nread < 0) {
        // An error or EOF occurred
        if (nread != UV_EOF) {
            std::cerr << "Read error: " << uv_strerror(nread) << std::endl;
            serverConfig->log->logError("server read {}({})", uv_strerror(nread), nread);
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
        serverConfig->log->logError("server connection {}({})", uv_strerror(status), status);
        return;
    }

    serverConfig->log->logInfo("server connection");
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

    } else {
        uv_close(reinterpret_cast<uv_handle_t*>(client), nullptr);
    }
}


// Callback for the timeout timer
void on_timeout(uv_timer_t* timer) {
    ServerConfig* serverConfig = static_cast<ServerConfig*>(uv_handle_get_data(reinterpret_cast<uv_handle_t*>(timer)));

    if (serverConfig) {
        serverConfig->log->logInfo("Server timed out and stopped listening");
        // Close the server handle
        uv_close(reinterpret_cast<uv_handle_t*>(serverConfig->serverHandle), on_server_closed);

        // Stop the timeout timer
        uv_timer_stop(timer);
    }
    else {
        serverConfig->log->logError("server configuration is missing for the timeout timer");
    }
}

// Callback for handling a SIGINT signal (e.g., Ctrl+C)
void on_signal_server(uv_signal_t* handle, int signum) {

    ServerConfig *serverConfig =  static_cast<ServerConfig*>(uv_handle_get_data(reinterpret_cast<uv_handle_t*>(&handle)));
    serverConfig->log->logInfo("Servers received Ctrl+C. Clean up and exit...");
    
    uv_signal_stop(handle);

    // Stop the event loop when a SIGINT signal is received
    uv_stop(handle->loop);

}


tfk_servers::tfk_servers(uv_loop_t* dl, const char *configData, const char *logLevel):loop(dl==nullptr?uv_default_loop():dl) {
    
    // Contains list of servers to run, from a JSON format
    json server_configs;

    // Determine the list of servers from environment variable specifying a file path,
    // If nothing specified, start one default server

    // Get the json list from the TFK_CONFIG environment variable
    const char* configFilePath = std::getenv("TFK_CONFIG");
    if (configData!=nullptr)
    {
        server_configs = json::parse(configData);
    }
    else if (configFilePath) {
        // Read server configurations from a JSON file
        std::ifstream config_file(configFilePath);
        if (!config_file.is_open()) {
            std::cerr << "Check environment value TFK_CONFIG.  Failed to open server config file: " << configFilePath<< std::endl;
            return;
        }
        try {
            config_file >> server_configs;
        } catch (const std::exception& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            return;
        }
    } 

    else
    {   // nothing specified in the environment variable, Run one default server
        server_configs = json::parse(kDefaultServer);
    }
  
try{
    // Create HTTP servers based on configurations
    for (const auto& config : server_configs["servers"]) {
        
        // Required keys
        std::string name = config["name"];
        int port = config["port"];

        // Set default key values
        std::string type = "message";//config["type"];            // "message", "random", "error"
        std::string message = name;//config["message"];      // Used for "message" type
        unsigned int data_length = 50;//config["data_length"];// Used for "random" type
        int error_code = 200; //config["error_code"];          // Used for "error" type
        std::string duration = "continuous";//config["duration"];    // "continuous", "timed", "timeout"
        int timeout_seconds = -1;            //config["timeout_seconds"];// Used for "timed" and "timeout" types
        
        std::unique_ptr<tfk_logger> log_ptr = std::make_unique<tfk_logger>(name.c_str(), logLevel);
           
        // Safely check for the existence of other keys
        if (auto it = config.find("type"); it != config.end() && it->is_string()) {
            type = *it;
        } else {
            log_ptr->logWarn( "missing or invalid 'type' key in server configuration; using default {}", type);
        }
        
        if (type=="message"){
            if (auto it = config.find("message"); it != config.end() && it->is_string()) {
                message = *it;
            } else {
                log_ptr->logWarn("missing or invalid 'message' key in server configuration; using default '{}'", message );
            }
        }

        if (type=="random"){
            if (auto it = config.find("data_length"); it != config.end() && it->is_number()) {
                data_length = *it;
            } else {   
               log_ptr->logWarn("missing or invalid 'data_length' key in server configuration; using default {}");
            }
        }

        if (type=="error"){
            if (auto it = config.find("error_code"); it != config.end() && it->is_number()) {
                error_code = *it;
            } else {
                log_ptr->logWarn("missing or invalid error_code: using default {}", error_code);
            }
        }
        

        if (auto it = config.find("duration"); it != config.end() && it->is_string()) {
            duration = *it;
        } else {
            log_ptr->logWarn("missing of invalid 'duration' key; using default {}", duration);
        }

        if (duration!="continuous")
        {
            if (auto it = config.find("timeout_seconds"); it != config.end() && it->is_number()) {
                timeout_seconds = *it;
            } else {
                log_ptr->logWarn("missing or invalid 'timeout_seconds' key; uing default {}",timeout_seconds );
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
            log_ptr->logInfo("Server on port {}, running {}, responding with {}: {}", port, duration, type, type=="error" ? std::to_string(error_code) : (type=="random" ? std::to_string(data_length):message));
            
            // Store server configuration in the server handle
            ServerConfig* serverConfig = new ServerConfig{ loop, name, port, type, message, data_length, error_code, duration, timeout_seconds, false, server, true, std::move(log_ptr), nullptr}; 
            uv_handle_set_data(reinterpret_cast<uv_handle_t*>(server), serverConfig);

            // Set up timeout timer if necessary
            if (duration == "timeout" || duration == "timed") {
                // Duration is in seconds
                timeout_seconds = config["timeout_seconds"];
                serverConfig->timeout_seconds = timeout_seconds;

                // Initialize and start the timer
                uv_timer_init(loop, &serverConfig->timeoutTimer);
                uv_handle_set_data(reinterpret_cast<uv_handle_t*>(&serverConfig->timeoutTimer), serverConfig);
                uv_timer_start(&serverConfig->timeoutTimer, on_timeout, timeout_seconds * 1000, 0);
            }

            int listen_result =uv_listen(reinterpret_cast<uv_stream_t*>(server), SOMAXCONN, on_connection);
            if (listen_result!=0)
            {
               // Detailed Server is running message
                log_ptr->logError("Server is not listening on port {}, {} ({})", port, uv_strerror(listen_result), listen_result);
            }
            
        }
        else
        {
            log_ptr->logError("Server not binding to port {} : {} ({})", port, uv_strerror(bind_result), bind_result);
            uv_close(reinterpret_cast<uv_handle_t*>(server), nullptr);
            free(server);
        }
    }
    }
    catch(const std::exception& e)
    {
        std::cerr << "server" << e.what() << std::endl;
    }
}