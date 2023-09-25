#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "audio.h"

SDL_AudioDeviceID audio_device = 0;

// these variables are used to change the volume and the panning of the music
float volume_slider_value = 0.5f;
float balance_slider_value = 0.5f;

SDL_AudioStream *stream = NULL;
SDL_AudioSpec desired;

void panic_and_abort(const char *title, const char *text)
{
    fprintf(stderr, "PANIC: %s ... %s\n", title, text);
    SDL_Quit();
    exit(1);
}

void SDLCALL feed_audio_device_callback(void *userdata, Uint8 *output_stream, int len)
{
    SDL_AudioStream *input_stream = (SDL_AudioStream *) SDL_AtomicGetPtr((void **) &stream);

    if (input_stream == NULL) {  // nothing playing, just write silence and bail.
        SDL_memset(output_stream, '\0', len);
        return;
    }

    const int num_converted_bytes = SDL_AudioStreamGet(input_stream, output_stream, len);
    if (num_converted_bytes > 0) {
        const int num_samples = (num_converted_bytes / sizeof (float));
        float *samples = (float *) output_stream;

        SDL_assert((num_samples % 2) == 0);  // this should always be stereo data

        // change the volume of the audio we're playing.
        if (volume_slider_value != 1.0f) {
            for (int i = 0; i < num_samples; i++) {
                samples[i] *= volume_slider_value;
            }
        }

        // first sample is left, second is right.
        // change the balance of the audio we're playing.
        if (balance_slider_value > 0.5f) {
            for (int i = 0; i < num_samples; i += 2) {
                samples[i] *= 1.0f - balance_slider_value;
            }
        } else if (balance_slider_value < 0.5f) {
            for (int i = 0; i < num_samples; i += 2) {
                samples[i+1] *= balance_slider_value;
            }
        }
    }

    len -= num_converted_bytes;  // now has number of bytes left after feeding the device.
    output_stream += num_converted_bytes;
    if (len > 0) {
        SDL_memset(output_stream, '\0', len);
    }
}

void stop_audio(Uint8 **audiobuf, Uint32 *audiolen)
{
    SDL_LockAudioDevice(audio_device);
    if (stream) {
        SDL_FreeAudioStream(stream);
        SDL_AtomicSetPtr((void **) &stream, NULL);
    }
    SDL_UnlockAudioDevice(audio_device);

    if (*audiobuf) {
        SDL_FreeWAV(*audiobuf);
    }

    *audiobuf = NULL;
    *audiolen = 0;
}

Uint32 open_new_audio_file(const char *fname, Uint8 **audiobuf, Uint32 *audiolen)
{
    SDL_AudioStream *tmpstream = stream;

    // make sure the audio callback can't touch `stream` while we're freeing it.
    SDL_LockAudioDevice(audio_device);
    SDL_AtomicSetPtr((void **) &stream, NULL);
    SDL_UnlockAudioDevice(audio_device);

    SDL_FreeAudioStream(tmpstream);
    *audiobuf = NULL;
    *audiolen = 0;
    Mix_Music *music = NULL;
    const char *ext = strrchr(fname, '.');
    if (ext == NULL)
    {
        printf("Invalid audio file: %s\n", fname);
        return 0;
    } else if (strcmp(ext, ".flac") == 0) {
        Mix_CloseAudio();
        SDL_FreeWAV(*audiobuf);
        if (Mix_OpenAudio(96000, AUDIO_F32, 2, 4096) != 0) {
            printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
            goto failed;
        }
        music = Mix_LoadMUS(fname);
        if (music != NULL)
        {
            Mix_PlayMusic(music, 1);
            Mix_PauseMusic();
        } else {
            printf("Mix_LoadMUS failed: %s\n", Mix_GetError());
            goto failed;
        }
    } else if (strcmp(ext, ".mp3") == 0) {
        Mix_CloseAudio();
        SDL_FreeWAV(*audiobuf);
        if (Mix_OpenAudio(48000, AUDIO_F32, 2, 4096) != 0) {
            printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
            goto failed;
        }
        music = Mix_LoadMUS(fname);
        if (music != NULL)
        {
            Mix_PlayMusic(music, 1);
            Mix_PauseMusic();
        } else {
            printf("Mix_LoadMUS failed: %s\n", Mix_GetError());
            goto failed;
        }
    } else {
        SDL_FreeWAV(*audiobuf);
        Mix_CloseAudio();
        if (SDL_LoadWAV(fname, &desired, audiobuf, audiolen) == NULL) {
            goto failed;
        }

        tmpstream = SDL_NewAudioStream(desired.format, desired.channels, desired.freq, AUDIO_F32, 2, 48000);
        if (!tmpstream) {
            printf("Couldn't open WAV file\n");
            goto failed;
        }

        if (SDL_AudioStreamPut(tmpstream, *audiobuf, *audiolen) == -1) {
            printf("Couldn't open WAV file\n");
            goto failed;
        }

        if (SDL_AudioStreamFlush(tmpstream) == -1) {
            printf("Couldn't open WAV file\n");
            goto failed;
        }
    }

    // make new `stream` available to the audio callback thread.
    SDL_LockAudioDevice(audio_device);
    SDL_AtomicSetPtr((void **) &stream, tmpstream);
    SDL_UnlockAudioDevice(audio_device);

    // adding some placeholder for length
    if (*audiolen == 0)
        *audiolen = 99999999;
    return (Uint32)(*audiolen/(desired.freq * desired.channels * (SDL_AUDIO_BITSIZE(desired.format)) / 8));

failed:
    Mix_CloseAudio();
    Mix_Quit();
    stop_audio(audiobuf, audiolen);
    return 0;
}

void init_audio(Uint8 **audiobuf, Uint32 *audiolen)
{
    
    if (SDL_Init(SDL_INIT_AUDIO) == -1) {
        panic_and_abort("SDL_Init failed", SDL_GetError());
    }
    Mix_Init(MIX_INIT_FLAC | MIX_INIT_MP3);
    SDL_zero(desired);
    desired.freq = 48000;
    desired.format = AUDIO_F32;
    desired.channels = 2;
    desired.samples = 4096;
    desired.callback = feed_audio_device_callback;
    audio_device = SDL_OpenAudioDevice(NULL, 0, &desired, NULL, 0);
    if (audio_device == 0) {
        panic_and_abort("Couldn't open audio device!", SDL_GetError());
    }

    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);  // tell SDL we want this event that is disabled by default.
}

void deinit_audio(Uint8 **audiobuf)
{
    SDL_FreeWAV(*audiobuf);
    Mix_CloseAudio();
    Mix_Quit();
    SDL_CloseAudioDevice(audio_device);
    SDL_Quit();
}
