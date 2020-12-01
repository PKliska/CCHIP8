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

static void test_ld1(void** state){
    chip8* chip = *state;

    char program[] = {0x65, 0x56, // ld V5, 0x56
                      0x65, 0x43, // ld V5, 0x43
                        };
    chip8_load_program(chip, program, sizeof program);
    chip8_step(chip);
    assert_int_equal(chip->V[0x5], 0x56);
    chip8_step(chip);
    assert_int_equal(chip->V[0x5], 0x43);
}
static void test_ld2(void** state){
    chip8* chip = *state;

    chip->V[0x3] = 78;
    chip->V[0x4] = 35;

    char program[] = {0x87, 0x30, // ld V7, V3
                      0x83, 0x40, // ld V3, V4
                      0x84, 0x70, // ld V4, V7
                        };
    chip8_load_program(chip, program, sizeof program);
    chip8_step(chip);
    assert_int_equal(chip->V[0x7], 78);
    chip8_step(chip);
    assert_int_equal(chip->V[0x3], 35);
    chip8_step(chip);
    assert_int_equal(chip->V[0x4], 78);
}
static void test_ld3(void** state){
    chip8* chip = *state;

    chip->I = 0x345;

    char program[] = {0xA7, 0x32, // ld I, 0x732
                      };
    chip8_load_program(chip, program, sizeof program);
    chip8_step(chip);
    assert_int_equal(chip->I, 0x732);
}
static void test_ld4(void** state){
    chip8* chip = *state;

    chip8_timer_set(&chip->DT, 134);

    char program[] = {0xFA, 0x07, // ld VA, DT
                      };
    chip8_load_program(chip, program, sizeof program);
    chip8_step(chip);
    assert_int_equal(chip->V[0xA], 134);
}

int main(){
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_load_program),
        cmocka_unit_test(test_ret),
        cmocka_unit_test(test_jp),
        cmocka_unit_test(test_se1),
        cmocka_unit_test(test_se2),
        cmocka_unit_test(test_call),
        cmocka_unit_test(test_cls),

        cmocka_unit_test(test_ld1),
        cmocka_unit_test(test_ld2),
        cmocka_unit_test(test_ld3),
        cmocka_unit_test(test_ld4)

    };
    return cmocka_run_group_tests(tests, setup, teardown);
}
