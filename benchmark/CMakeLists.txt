cmake_minimum_required(VERSION 3.14)

project(monitaal-benchmark LANGUAGES CXX)

add_executable(monitaal-benchmark main.cpp)

target_link_libraries(monitaal-benchmark PRIVATE
        ${Boost_LIBRARIES}
        MoniTAal)

install(TARGETS monitaal-benchmark
        RUNTIME DESTINATION benchmark
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)