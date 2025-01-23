cmake_minimum_required(VERSION 3.14)

find_package(Boost COMPONENTS program_options)

project(monitaal-benchmark LANGUAGES CXX)

set(MODEL_FILES
        gear_controller_model.h
        b_live_a_freq.h)

add_executable(monitaal-benchmark 
        ${MODEL_FILES}
        main.cpp)

target_link_libraries(monitaal-benchmark PRIVATE
        ${Boost_LIBRARIES}
        MoniTAal)

install(TARGETS monitaal-benchmark
        RUNTIME DESTINATION benchmark
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)