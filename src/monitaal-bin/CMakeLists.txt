cmake_minimum_required(VERSION 3.14)

find_package(Boost COMPONENTS program_options)

project(MoniTAal-bin LANGUAGES CXX)

add_executable(MoniTAal-bin main.cpp)

target_link_libraries(MoniTAal-bin PRIVATE
        MoniTAal
        ${Boost_LIBRARIES})

install(TARGETS MoniTAal-bin
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)