cmake_minimum_required(VERSION 3.14)

project(MoniTAal LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(include)

INCLUDE_DIRECTORIES( ${PugiXML_INCLUDE_DIRS})

set(HEADER_FILES
        TA.h
        state.h
        Fixpoint.h
        Parser.h
        types.h
        Monitor.h)

add_library(MoniTAal
        TA.cpp
        state.cpp
        Fixpoint.cpp
        Parser.cpp
        Monitor.cpp)

target_link_libraries(MoniTAal PRIVATE
        ${pugixml_libraries}
        -lpardibaal
        -lutap)

add_dependencies(MoniTAal pugixml pardibaal utap)

install(TARGETS MoniTAal
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(FILES ${HEADER_FILES} DESTINATION include/monitaal)