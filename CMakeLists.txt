cmake_minimum_required(VERSION 3.28.1)

project(traffik)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_STANDARD 17)

#set_property(TARGET traffik PROPERTY CXX_STANDARD 17)

# Common source files for both standalone executable and microservice
set(COMMON_SOURCES
    cpp/src/tfk_clients/tfk_clients.cc
    cpp/src/tfk_servers/tfk_servers.cc
    cpp/src/tfk_logger/tfk_logger.cc
    # Add more common source files as needed
)

# Additional source files for standalone executable
set(EXECUTABLE_SOURCES
    cpp/src/main.cc
    # Add more executable-specific source files as needed
)

# Additional source files for microservice
set(MICROSERVICE_SOURCES
    cpp/src/microservices/cpp-mircoservice/main.cc
    # Add more microservice-specific source files as needed
)

# Define a common library target
add_library(common_library ${COMMON_SOURCES})

# Define the standalone executable target
add_executable(traffik ${COMMON_SOURCES} ${EXECUTABLE_SOURCES})

# Define the microservice target
# add_executable(traffik_ms ${COMMON_SOURCES} ${MICROSERVICE_SOURCES})

# add_executable(traffik main.cc tfk_clients.cc tfk_servers.cc tfk_logger.cc)

# Additional configurations for each target (e.g., include directories, compiler flags)
include_directories(/opt/homebrew/include)
find_library(LIBUV_LIBRARY NAMES uv PATHS /opt/homebrew/lib)
find_package(spdlog REQUIRED)

# Link common library to both targets
target_link_libraries(traffik PRIVATE common_library ${LIBUV_LIBRARY} spdlog::spdlog )
# target_link_libraries(traffik_ms PRIVATE common_library ${LIBUV_LIBRARY} spdlog::spdlog )
# target_link_libraries(traffik PRIVATE ${LIBUV_LIBRARY} spdlog::spdlog )

