# traffik

traffik is a network traffic generator

Traffik allows the creation of clients that send messages to servers, and the creation of servers that respond to clients.

Clients and servers may be created that send and respond to one another!

Clients and Servers are configured by json definitions. 

Clients may be configured to send a fixed number of messages, or indefinitely at a specified rate.

Servers may be configured to respond with an fixed html page, a randomized return page of a fixed length of bytes, or with an http error code.  
Servers may be also configured to either run indefinitely, or for a specified amount of time, or until an inactivity timeout has been met.

In C++ build, default traffik runs a server on 8080 indefinitely, and a client is created that sends 10 messages to server at 127.0.0.1:8080

In Go build, default traffic is obtained through predefined go/input/data json files (included)

This assumes a TFK_CONFIG environment variable has not been set.

## Configuration

To override the default traffic, define an environment variable TFK_CONFIG to the configuration path file, a JSON file where clients and servers are pre-defined.  

The "data/input" folders have two json files demonstrating client and server json samples, and are installed by both Dockerfiles for useage.  You may add your own json files in this folder to be copied, and set the TFK_CONFIG environment variable to create both clients and servers from one file.

Logging levels have not yet been made customizable and are currently set at log.Info level.



If spdlog is not prefered, in tfk_logging.h comment out line //#ifdef USE_SPDLOG and recompile to use source code's own debugging


## Docker Go Build

    traffik/go>   %  docker build -t traffik -f Dockerfile .
    

## Docker C++ Build

    traffik/cpp>  %  docker build -t traffik -f Dockerfile .
    

## Standalone C++ Visual Studio (MacOS)
There is a CMakeLists.txt in the traffik folder that references the subfolder /cpp which also contains a CMakeLists.txt 

# Prerequisite to C++ compilation
C++17 is used
libuv must be installed for C++ compilation: 
    %   brew install libuv 

optional: spdlog can be installed for logging if SPDLOG is defined before compiling, otherwise, an in-house logging is used: 
    %   brew install spdlog 


## File structure:

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


  
    

