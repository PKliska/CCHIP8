cmake_minimum_required(VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

project(CCHIP8 VERSION 1.0)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

include(FindPkgConfig)
pkg_search_module(SDL2 REQUIRED sdl2)
include_directories()


configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h)
include_directories(include)

add_subdirectory(src)

option(ENABLE_TESTS "Perform unit tests after build" ON)
if (ENABLE_TESTS)
    include(CTest)
    list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
    add_subdirectory(tests)
endif (ENABLE_TESTS)

option(BUILD_DOC "Build documentation" ON)
if(BUILD_DOC)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        add_custom_target(doc
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Generate documentation with Doxygen"
                VERBATIM )
    else(DOXYGEN_FOUND)
        message("Doxygen not found.")
    endif(DOXYGEN_FOUND)
endif(BUILD_DOC)
