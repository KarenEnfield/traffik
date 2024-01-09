#include "tfk_clients.h"
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

struct ClientConfig {
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

void on_connect(uv_connect_t* req, int status);
void on_write(uv_write_t* req, int status);
void on_close(uv_handle_t* handle);
void on_timer(uv_timer_t* timer);
void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

static void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    // Allocate a buffer for reading data
    buf->base = static_cast<char*>(malloc(suggested_size));
    buf->len = suggested_size;
}

struct BufferData {
    uv_tcp_t* client;
    std::vector<char> data;
};

void on_connect(uv_connect_t* req, int status) {

    ClientConfig * clientConfig = static_cast<ClientConfig*>(req->data);

    clientConfig->total_sends++;
    if (status < 0) {
        clientConfig->consecutive_failures++;
        clientConfig->log->logWarn ("{}:{} - {}({}) - {} client attempt(s)", clientConfig->ip,  clientConfig->port, uv_strerror(status) ,status, clientConfig->consecutive_failures); 

    } 
    else {
        // Connection established, now set up reading
        uv_read_start(reinterpret_cast<uv_stream_t*>(req->handle), alloc_buffer, on_read);

        // Send a write request; 
        status = uv_write(clientConfig->write_req.get(), reinterpret_cast<uv_stream_t*>(req->handle), &clientConfig->buf, 1, on_write);
        if (status)
        {
            clientConfig->consecutive_failures++;
            clientConfig->log->logError("write request {}({}) - {} client attempt(s)", uv_strerror(status), status, clientConfig->consecutive_failures);
        }
        
    }        

}


void on_write(uv_write_t* req, int status) {
    
    ClientConfig * clientConfig = static_cast<ClientConfig*>(req->data);
    
    if (status < 0) {
        clientConfig->consecutive_failures++;
        clientConfig->log->logWarn("{}({}) - {} client attempts", uv_strerror(status), status, clientConfig->consecutive_failures);
    }
    else   
    { 
        clientConfig->successful_sends++;
        clientConfig->consecutive_failures = 0;
        clientConfig->log->logInfo("client send ({})", clientConfig->successful_sends);
    }
    
    uv_close(reinterpret_cast<uv_handle_t*>(req->handle), on_close);

}

void on_close(uv_handle_t* handle) {
    //dont delete tcp handle here
}

void on_timer(uv_timer_t* timer) {
    
    // Get server configuration from handle data
    ClientConfig* clientConfig = static_cast<ClientConfig*>(uv_handle_get_data(reinterpret_cast<uv_handle_t*>(timer)));
    
    uv_tcp_t* handle = clientConfig->handle.get();
    
    
    // Stop continuous sending if consecutive failures exceed a threshold
    const int maxconsecutive_failures = -1;
    if (maxconsecutive_failures>0 && clientConfig->consecutive_failures >= maxconsecutive_failures) {
        clientConfig->log->logError("Client stopped due to {} consecutive send failures", clientConfig->successful_sends);
        uv_timer_stop(timer);
        return;
    }

    // Stop continuous sending if the maximum number of sends is reached
    if (clientConfig->max_sends > 0 && clientConfig->total_sends >= clientConfig->max_sends) {
        clientConfig->log->logInfo("Client stopped after reaching maximum sends ({})", clientConfig->successful_sends);
        uv_timer_stop(timer);
        return;
    }

    int status;
    // Repeatedly connect to the server

    // Create a tcp handle
    status = uv_tcp_init(clientConfig->loop, handle);
    if (status) {
        clientConfig->log->logError("client timer init {}({})", uv_strerror(status), status);
    }
       
    // Target server address and port
    status = uv_ip4_addr(clientConfig->ip.c_str(), clientConfig->port, &(clientConfig->inaddr));
    if (status) {
        clientConfig->log->logError("client ipv4 addr {}:{} - {}({}) ",clientConfig->ip , clientConfig->port, uv_strerror(status), status);
    }
    // Create a connect request
    uv_connect_t* connect_req = static_cast<uv_connect_t*>(new uv_connect_t);
    
    // Assign the data
    connect_req->data = clientConfig;

    // Initiate the connection
    status = uv_tcp_connect(connect_req, handle, reinterpret_cast<const struct sockaddr*>(&clientConfig->inaddr), on_connect);
    if (status) {
        clientConfig->log->logError("client tcp_connect {}({})", uv_strerror(status), status);
    }
}

