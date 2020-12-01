#include "chip8_timer.h"


void chip8_timer_set(chip8_timer* t, unsigned char x){
    t->value = x;
    t->last = clock();
}

unsigned char chip8_timer_get(chip8_timer* t){
    if(t->value == 0) return 0;

    unsigned char ndt = (t->last - clock())*60/CLOCKS_PER_SEC;
    if(t->value < ndt){
        t->value = 0;
    }else{
        t->value -= ndt;
    }
    if(ndt) t->last = clock();

    return t->value;
}
