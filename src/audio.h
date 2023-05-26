#pragma once
#include <SDL2/SDL.h>

void panic_and_abort(const char *title, const char *text);

void SDLCALL feed_audio_device_callback(void *userdata, Uint8 *output_stream, int len);

void stop_audio(Uint8 **audiobuf, Uint32 *audiolen);

Uint32 open_new_audio_file(const char *fname, Uint8 **audiobuf, Uint32 *audiolen);

void init_audio(Uint8 **audiobuf, Uint32 *audiolen);

void deinit_audio(Uint8 **audiobuf);