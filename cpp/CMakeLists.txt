cmake_minimum_required(VERSION 3.28.1)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_STANDARD 17)

add_definitions(-DUSE_SPDLOG)

# Common source files for both standalone executable and microservice
set(COMMON_SOURCES
    src/tfk_clients/tfk_clients.cc
    src/tfk_servers/tfk_servers.cc
    src/tfk_logger/tfk_logger.cc
    # Add more common source files as needed
)

# Additional source files for standalone executable
set(EXECUTABLE_SOURCES
    src/main.cc
    # Add more executable-specific source files as needed
)

# Define a common library target
add_library(common_library ${COMMON_SOURCES})

# Define the standalone executable target
add_executable(traffik ${EXECUTABLE_SOURCES})

# Additional configurations for each target (e.g., include directories, compiler flags)
include_directories(/opt/homebrew/include)
find_library(LIBUV_LIBRARY NAMES uv PATHS /opt/homebrew/lib)

#ifdef USE_SPDLOG
find_package(spdlog REQUIRED)
target_link_libraries(traffik PRIVATE common_library ${LIBUV_LIBRARY} spdlog::spdlog )
#else
target_link_libraries(traffik PRIVATE common_library ${LIBUV_LIBRARY} )
#endif

# Link common library to both targets
target_link_libraries(traffik PRIVATE common_library ${LIBUV_LIBRARY} spdlog::spdlog )

