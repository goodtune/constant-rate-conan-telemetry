cmake_minimum_required(VERSION 3.22)
project(constant_rate_conan_telemetry)

set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(constant_rate_conan_telemetry main.cpp)

target_link_libraries(constant_rate_conan_telemetry ${CONAN_LIBS})
