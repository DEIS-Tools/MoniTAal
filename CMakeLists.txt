cmake_minimum_required(VERSION 3.17)
project(monitaal)

set(CMAKE_CXX_STANDARD 20)

set(EXTERNAL_INSTALL_LOCATION "${CMAKE_BINARY_DIR}/external")

OPTION(MONITAAL_BUILD_TEST "Builds tests" OFF)
OPTION(MONITAAL_BUILD_BIN "Builds benchmark binary" OFF)

FIND_PACKAGE(Boost REQUIRED)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

file(MAKE_DIRECTORY ${EXTERNAL_INSTALL_LOCATION})
file(MAKE_DIRECTORY ${EXTERNAL_INSTALL_LOCATION}/lib)
file(MAKE_DIRECTORY ${EXTERNAL_INSTALL_LOCATION}/include)

include(ExternalProject)

ExternalProject_add(pugixml
    GIT_REPOSITORY git@github.com:zeux/pugixml.git
    CMAKE_ARGS (
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DCMAKE_BUILD_TYPE=Release)
)

ExternalProject_add(pardibaal
    GIT_REPOSITORY git@github.com:DEIS-Tools/PARDIBAAL.git
    GIT_TAG main
    CMAKE_ARGS (
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DCMAKE_BUILD_TYPE=Release)
)

set(CMAKE_PREFIX_PATH ${EXTERNAL_INSTALL_LOCATION};${CMAKE_PREFIX_PATH})
include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)


include_directories(include)
add_subdirectory(src/monitaal)
target_include_directories(MoniTAal INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(MONITAAL_BUILD_BIN)
    add_subdirectory(src/monitaal-bin)
endif()
    
if(MONITAAL_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif()