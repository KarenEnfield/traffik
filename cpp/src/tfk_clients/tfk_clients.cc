#include "tfk_clients.h"
#include "../tfk_common.h"
#include "../tfk_logger/tfk_logger.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <unistd.h>

#include <uv.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct ClientData {
    uv_loop_t *loop;
    std::string name;
    std::string url;
    int rate;
    std::string message;
    int max_sends;
    std::string ip;
    int port;
    uv_buf_t buf; 
    uv_buf_t rd_buf;
    sockaddr_in inaddr;
    std::unique_ptr<uv_tcp_t> handle;
    std::unique_ptr<uv_timer_t> timer; 
    std::unique_ptr<uv_write_t> write_req;
    int successful_sends;
    int consecutive_failures;
    unsigned int total_sends;
    std::unique_ptr<tfk_logger> log;
};

std::unordered_map<std::string, std::unique_ptr<ClientData>> clients;

void on_connect(uv_connect_t* req, int status);
void on_write(uv_write_t* req, int status);
void on_close(uv_handle_t* handle);
void on_timer(uv_timer_t* timer);
void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

void on_connect(uv_connect_t* req, int status) {
    auto client_data = static_cast<ClientData*>(req->data);
    client_data->total_sends++;
    if (status < 0) {
        client_data->consecutive_failures++;
        client_data->log->logWarn ("{}:{} - {}({}) - {} attempt(s)", client_data->ip,  client_data->port, uv_strerror(status) ,status, client_data->consecutive_failures); 
    } else {
        // Connection established, now set up reading
        uv_read_start(reinterpret_cast<uv_stream_t*>(req->handle), alloc_buffer, on_read);

        // Send a write request; 
        status = uv_write(client_data->write_req.get(), reinterpret_cast<uv_stream_t*>(req->handle), &client_data->buf, 1, on_write);
        if (status)
        {
            client_data->consecutive_failures++;
            client_data->log->logError("write request {}({}) - {} attempt(s)", uv_strerror(status), status, client_data->consecutive_failures);
        }
    }        
    //delete req; reuse
}


void on_write(uv_write_t* req, int status) {
    auto client_data = static_cast<ClientData*>(req->data);
    
    if (status < 0) {
        client_data->consecutive_failures++;
        client_data->log->logWarn("{}({}) - {} attempts", uv_strerror(status), status, client_data->consecutive_failures);
    }
    else   
    { 
        client_data->successful_sends++;
        client_data->consecutive_failures = 0;
        client_data->log->logInfo("{} successful sends", client_data->successful_sends);
    }
    
    uv_close(reinterpret_cast<uv_handle_t*>(req->handle), on_close);
}

void on_close(uv_handle_t* handle) {
    //uv_tcp_t* tcp_handle = reinterpret_cast<uv_tcp_t*>(handle);
    //dont delete tcp handle here
}

void on_timer(uv_timer_t* timer) {
    auto client_data = clients[reinterpret_cast<std::string*>(timer->data)->c_str()].get();
    uv_tcp_t* handle = client_data->handle.get();
    
    // Stop continuous sending if consecutive failures exceed a threshold
    const int maxconsecutive_failures = -1;
    if (maxconsecutive_failures>0 && client_data->consecutive_failures >= maxconsecutive_failures) {
        client_data->log->logError("stopping client due to {} consecutive send failures", client_data->successful_sends);
        uv_timer_stop(timer);
        return;
    }

    // Stop continuous sending if the maximum number of sends is reached
    if (client_data->max_sends > 0 && client_data->total_sends >= client_data->max_sends) {
        client_data->log->logInfo("stopping client after reaching maximum sends ({})", client_data->successful_sends);
        uv_timer_stop(timer);
        return;
    }

    int status;
    // Repeatedly connect to the server

    // Create a tcp handle
    status = uv_tcp_init(client_data->loop, handle);
    if (status) 
        client_data->log->logError("timer init {}({})", client_data->successful_sends, uv_strerror(status), status);
       
    // Target server address and port
    status = uv_ip4_addr(client_data->ip.c_str(), client_data->port, &(client_data->inaddr));
    if (status) 
        client_data->log->logError("ipv4 addr {}:{} - {}({}) ",client_data->ip , client_data->port, uv_strerror(status), status);
    // Create a connect request
    uv_connect_t* connect_req = static_cast<uv_connect_t*>(new uv_connect_t);
    
    // Assign the data
    connect_req->data = client_data;

    // Initiate the connection
    status = uv_tcp_connect(connect_req, handle, reinterpret_cast<const struct sockaddr*>(&client_data->inaddr), on_connect);
    if (status) 
        client_data->log->logError("tcp_connect {}({})", uv_strerror(status), status);
}

// Callback for handling data read from the server
void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    if (nread > 0) {
        // Data is available, process it here
        ClientData* config = static_cast<ClientData*>(uv_handle_get_data(reinterpret_cast<uv_handle_t*>(stream)));
        config->log->logDebug("{}({})");
    } else if (nread < 0) {
        // An error or EOF occurred
        if (nread != UV_EOF) {
            ClientData* config = static_cast<ClientData*>(uv_handle_get_data(reinterpret_cast<uv_handle_t*>(stream)));
            config->log->logError("on read {}", uv_strerror(nread));
        }
        // Close the stream (TCP connection)
        uv_close(reinterpret_cast<uv_handle_t*>(stream), nullptr);
    }

    // Release the buffer since it's no longer needed
    free(buf->base);
}


