cmake_minimum_required(VERSION 3.17)
project(fixpoint)

set(CMAKE_CXX_STANDARD 20)

set(EXTERNAL_INSTALL_LOCATION "${CMAKE_BINARY_DIR}/external")

FIND_PACKAGE(Boost 1.40 COMPONENTS program_options REQUIRED )
FIND_PACKAGE(pugixml REQUIRED)

INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
INCLUDE_DIRECTORIES( ${PugiXML_INCLUDE_DIR})
link_directories( ${PugiXML_LIB_DIR})

file(MAKE_DIRECTORY ${EXTERNAL_INSTALL_LOCATION})
file(MAKE_DIRECTORY ${EXTERNAL_INSTALL_LOCATION}/lib)
file(MAKE_DIRECTORY ${EXTERNAL_INSTALL_LOCATION}/include)

include(ExternalProject)
ExternalProject_add(pardibaal
    GIT_REPOSITORY git@github.com:DEIS-Tools/PARDIBAAL.git
    GIT_TAG clock-constraint
    CMAKE_ARGS (
        -DBUILD_TESTING=OFF
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
)

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

set(HEADER_FILES
        TA.h
        Parser.h
        types.h
        state_t.h
        Fixpoint.h)

add_library(fixpoint
        ${HEADER_FILES}
        TA.cpp
        Parser.cpp
        state_t.cpp
        Fixpoint.cpp)

target_link_libraries(fixpoint PRIVATE
        pugixml
        -lpardibaal)

install(TARGETS fixpoint
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(FILES ${HEADER_FILES} DESTINATION include/fixpoint)

add_dependencies(fixpoint pardibaal)

add_executable(fixpoint-bin main.cpp)

add_dependencies(fixpoint-bin fixpoint)

target_link_libraries(fixpoint-bin PRIVATE
        ${Boost_LIBRARIES}
        fixpoint)

install(TARGETS fixpoint-bin
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)