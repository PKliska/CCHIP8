#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "chip8.h"
#include "SDL.h"

int TPS = 500;
int FPS = 60;


void set_key(chip8* chip, SDL_Keycode sym, bool state){
    switch(sym){
    case SDLK_0:
        chip8_set_key_state(chip, CHIP8_KEY_0, state);
        break;
    case SDLK_1:
        chip8_set_key_state(chip, CHIP8_KEY_1, state);
        break;
    case SDLK_2:
        chip8_set_key_state(chip, CHIP8_KEY_2, state);
        break;
    case SDLK_3:
        chip8_set_key_state(chip, CHIP8_KEY_3, state);
        break;
    case SDLK_4:
        chip8_set_key_state(chip, CHIP8_KEY_4, state);
        break;
    case SDLK_5:
        chip8_set_key_state(chip, CHIP8_KEY_5, state);
        break;
    case SDLK_6:
        chip8_set_key_state(chip, CHIP8_KEY_6, state);
        break;
    case SDLK_7:
        chip8_set_key_state(chip, CHIP8_KEY_7, state);
        break;
    case SDLK_8:
        chip8_set_key_state(chip, CHIP8_KEY_8, state);
        break;
    case SDLK_9:
        chip8_set_key_state(chip, CHIP8_KEY_9, state);
        break;
    case SDLK_a:
        chip8_set_key_state(chip, CHIP8_KEY_A, state);
        break;
    case SDLK_b:
        chip8_set_key_state(chip, CHIP8_KEY_B, state);
        break;
    case SDLK_c:
        chip8_set_key_state(chip, CHIP8_KEY_C, state);
        break;
    case SDLK_d:
        chip8_set_key_state(chip, CHIP8_KEY_D, state);
        break;
    case SDLK_e:
        chip8_set_key_state(chip, CHIP8_KEY_E, state);
        break;
    case SDLK_f:
        chip8_set_key_state(chip, CHIP8_KEY_F, state);
        break;
    }
}

int AL = 44100;
float AUDIO[44100];
void audio_callback(void* userdata, Uint8* stream, int len){
    int *x = userdata;
    len /= sizeof(float);
    float* s = (float*)stream;
    for(int i=0;i<len;i++){
        s[i] = AUDIO[*x];
        (*x)++;
        if(*x==AL) *x=0;
    }
}




int main(int argc, char** argv){
    printf("CCHIP8 version %d.%d\n",
            CCHIP8_VERSION_MAJOR, CCHIP8_VERSION_MINOR);

    if(argc != 2){
        exit(EXIT_FAILURE);
    }

    chip8* chip = new_chip8();

    chip8_load_program_from_file(chip, argv[1]);


    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)){
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s", SDL_GetError());
        goto sdl_error;
    }

    SDL_Window* window = SDL_CreateWindow("CCHIP8 emulator",
                                            SDL_WINDOWPOS_UNDEFINED,
                                            SDL_WINDOWPOS_UNDEFINED,
                                            640,
                                            320,
                                            SDL_WINDOW_RESIZABLE);
    if(window == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "%s", SDL_GetError());
        goto window_error;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "%s", SDL_GetError());
        goto renderer_error;
    }
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if(texture == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "%s", SDL_GetError());
        goto texture_error;
    }
    for(int i=0;i<AL;i++){
        AUDIO[i] = sin(200*i*(2 * M_PI)/AL);
    }

    SDL_AudioSpec want;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = 44100;
    want.format = AUDIO_F32;
    want.channels = 1;
    want.samples = 4096;
    want.callback = audio_callback;
    int *x = malloc(sizeof(int));
    *x = 0;
    want.userdata = x;
    SDL_AudioDeviceID audio_dev = SDL_OpenAudioDevice(NULL, false, &want, NULL, 0);
    SDL_PauseAudioDevice(audio_dev, true);

    bool running = true;
    unsigned int last_tick = SDL_GetTicks();
    unsigned int last_frame = SDL_GetTicks();
    while(running){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                set_key(chip, event.key.keysym.sym, true);
                break;
            case SDL_KEYUP:
                set_key(chip, event.key.keysym.sym, false);
                break;
            default:
                break;
            }
        }
        if(SDL_GetTicks() < last_tick + 1000/TPS){
            SDL_Delay((last_tick + 1000/TPS) - SDL_GetTicks());
        }
        int res = chip8_step(chip);
        last_tick = SDL_GetTicks();
        if(res){
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                        "Invalid chip8 instruction PC=%#05x", chip->PC);
            running = false;
            SDL_Delay(5000);
        }

        if(SDL_TICKS_PASSED(SDL_GetTicks(), last_frame + 1000/FPS)){
            char* pixels;
            int pitch;
            SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch);
            for(int y=0;y<32;y++){
                for(int x=0;x<64;x++){
                    if(chip->display[y][x]){
                        pixels[y*pitch + 4*x] = 0xFF;
                        pixels[y*pitch + 4*x+1] = 0x20;
                        pixels[y*pitch + 4*x+2] = 0x10;
                    }else{
                        pixels[y*pitch + 4*x] = 0xAA;
                        pixels[y*pitch + 4*x+1] = 0x20;
                        pixels[y*pitch + 4*x+2] = 0x10;
                    }
                }
            }
            SDL_UnlockTexture(texture);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            last_frame = SDL_GetTicks();
        }
        if(chip8_st_active(chip)){
            SDL_PauseAudioDevice(audio_dev, false);
        }else{
            SDL_PauseAudioDevice(audio_dev, true);
        }
    }
    free(x);
    SDL_CloseAudioDevice(audio_dev);
    SDL_DestroyTexture(texture);
texture_error:
    SDL_DestroyRenderer(renderer);
renderer_error:
    SDL_DestroyWindow(window);
window_error:
    SDL_Quit();
sdl_error:
    free_chip8(chip);
}
