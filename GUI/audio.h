#pragma once
#include <SDL2/SDL.h>

void panic_and_abort(const char *title, const char *text);

void SDLCALL feed_audio_device_callback(void *userdata, Uint8 *output_stream, int len);

void stop_audio(void);

SDL_bool open_new_audio_file(const char *fname);

void init_everything();

void deinit_audio();