cmake_minimum_required(VERSION 3.28.1)

project(traffik)

add_executable(traffik main.cc tfk_clients.cc tfk_servers.cc tfk_logger.cc)
include_directories(/opt/homebrew/include)
find_library(LIBUV_LIBRARY NAMES uv PATHS /opt/homebrew/lib)
find_package(spdlog REQUIRED)
target_link_libraries(traffik PRIVATE ${LIBUV_LIBRARY} spdlog::spdlog )

set(CMAKE_CXX_COMPILER "clang++")
set_property(TARGET traffik PROPERTY CXX_STANDARD 17)
