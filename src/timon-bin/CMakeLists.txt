cmake_minimum_required(VERSION 3.14)

project(Timon-bin LANGUAGES CXX)

add_executable(Timon-bin main.cpp)

target_link_libraries(Timon-bin PRIVATE
        ${Boost_LIBRARIES}
        Timon)

install(TARGETS Timon-bin
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)