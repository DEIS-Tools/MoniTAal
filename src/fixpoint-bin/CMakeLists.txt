cmake_minimum_required(VERSION 3.14)

project(fixpoint-bin LANGUAGES CXX)

add_executable(fixpoint-bin main.cpp)

target_link_libraries(fixpoint-bin PRIVATE
        ${Boost_LIBRARIES}
        fixpoint)

install(TARGETS fixpoint-bin
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)