#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "chip8.h"
#define DEBUG false


static unsigned char DIGITS[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
                             };


chip8* new_chip8(){
    chip8* chip = malloc(sizeof(*chip));
    chip->SP = 0;
    chip8_timer_set(&chip->DT, 0);
    chip8_timer_set(&chip->ST, 0);
    for(int i=0;i<16;i++) chip->V[i] = 0;
    for(int i=0;i<16;i++) chip->keyboard[i] = false;
    memcpy(chip->memory, DIGITS, sizeof DIGITS);
    return chip;
}
void free_chip8(chip8* chip){
    free(chip);
}

void chip8_set_key_state(chip8* chip, chip8_key key, bool pressed){
    if(chip->state.cpu == CHIP8_WAITING_KEYPRESS && pressed){
        chip->V[chip->state.V] = key;
        chip->state.cpu = CHIP8_OK;
    }
    chip->keyboard[key] = pressed;
}

bool chip8_st_active(chip8* chip){
    return chip8_timer_get(&chip->ST) > 0;
}

static void se(chip8* chip, unsigned char x, unsigned char y){
    if(x == y)
        chip->PC+=2;
}
static void sne(chip8* chip, unsigned char x, unsigned char y){
    if(x != y)
        chip->PC++;
}

static void ins8(chip8* chip, unsigned char x, unsigned char y, unsigned char z){
    unsigned char* V = chip->V;
    switch(z){
    case 0x0: /*ld Vx, Vy*/
        if(DEBUG) fprintf(stderr, "%#06x: ld V%x, V%x\n",
                        chip->PC, x, y);
        V[x] = V[y];
        break;
    case 0x1: /*or Vx, Vy*/
        if(DEBUG) fprintf(stderr, "%#06x: or V%x V%x\n", chip->PC, x, y);
        V[x] |= V[y];
        break;
    case 0x2: /*and Vx, Vy*/
        if(DEBUG) fprintf(stderr, "%#06x: and V%x V%x\n", chip->PC, x, y);
        V[x] &= V[y];
        break;
    case 0x3: /*xor Vx, Vy*/
        if(DEBUG) fprintf(stderr, "%#06x: xor V%x V%x\n", chip->PC, x, y);
        V[x] ^= V[y];
        break;
    case 0x4: /*add Vx, Vy*/
        if(DEBUG) fprintf(stderr, "%#06x: add V%x, V%x\n", chip->PC, x, y);
        if(0xFF - V[x] < V[y]){
            V[x] += V[y];
            V[0xF] = 1;
        }else{
            V[x] += V[y];
            V[0xF] = 0;
        }
        break;
    case 0x5: /*sub Vx, Vy*/
        if(DEBUG) fprintf(stderr, "%#06x: sub V%x, V%x\n", chip->PC, x, y);
        if(V[x] > V[y]){
            V[x] -= V[y];
            V[0xF] = 0;
        }else{
            V[x] -= V[y];
            V[0xF] = 1;
        }
        break;
    case 0x6: /*shr Vx*/
        if(DEBUG) fprintf(stderr, "%#06x: shr V%x\n", chip->PC, x);
        if(V[x] & 1) V[0xF] = 1;
        V[x] >>= 1;
        break;
    case 0x7: /*subn Vx, Vy*/
        if(DEBUG) fprintf(stderr, "%#06x: subn V%x, V%x\n", chip->PC, x, y);
        if(V[y] > V[x]) V[0xF] = 1;
        else V[0xF] = 0;
        V[x] = V[y] - V[x];
        break;
    case 0xE: /*shl Vx*/
        if(DEBUG) fprintf(stderr, "%#06x: shl V%x\n", chip->PC, x);
        if(V[x] & 0x80) V[0xF] = 1;
        else V[0xF] = 0;
        V[x] <<= 1;
        break;
    default:
        chip->state.cpu = CHIP8_BADINS;
    }
    chip->PC += 2;
}

