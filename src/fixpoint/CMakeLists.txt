cmake_minimum_required(VERSION 3.14)

project(fixpoint-lib LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(include)

set(HEADER_FILES
        TA.h
        state_t.h
        Fixpoint.h
        Parser.h
        types.h)

add_library(fixpoint
        TA.cpp
        state_t.cpp
        Fixpoint.cpp
        Parser.cpp)

target_link_libraries(fixpoint PRIVATE
        pugixml
        -lpardibaal)

add_dependencies(fixpoint pardibaal)

install(TARGETS fixpoint
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(FILES ${HEADER_FILES} DESTINATION include/fixpoint)