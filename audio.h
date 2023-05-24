#pragma once
#include <SDL2/SDL.h>

SDL_AudioDeviceID audio_device = 0;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

void panic_and_abort(const char *title, const char *text);

float volume_slider_value = 1.0f;
float balance_slider_value = 0.5f;

Uint8 *wavbuf = NULL;
Uint32 wavlen = 0;
SDL_AudioSpec wavspec;
SDL_AudioStream *stream = NULL;

void SDLCALL feed_audio_device_callback(void *userdata, Uint8 *output_stream, int len);

void stop_audio(void);

SDL_bool open_new_audio_file(const char *fname);

void init_everything();

void deinit_audio();