void insF(chip8* chip, unsigned char x, unsigned char kk){
    unsigned char* V = chip->V;
    switch(kk){
        case 0x07: /*ld Vx, DT*/
            if(DEBUG) fprintf(stderr, "%#06x: ld V%x, DT\n", chip->PC, x);

            chip->V[x] = chip8_timer_get(&chip->DT);
            break;
        case 0x0A: /*ld Vx, K*/
            if(DEBUG) fprintf(stderr, "%#06x: ld V%x, K\n", chip->PC, x);
            chip->state.cpu = CHIP8_WAITING_KEYPRESS;
            chip->state.V = x;
            break;
        case 0x15:
            if(DEBUG) fprintf(stderr, "%#06x: ld DT, V%x\n", chip->PC, x);
            chip8_timer_set(&chip->DT, chip->V[x]);
            break;
        case 0x18:
            if(DEBUG) fprintf(stderr, "%#06x: ld ST V%x\n", chip->PC, 0xF000);

            chip8_timer_set(&chip->ST, chip->V[x]);
            break;
        case 0x1E: /*add I, Vx*/
            if(DEBUG) fprintf(stderr, "%#06x: add I V%x\n", chip->PC, x);

            chip->I += V[x];
            break;
        case 0x29:
            if(DEBUG) fprintf(stderr, "%#06x: todo %#06x\n", chip->PC, 0xF000);
            chip->I = (V[x] & 0xF)*5;
            break;
        case 0x33:
            if(DEBUG) fprintf(stderr, "%#06x: todo %#06x\n", chip->PC, 0xF000);
            chip->memory[chip->I] = (V[x]/100)%10;
            chip->memory[chip->I+1] = (V[x]/10)%10;
            chip->memory[chip->I+2] = V[x]%10;
            break;
        case 0x55:
            if(DEBUG) fprintf(stderr, "%#06x: todo %#06x\n", chip->PC, 0xF000);
            for(int i=0;i<=x;i++) chip->memory[chip->I + i] = V[i];
            break;
        case 0x65:
            if(DEBUG) fprintf(stderr, "%#06x: todo %#06x\n", chip->PC, 0xF000);
            for(int i=0;i<=x;i++) V[i] = chip->memory[chip->I + i];
            break;
        default:
            chip->state.cpu = CHIP8_BADINS;
    }
}

