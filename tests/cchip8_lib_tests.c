#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include "chip8.h"


static int setup(void** state){
    *state = new_chip8();
    return 0;
}
static int teardown(void** state){
    free_chip8(*state);
    return 0;
}

static void test_load_program(void** state){
    chip8* chip = *state;
    char program[] = {};
    size_t len = (sizeof program)/(sizeof program[0]);
    chip8_load_program(chip, program, len);
    assert_int_equal(chip->PC, 0x200);
    assert_memory_equal(chip->memory + 0x200, program, len);
}
static void test_ret(void** state){
    chip8* chip = *state;
    chip->PC = 0x200;
    chip->memory[0x200] = 0x00;
    chip->memory[0x201] = 0xEE;
    chip->SP = 1;
    chip->stack[0] = 0x406;
    chip8_step(chip);
    assert_int_equal(chip->PC, 0x406);
    assert_int_equal(chip->SP, 0);
}
static void test_jp(void** state){
    chip8* chip = *state;
    chip->PC = 0x200;
    chip->memory[0x200] = 0x1A;
    chip->memory[0x201] = 0xBC;
    chip8_step(chip);
    assert_int_equal(chip->PC, 0x0ABC);
}
static void test_se1(void** state){
    chip8* chip = *state;
    chip->PC = 0x200;
    chip->memory[0x200] = 0x30;
    chip->memory[0x201] = 0x42;
    chip->V[0] = 0x42;
    chip8_step(chip);
    assert_int_equal(chip->PC, 0x204);
}
static void test_se2(void** state){
    chip8* chip = *state;
    chip->PC = 0x200;
    chip->memory[0x200] = 0x30;
    chip->memory[0x201] = 0x42;
    chip->V[0] = 0x21;
    chip8_step(chip);
    assert_int_equal(chip->PC, 0x202);
}
static void test_call(void** state){
    chip8* chip = *state;
    chip->PC = 0x200;
    //call 0x208
    chip->memory[0x200] = 0x22;
    chip->memory[0x201] = 0x08;
    chip8_step(chip);
    assert_int_equal(chip->PC, 0x208);
    assert_int_equal(chip->SP, 1);
    assert_int_equal(chip->stack[0], 0x202);
}
static void test_cls(void** state){
    chip8* chip = *state;

    for(int i=0;i<32;i++)
        for(int j=0;j<64;j++)
            chip->display[i][j] = true;

    char program[] = {0x00, 0xE0}; //cls
    chip8_load_program(chip, program, sizeof program);
    chip8_step(chip);
    for(int i=0;i<32;i++)
        for(int j=0;j<64;j++)
            assert_false(chip->display[i][j]);

}

int main(){
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_load_program),
        cmocka_unit_test(test_ret),
        cmocka_unit_test(test_jp),
        cmocka_unit_test(test_se1),
        cmocka_unit_test(test_se2),
        cmocka_unit_test(test_call),
        cmocka_unit_test(test_cls)
    };
    return cmocka_run_group_tests(tests, setup, teardown);
}
