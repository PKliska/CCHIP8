find_package(CMocka CONFIG REQUIRED)


add_executable(cchip8_lib_tests cchip8_lib_tests.c)
target_link_libraries(cchip8_lib_tests cchip8-lib ${CMOCKA_LIBRARY})

add_test(cchip8_lib cchip8_lib_tests)