tfk_clients::tfk_clients(uv_loop_t * dl):loop(dl==nullptr?uv_default_loop():dl)
{
    std::string filename = "../data/input/tfk_clients_config.json";
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        std::cerr << "Failed to open config.json" << std::endl;
        return;
    }
    
    try {
        
        json config;
        file >> config;
        for (const auto& client : config["clients"]) {
            
            std::string name = client["name"];
            std::string url = client["url"];
            int rate = client["rate"];
            std::string message = client["message"];
            int max_sends = client.value("max_sends", -1);
            std::string ip = "";
            int port = 80;

            clients[name] = std::make_unique<ClientData>();
            ClientData *client_data = clients[name].get();
            
            // Create a logger - check for duplicates
            client_data->log = std::make_unique<tfk_logger>(name);
           
            // Check if the URL has the "http://" prefix and remove it
            std::string cleanedUrl = url;
            if (cleanedUrl.compare(0, 7, "http://") == 0) {
                cleanedUrl.erase(0, 7);
            }

            // Handle "localhost" as a special case
            // Replace "localhost" with "127.0.0.1"
            size_t pos = cleanedUrl.find("localhost");
            if (pos != std::string::npos) {
                cleanedUrl.replace(pos, 9, "127.0.0.1");
                ip = "127.0.0.1";
            }

            // Get/set the ip address from the url
            struct addrinfo hints, *result, *p;
            memset(&hints, 0, sizeof(struct addrinfo));
            hints.ai_family = AF_UNSPEC;    // Allow IPv4 or IPv6
            hints.ai_socktype = SOCK_STREAM; // Use TCP socket type

            size_t colonPos = cleanedUrl.find(":");
            std::string host = (colonPos != std::string::npos) ? cleanedUrl.substr(0, colonPos) : cleanedUrl;
            std::string serviceStr;
            if (colonPos != std::string::npos) {
                serviceStr = cleanedUrl.substr(colonPos + 1);
                port = atoi(serviceStr.c_str());
            } else {
                serviceStr = "http";
            }
            const char * service = serviceStr.c_str();

            // Resolve the URL to IP addresses
            int status = getaddrinfo(host.c_str(), service, &hints, &result);
            if (status != 0) {
                client_data->log->logWarn("getaddrinfo for url {} - {}({})",url, gai_strerror(status), status);
            }
            else
            {
                // Iterate through the list of addresses and print them
                for (p = result; p != NULL; p = p->ai_next) {
                    void *addr;
                    char ipstr[INET6_ADDRSTRLEN];
                    int port = -1;

                    if (p->ai_family == AF_INET) { // IPv4
                        struct sockaddr_in *ipv4 = reinterpret_cast<struct sockaddr_in*>(p->ai_addr);
                        addr = &(ipv4->sin_addr);
                        // Convert the IP address to a string and print it
                        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
                        // Set the new IP
                        ip = ipstr;
                        port = ipv4->sin_port;
                        
                    } else { // IPv6
                        struct sockaddr_in6 *ipv6 = reinterpret_cast<struct sockaddr_in6*>(p->ai_addr);
                        addr = &(ipv6->sin6_addr);
                        // Convert the IP address to a string and print it
                        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
                        ip = ipstr;
                        port = ipv6->sin6_port;
                    }
                    
                }
                
                client_data->log->logInfo("{} resolves to {}:{}",url, ip, port );
                // Free the memory allocated by getaddrinfo
                freeaddrinfo(result);
            }

        
            
            // set client data
            client_data->loop = loop;
            client_data->name = name;
            client_data->url = url;
            client_data->rate = rate;
            client_data->message = message;
            client_data->max_sends = max_sends;
            client_data->ip = ip;
            client_data->port = port;
            // Create a write buffer for reuse
            client_data->buf = uv_buf_init( const_cast<char*>(client_data->message.c_str()), sizeof(message));
            client_data->successful_sends = 0;
            client_data->consecutive_failures = 0;
            client_data->total_sends = 0;
             // Create tcp client handle
            client_data->handle = std::make_unique<uv_tcp_t>();

            // Add client handle to uv loop for scheduling
            status = uv_tcp_init(loop,  client_data->handle.get());
            if (status)
                client_data->log->logError("tcp init {}({})", uv_strerror(status), status);
            // Create a uv timer
            client_data->timer = std::make_unique<uv_timer_t>();
            
            // Add uv_timer to the timed processes
            status = uv_timer_init(loop, client_data->timer.get());
            if (status)
                client_data->log->logError("timer init {}({})",uv_strerror(status),status);

            // Create a write request for reuse
            client_data->write_req = std::make_unique<uv_write_t>();
            client_data->write_req.get()->data = client_data;

            // Set the timer data
            client_data->timer.get()->data = (void *)client_data->name.c_str(); // Use the name as the key
            
            // Start the timer
            status = uv_timer_start(client_data->timer.get(), on_timer, 0, static_cast<uint64_t>(1e3 / client_data->rate));  
            if (status)
                client_data->log->logError("timer start {}({})",uv_strerror(status),status);
        }

        // Run the event loop
        //uv_run(loop, UV_RUN_DEFAULT);

    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON client configuration: " << e.what() << std::endl;
    }
}