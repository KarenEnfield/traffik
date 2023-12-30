# traffik
Network traffic generator

Traffik allows the creation of clients that send requests to servers, and the creation of servers that respond to clients. Clients and Servers are created from reading a json file. Clients may run a number of times, or indefinitely.
Servers may respond with an error code or an html page of a message or random bytes of a particular length Servers may remain running indefinitely, or until an inactivity timeout has been reached

Dependencies C++ compilation: 
libuv must be installed for compilation: brew install libuv 
optional: spdlog can be installed for logging: brew install spdlog 
If spdlog is not prefered, in tfk_logging.h comment out line //#ifdef USE_SPDLOG and recompile to use source code's own debugging

File structure:

traffik/
|-- CMakeLists.txt  # The root CMakeLists.txt file
|
|-- cpp/
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
|   |-- src/
|   |   |-- main/
|   |   |   |-- main.go
|   |   |-- module1/
|   |   |   |-- source files (.go)
|   |   |
|   |   |-- module2/
|   |   |   |-- source files (.go)
|   |   |
|   |   |-- ...
|   |
|   |-- test/
|       |-- module1/
|       |   |-- test files (.go)
|       |
|       |-- module2/
|       |   |-- test files (.go)
|       |
|       |-- ...
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
|-- data/
|   |-- input/
|       |-- input1.json
|       |-- input2.json
|       |-- ...
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
|-- docker/
|   |-- compose/
|   |   |-- docker-compose.yml
|   |
|   |-- ...
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
|-- ...
