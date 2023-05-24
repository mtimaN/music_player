#pragma once
#include <SDL2/SDL.h>

void panic_and_abort(const char *title, const char *text);

void SDLCALL feed_audio_device_callback(void *userdata, Uint8 *output_stream, int len);

void stop_audio(Uint8 **wavbuf, Uint32 *wavlen);

SDL_bool open_new_audio_file(const char *fname, Uint8 **wavbuf, Uint32 *wavlen, SDL_AudioSpec *wavspec);

void init_everything(Uint8 **wavbuf, Uint32 *wavlen);

void deinit_audio(Uint8 **wavbuf);