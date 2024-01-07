# traffik
Network traffic generator

Traffik allows the creation of clients that send requests to servers, and the creation of servers that respond to clients. Clients and Servers are created from reading a json file. Clients may run a number of times, or indefinitely.
Servers may respond with an error code or an html page of a message or random bytes of a particular length Servers may remain running indefinitely, or until an inactivity timeout has been reached

In C++ build, default traffic is a server on 8080 continuously running, and a client that sends 10 messages to server at 127.0.0.1:8080
In Go build, default traffic is obtained through predefined go/input/data json files

To override the default traffic, assign environment variable TFK_CONFIG to the configuration path file, where clients and servers are defined by their json format.  The respective "data/input" folders have two json files demonstrating client and server json samples, and are installed by both Dockerfiles in a /data/input folder for convenience.  You may add your own json files in this folder and set the TFK_CONFIG environment variable to configure both clients and servers from one file.

Logging levels have not yet been made customizable yet and are currently set at log.Info level.


Dependencies C++ compilation: 
libuv must be installed for compilation: brew install libuv 
optional: spdlog can be installed for logging: brew install spdlog 
If spdlog is not prefered, in tfk_logging.h comment out line //#ifdef USE_SPDLOG and recompile to use source code's own debugging

File structure:

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

Docker Go Build

    traffik/go   % docker build -t traffik -f Dockerfile .
    

Docker C++ Build

    traffik/cpp  % docker build -t traffik -f Dockerfile .

Modify COn    
    

