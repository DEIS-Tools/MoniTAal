cmake_minimum_required(VERSION 3.14)

project(MoniTAal LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(include)

set(HEADER_FILES
        TA.h
        state.h
        Fixpoint.h
        Parser.h
        types.h
        Monitor.h
        EventParser.h
        delay_state_t.h
        symbolic_state_base.h)

add_library(MoniTAal
        ${HEADER_FILES}
        TA.cpp
        state.cpp
        Fixpoint.cpp
        Parser.cpp
        Monitor.cpp
        EventParser.cpp
        delay_state_t.cpp
        symbolic_state_base.cpp)

target_link_libraries(MoniTAal PRIVATE
        -lpugixml
        -lpardibaal)

add_dependencies(MoniTAal pardibaal)

install(TARGETS MoniTAal
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(FILES ${HEADER_FILES} DESTINATION include/monitaal)