int chip8_step(chip8* chip){
    if(chip->state.cpu == CHIP8_BADINS){
        return -1;
    }else if(chip->state.cpu == CHIP8_WAITING_KEYPRESS){
        return 0;
    }


    unsigned short ins = chip->memory[chip->PC] << 8
                        |chip->memory[chip->PC+1];
    unsigned char type = ins >> 12;
    unsigned short nnn = (ins & 0x0FFF);
    unsigned char x = (ins & 0x0F00)>>8;
    unsigned char y = (ins & 0x00F0)>>4;
    unsigned char z = (ins & 0x000F);
    unsigned char kk = (ins & 0x00FF);
    unsigned char* V = chip->V;


    switch (ins & 0xF000){
    case 0x0000:
        switch(ins & 0x0FFF){
        case 0x0E0: /*cls*/
            if(DEBUG) fprintf(stderr, "%#06x: cls\n", chip->PC);

            memset(chip->display, 0, 32*64);
            chip->PC += 2;
            break;
        case 0x0EE: /*ret*/
            if(DEBUG) fprintf(stderr, "%#06x: ret\n", chip->PC);

            chip->SP--;
            chip->PC = chip->stack[chip->SP];
            break;
        default:
            chip->state.cpu = CHIP8_BADINS;
            return -1;
        }
        break;
    case 0x1000: /*jp nnn*/
        if(DEBUG) fprintf(stderr, "%#06x: jp %#05x\n", chip->PC, ins & 0xFFF);

        chip->PC = ins & 0xFFF;
        break;
    case 0x2000: /*call nnn*/
        if(DEBUG) fprintf(stderr, "%#06x: call %#05x\n", chip->PC, ins & 0xFFF);

        chip->stack[chip->SP] = chip->PC+2;
        chip->SP++;
        chip->PC = ins & 0xFFF;
        break;
    case 0x3000: /* se Vx, kk */
        if(DEBUG) fprintf(stderr, "%#06x: se V%x %d\n",
                          chip->PC, (ins & 0x0F00)>>8, ins & 0x00FF);

        if(V[(ins & 0x0F00)>>8] == (ins & 0x00FF)){
            chip->PC += 2;
        }
        chip->PC += 2;
        break;
    case 0x4000: /* sne Vx, kk */
        if(DEBUG) fprintf(stderr, "%#06x: sne V%x %d\n",
                          chip->PC, (ins & 0x0F00)>>8, ins & 0x00FF);

        if(V[(ins & 0x0F00)>>8] != (ins & 0x00FF)){
            chip->PC += 2;
        }
        chip->PC += 2;
        break;
    case 0x5000:
        if(ins & 0x000F){
            chip->state.cpu = CHIP8_BADINS;
        }else{ // SE Vx, Vy
            se(chip, V[x], V[y]);
        }
        chip->PC += 2;
        break;
    case 0x6000: /* ld Vx, kk */
        if(DEBUG) fprintf(stderr, "%#06x: ld V%x, %#04x\n",
                            chip->PC, (ins & 0x0F00)>>8, ins & 0x00FF);

        chip->V[(ins & 0x0F00)>>8] = ins & 0x00FF;
        chip->PC += 2;
        break;
    case 0x7000: /* add Vx, kk */
        if(DEBUG) fprintf(stderr, "%#06x: add V%x, %#04x\n",
                            chip->PC, (ins & 0x0F00)>>8, ins & 0x00FF);

        chip->V[(ins & 0x0F00)>>8] += ins & 0x00FF;
        chip->PC += 2;
        break;
    case 0x8000:
        ins8(chip, x, y, z);
        break;
    case 0x9000: /*sne Vx Vy*/
        if(ins & 0x000F){
            chip->state.cpu = CHIP8_BADINS;
            return -1;
        }
        if(DEBUG) fprintf(stderr, "%#06x: sne V%x V%x\n",
                          chip->PC, (ins & 0x0F00)>>8, (ins & 0x00F0)>>4);

        if(V[(ins & 0x0F00)>>8] != V[(ins & 0x00F0)>>4]){
            chip->PC += 2;
        }
        chip->PC += 2;
        break;
    case 0xA000: /*ld I, nnn*/
        if(DEBUG) fprintf(stderr, "%#06x: ld I, %#05x\n", chip->PC, ins & 0xFFF);

        chip->I = ins & 0x0FFF;
        chip->PC += 2;
        break;
    case 0xB000: /* jp nnn + V0 */
        if(DEBUG) fprintf(stderr, "%#06x: jp %#05x\n",
                          chip->PC, chip->V[0] + (ins & 0xFFF));

        chip->PC = chip->V[0] + (ins & 0xFFF);
        break;
    case 0xC000: /* rnd Vx, kk */
        if(DEBUG) fprintf(stderr, "%#06x: rnd V%x, %#04x\n",
                                    chip->PC, x, kk);
        V[x] = rand() & kk;
        chip->PC += 2;
        break;
    case 0xD000: /*drw Vx, Vy, n*/
        if(DEBUG) fprintf(stderr, "%#06x: drw V%x, V%x, %d\n",
                            chip->PC, (ins&0x0F00)>>8,
                            (ins&0x00F0)>>4, (ins&0x000F));
        {
        int vx = chip->V[(ins&0x0F00)>>8];
        int vy = chip->V[(ins&0x00F0)>>4];
        chip->V[0xF] = 0;
        for(int i=0;i<(ins&0x000F);i++){
            for(int j=0;j<8;j++){
                if(chip->memory[chip->I + i] & (1<<(7-j))){
                    int x = (vx + j)%64;
                    int y = (vy + i)%32;
                    chip->display[y][x] ^= 1;
                    if(!chip->display[y][x]) chip->V[0xF] = 1;
                }
            }
        }
        }
        chip->PC += 2;
        break;
    case 0xE000:
        {
        int x = (ins&0x0F00)>>8;
        switch(ins & 0x00FF){
        case 0x009E: /*skp Vx*/
            if(DEBUG) fprintf(stderr, "%#06x: skp V%x\n", chip->PC, x);

            if(0 <= chip->V[x] && chip->V[x] < 16){
                if(chip->keyboard[chip->V[x]]) chip->PC += 2;
            }
            break;
        case 0x00A1: /*skpn Vx*/
            if(DEBUG) fprintf(stderr, "%#06x: skp V%x\n", chip->PC, x);

            if(0 <= chip->V[x] && chip->V[x] < 16){
                if(!chip->keyboard[chip->V[x]]) chip->PC += 2;
            }
            break;
        default:
            chip->state.cpu = CHIP8_BADINS;
            return -1;
        }
        chip->PC += 2;
        }
        break;
    case 0xF000:
        insF(chip, x, kk);
        chip->PC += 2;
        break;
    default:
        assert(0);
    }
    return 0;
}

void chip8_load_program_from_file(chip8* chip, const char* filename){
    FILE* fp = fopen(filename, "rb");
    fread(chip->memory + 0x200, 0x1000 - 0x200, 1, fp);
    chip->PC = 0x200;
    fclose(fp);
}

void chip8_load_program(chip8* chip, char* program, size_t len){
    memcpy(chip->memory + 0x200, program, len);
    chip->PC = 0x200;
}
