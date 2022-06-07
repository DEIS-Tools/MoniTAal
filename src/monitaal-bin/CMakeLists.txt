cmake_minimum_required(VERSION 3.14)

project(MoniTAal-bin LANGUAGES CXX)

add_executable(MoniTAal-bin main.cpp)

target_link_libraries(MoniTAal-bin PRIVATE
        ${Boost_LIBRARIES}
        MoniTAal)

install(TARGETS MoniTAal-bin
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)