// Callback for handling data read from the server
void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
        
    if (nread > 0) {
        // Data is available, process it here
        ClientConfig* clientConfig = static_cast<ClientConfig*>(uv_handle_get_data(reinterpret_cast<uv_handle_t*>(stream)));
        clientConfig->log->logDebug("client read ({}) data bytes",nread);
    } else if (nread < 0) {
        // An error or EOF occurred
        if (nread != UV_EOF) {
            ClientConfig* clientConfig = static_cast<ClientConfig*>(uv_handle_get_data(reinterpret_cast<uv_handle_t*>(stream)));
            clientConfig->log->logError("client on read {}", uv_strerror(nread));
        }
        // Close the stream (TCP connection)
        uv_close(reinterpret_cast<uv_handle_t*>(stream), nullptr);
    }

    // Release the buffer since it's no longer needed
    free(buf->base);
    
}

tfk_clients::tfk_clients(uv_loop_t * dl):loop(dl==nullptr?uv_default_loop():dl)
{
    // Contains list of clients to run, from a JSON format
    json config;

    // Determine list of clients, if specified, else run a default client
    // Get the value of the TFK_CONFIG environment variable
    const char* configFilePath = std::getenv("TFK_CONFIG");
    
    if (configFilePath) {
        // Now you can use 'configFilePath' in your code

        // Read client configurations from a JSON file
        std::ifstream file(configFilePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open client config file" << std::endl;
            return;
        }
        try {    
            file >> config;
        } catch (const std::exception& e) {
            std::cerr << "Error parsing JSON client configuration: " << e.what() << std::endl;
            return;
        }
    } 
    // else no clients to process ??  Lets send one client to the local server to create traffik
    else
    {   // nothing specified in the environment variable, Run one default server
        config = json::parse(R"(
        {
          "clients":[
            {
              "name": "default",
              "url": "127.0.0.1:8080",
              "rate": 1,
              "message": "Hello World",
              "max_sends" : 10
            }
          ]
        })");
    }


    for (const auto& client : config["clients"]) {
        
        std::string name = client["name"];
        std::string url = client["url"];
        int rate = client["rate"];
        std::string message = client["message"];
        int max_sends = client.value("max_sends", -1);
        std::string ip = url;
        int port = 80;

        ClientConfig *clientConfig = new ClientConfig;

        // Create a logger - check for duplicates
        clientConfig->log = std::make_unique<tfk_logger>(name);
        
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
            clientConfig->log->logWarn("client getaddrinfo for url {} - {}({})",url, gai_strerror(status), status);
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
                    
                } 
                #if 0
                else { // IPv6
                    struct sockaddr_in6 *ipv6 = reinterpret_cast<struct sockaddr_in6*>(p->ai_addr);
                    addr = &(ipv6->sin6_addr);
                    // Convert the IP address to a string and print it
                    inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
                    ip = ipstr;
                    port = ipv6->sin6_port;
                }
                #endif
            }
            
            // Free the memory allocated by getaddrinfo
            freeaddrinfo(result);
        }
        std::string tmp = max_sends >=0 ? std::to_string(max_sends) : "infininte" ;
        clientConfig->log->logInfo("Client url {} resolved to {}:{} for {} sends",url, ip, port, tmp);
        // Set client data
        clientConfig->loop = loop;
        clientConfig->name = name;
        clientConfig->url = url;
        clientConfig->rate = rate;
        clientConfig->message = message;
        clientConfig->max_sends = max_sends;
        clientConfig->ip = ip;
        clientConfig->port = port;
        // Create a write buffer for reuse
        clientConfig->buf = uv_buf_init( const_cast<char*>(clientConfig->message.c_str()), sizeof(message));
        clientConfig->successful_sends = 0;
        clientConfig->consecutive_failures = 0;
        clientConfig->total_sends = 0;
            // Create tcp client handle
        clientConfig->handle = std::make_unique<uv_tcp_t>();

        // Add client handle to uv loop for scheduling
        status = uv_tcp_init(loop,  clientConfig->handle.get());
        if (status){
            clientConfig->log->logError("client tcp init {}({})", uv_strerror(status), status);
        }
        // Create a uv timer
        clientConfig->timer = std::make_unique<uv_timer_t>();

        // Add uv_timer to the timed processes
        status = uv_timer_init(loop, clientConfig->timer.get());
        if (status){
            clientConfig->log->logError("client timer init {}({})",uv_strerror(status),status);
        }

        // Create a write request for reuse
        clientConfig->write_req = std::make_unique<uv_write_t>();
        clientConfig->write_req.get()->data = clientConfig;

        // Set the timer data
       // clientConfig->timer.get()->data = (void *)clientConfig->name.c_str(); // Use the name as the key
        uv_handle_set_data(reinterpret_cast<uv_handle_t*>(clientConfig->timer.get()), clientConfig);


        // Start the timer
        status = uv_timer_start(clientConfig->timer.get(), on_timer, 0, static_cast<uint64_t>(1e3 / clientConfig->rate));  
        if (status){
            clientConfig->log->logError("client timer start {}({})",uv_strerror(status),status);
        }

    }

}