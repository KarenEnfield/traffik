# traffik

Traffik is a network traffic generator, creating tcp network traffic for the purposes of observability.

Traffik creates clients that send periodic messages to urls, and  servers that respond to requests, as determined by a predefined JSON file.  Clients and servers may also be created by Traffik that send and respond to one another!

Clients may be configured to send a fixed or indefinite number of messages at a periodic rate.

Servers may be configured to respond with either a fixed html page with a message, an html page with randomized bytes of a specified length, or with an http error code.  

Servers may be also configured to run indefinitely, or for a specified amount of time, or until an inactivity timeout has been reached.

In the C++ version of Traffik, default Traffik runs a server on 8080 indefinitely, and a client is created that sends 10 messages to server at 127.0.0.1:8080, unless a TFK_CONFIG environment variable specifies a JSON file to customize the list of clients and servers

In the Go version of Traffik, default Traffic is obtained through 2 predefined go/input/data JSON files (included), unless TFK_CONFIG environment variable specifies a JSON file to customize the list of clients and servers. 

## Configuration

To override the default Traffik behavior, define an environment variable TFK_CONFIG that contains the configuration path file name, a JSON file where clients and servers are pre-defined.  

The "data/input" folders have two JSON files demonstrating client and server JSON samples, and are installed by both Dockerfiles for useage.  You may add your own JSON files in this folder to be copied, and set the TFK_CONFIG environment variable to create both clients and servers from one file.

## Logging 
Logging levels are set at log.Info level by default.
TFK_LOG_LEVEL may be specified in the enviroment to 
"trace", "debug", "info", "warning", "error", "critical", or "off".


## Docker Go Build

    traffik/go>   %  docker build -t traffik -f Dockerfile .
    

## Docker C++ Build

    traffik/cpp>  %  docker build -t traffik -f Dockerfile .


## Standalone C++ Visual Studio (MacOS)
There is a CMakeLists.txt in the traffik folder that references the subfolder /cpp which also contains a CMakeLists.txt 

## Prerequisite to C++ compilation
C++17 is used
libuv must be installed for C++ compilation: 
    %   brew install libuv 

optional spdlog: the open source project "spdlog" can be installed and used for logging with higher time precision, instead of traffik's own logging mechanism.
    Install spdlog in (macOS)
        %   brew install spdlog 
    CMakeLists.txt, uncomment this line
        add_definitions(
            -DUSE_SPDLOG
            )

optional quiet logging in default compile
CMakeLists.txt, uncomment this line to have no output other than error messages as default
        add_definitions(
            -DLOG_LEVEL_ERROR
        )    

## Command Line overrides (CPP build only)
    "--port int // indicates creation of a server
    --server_type "<string>"
    --message "<string>"
    --timeout <int>
    --data_length <int>
    --url  "<string>" // indicates the creation of a client
    --name "<string>"
    --error_code <int>
    --max_sends <int>
    --rate <int>
    --log_level "<string>"//
    

## File directory structure:

traffik/
|-- CMakeLists.txt  # The root CMakeLists.txt file
|
|-- cpp/
|   |-- Dockerfile
|   |-- docker-compose.yml
|   |-- CMakeLists.txt
|   |
|   |-- src/
|   |   |-- main/
|   |   |   |-- standalone_main.cpp
|   |   |   |-- microservice_main.cpp
|   |   |   |-- common.cpp
|   |   |   |-- common.h
|   |   |-- module1/
|   |   |   |-- source files (.cpp, .h)
|   |   |
|   |   |-- module2/
|   |   |   |-- source files (.cpp, .h)
|   |   |
|   |   |-- ...
|   |
|   |
|   |-- data/
|      |-- input/
|         |-- input1.json
|         |-- input2.json
|         |-- ..
|   |
|   |-- test/
|       |-- module1/
|       |   |-- test files (.cpp)
|       |
|       |-- module2/
|       |   |-- test files (.cpp)
|       |
|       |-- ...
|
|-- go/
|   |-- Dockerfile
|   |-- docker-compose.yml
|   |-- go.mod
|   |    
|   |-- src/
|   |   |-- main/
|   |   |   |-- main.go
|   |   |-- module1/
|   |   |   |-- source files (.go)
|   |   |   |-- source_test files (.go)
|   |   |
|   |   |-- module2/
|   |   |   |-- source files (.go)
|   |   |   |-- source_test files (.go)
|   |   |
|   |   |-- ...
|   |
|   |-- test/
|   |   |-- main.go
|   |
|   |-- main/ 
|   |   |-- main.go
|   |
|   |-- data/
|       |-- input/
|           |-- input1.json
|           |-- input2.json
|           |-- ..
|
|-- microservices/
|   |-- go-microservice/
|   |   |-- main.go
|   |   |-- ...
|   |
|   |-- cpp-microservice/
|       |-- main.cpp
|       |-- ...
|
.
|-- ui/
|   |-- src/
|   |   |-- frontend/
|   |   |   |-- (UI source files, e.g., HTML, CSS, JavaScript, React, etc.)
|   |   |
|   |   |-- backend/
|   |       |-- (Backend for the UI, e.g., Node.js, Express, etc.)
|   |
|   |-- ...
|
|
|-- helm/
|   |-- traffik-chart/
|       |-- Chart.yaml
|       |-- values.yaml
|       |-- templates/
|       |   |-- deployment.yaml
|       |   |-- service.yaml
|       |
|       |-- ...
|
|-- scripts/
|   |-- build.sh (build script for C++)
|   |-- run_tests.sh (run unit tests script for both languages)
|   |-- ...
|
|-- docs/
|   |-- (project documentation)
|
|-- README.md
|-- LICENSE
|-- CMakeLists.txt (project file)
|-- ...


  
    

