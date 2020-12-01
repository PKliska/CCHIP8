file(GLOB SRC_FILES "*.c")
list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/main.c)

add_library(cchip8-lib STATIC ${SRC_FILES})
target_include_directories(cchip8-lib
                            PRIVATE
                            ${CMAKE_BINARY_DIR})
target_link_libraries(cchip8-lib)

add_executable(cchip8 main.c)
target_include_directories(cchip8
                            PRIVATE
                            ${CMAKE_BINARY_DIR}
                            ${SDL2_INCLUDE_DIRS})
target_link_libraries(cchip8 cchip8-lib m ${SDL2_LIBRARIES})

