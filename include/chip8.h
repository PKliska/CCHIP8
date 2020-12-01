/**
 * \file
 * \brief Contains interface for CHIP8
 */
#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <time.h>
#include "chip8_timer.h"

typedef enum {
    CHIP8_OK = 0,
    CHIP8_BADINS = 1,
    CHIP8_WAITING_KEYPRESS = 2,
} chip8_cpu_state;

/**
 * \brief Describes state of a CHIP8 chip
 *
 * If cpu is CHIP8_OK, chip is ready to execute another cycle. If cpu is
 * CHIP8_BADINS chip has encountered an invalid instruction and halted. If cpu
 * is CHIP8_WAITING_KEYPRESS, chip is waiting for a call to
 * chip8_set_key_state(). When a key is set to pressed its value is written to
 * the register V.
 */
typedef struct {
    chip8_cpu_state cpu;
    char V;
} chip8_state;

typedef enum {
    CHIP8_KEY_0 = 0x0,
    CHIP8_KEY_1 = 0x1,
    CHIP8_KEY_2 = 0x2,
    CHIP8_KEY_3 = 0x3,
    CHIP8_KEY_4 = 0x4,
    CHIP8_KEY_5 = 0x5,
    CHIP8_KEY_6 = 0x6,
    CHIP8_KEY_7 = 0x7,
    CHIP8_KEY_8 = 0x8,
    CHIP8_KEY_9 = 0x9,
    CHIP8_KEY_A = 0xA,
    CHIP8_KEY_B = 0xB,
    CHIP8_KEY_C = 0xC,
    CHIP8_KEY_D = 0xD,
    CHIP8_KEY_E = 0xE,
    CHIP8_KEY_F = 0xF
} chip8_key;

/**
 * \brief Instance of a CHIP8 chip
 *
 * To create a new instance use new_chip8(). Must be cleaned up with free_chip8()
 */
typedef struct {
    /** V0-VF 8-bit registers */
    unsigned char V[16];
    /**16-bit register*/
    unsigned short I;
    /**Program counter*/
    unsigned short PC;
    /**Stack pointer*/
    unsigned char SP;
    /**Delay timer*/
    chip8_timer DT;
    /**Sound timer*/
    chip8_timer ST;
    /**Stack*/
    unsigned short stack[16];
    /**Chip's RAM*/
    unsigned char memory[4096];
    /**Chip's display*/
    bool display[32][64];
    /**State of keyboard keys. true if pressed, false otherwise*/
    bool keyboard[16];
    /**State of the chip*/
    chip8_state state;
} chip8;

/**
 * Creates a new CHIP8 chip on heap. Every chip that's created must be freed
 * using free_chip8(chip8* chip)
 */
chip8* new_chip8();

/**
 * Execute a single instruction pointed to by PC of CHIP8 chip.
 * \param chip pointer to the chip on which the instruction is to be executed
 */
int chip8_step(chip8* chip);

/**
 * Check if the sound timer of CHIP8 chip is currently active.
 * \param chip pointer to the chip to be checked
 */
bool chip8_st_active(chip8* chip);

/**
 * Sets the state of key \p key on CHIP8 \p chip to \p pressed. If the chip is
 * currently in the state CHIP8_WAITING_KEYPRESS, and \p pressed is true, chip's
 * state is set to CHIP8_OK, and the key value is loaded into the register V
 * @c chip->state.V
 */
void chip8_set_key_state(chip8* chip, chip8_key key, bool pressed);

/**
 * Copies the program into chip's memory at location 0x200 and sets PC to
 * 0x200. After loading the program can be executed using the step(chip8* chip)
 * function.
 * \param chip pointer to chip into which the program is to be loaded
 * \param program pointer to a valid CHIP8 program of len bytes
 * \param len length of the program in bytes
*/
void chip8_load_program(chip8* chip, char* program, size_t len);

/** Frees the specified chip
 * \param chip chip to free
*/
void free_chip8(chip8* chip);

/**
 * Copies the program from the file \p filename into chip's memory at location
 * 0x200 and sets PC to 0x200. After loading the program can be executed using
 * the step(chip8* chip) function.
 * \param chip pointer to chip into which the program is to be loaded
 * \param filename name of the file to be loaded
*/
void chip8_load_program_from_file(chip8* chip, const char* filename);
#endif
