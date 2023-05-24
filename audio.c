#include <stdio.h>
#include <SDL2/SDL.h>

static SDL_AudioDeviceID audio_device = 0;
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

void panic_and_abort(const char *title, const char *text)
{
    fprintf(stderr, "PANIC: %s ... %s\n", title, text);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, text, window);
    SDL_Quit();
    exit(1);
}

static float volume_slider_value = 1.0f;
static float balance_slider_value = 0.5f;

static Uint8 *wavbuf = NULL;
static Uint32 wavlen = 0;
static SDL_AudioSpec wavspec;
static SDL_AudioStream *stream = NULL;

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

        SDL_assert((num_samples % 2) == 0);  // this should always be stereo data (at least for now).

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

void stop_audio(void)
{
    SDL_LockAudioDevice(audio_device);
    if (stream) {
        SDL_FreeAudioStream(stream);
        SDL_AtomicSetPtr((void **) &stream, NULL);
    }
    SDL_UnlockAudioDevice(audio_device);

    if (wavbuf) {
        SDL_FreeWAV(wavbuf);
    }

    wavbuf = NULL;
    wavlen = 0;
}

SDL_bool open_new_audio_file(const char *fname)
{
    SDL_AudioStream *tmpstream = stream;

    // make sure the audio callback can't touch `stream` while we're freeing it.
    SDL_LockAudioDevice(audio_device);
    SDL_AtomicSetPtr((void **) &stream, NULL);
    SDL_UnlockAudioDevice(audio_device);

    SDL_FreeAudioStream(tmpstream);
    SDL_FreeWAV(wavbuf);
    wavbuf = NULL;
    wavlen = 0;

    if (SDL_LoadWAV(fname, &wavspec, &wavbuf, &wavlen) == NULL) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't load wav file!", SDL_GetError(), window);
        goto failed;
    }

    tmpstream = SDL_NewAudioStream(wavspec.format, wavspec.channels, wavspec.freq, AUDIO_F32, 2, 48000);
    if (!tmpstream) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Couldn't create audio stream!", SDL_GetError(), window);
        goto failed;
    }

    if (SDL_AudioStreamPut(tmpstream, wavbuf, wavlen) == -1) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Audio stream put failed", SDL_GetError(), window);
        goto failed;
    }

    if (SDL_AudioStreamFlush(tmpstream) == -1) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Audio stream flush failed", SDL_GetError(), window);
        goto failed;
    }

    // make new `stream` available to the audio callback thread.
    SDL_LockAudioDevice(audio_device);
    SDL_AtomicSetPtr((void **) &stream, tmpstream);
    SDL_UnlockAudioDevice(audio_device);

    return SDL_TRUE;

failed:
    stop_audio();
    return SDL_FALSE;
}

void init_everything()
{
    SDL_AudioSpec desired;

    if (SDL_Init(SDL_INIT_AUDIO) == -1) {
        panic_and_abort("SDL_Init failed", SDL_GetError());
    }

    SDL_zero(desired);
    desired.freq = 48000;
    desired.format = AUDIO_F32;
    desired.channels = 2;
    desired.samples = 4096;
    desired.callback = feed_audio_device_callback;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &desired, NULL, 0);
    if (audio_device == 0) {
        panic_and_abort("Couldn't audio device!", SDL_GetError());
    }

    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);  // tell SDL we want this event that is disabled by default.

    open_new_audio_file("Test_Songs/LippsInc-FunkyTownlongVersion.wav");
}

void deinit_audio(Uint8 **wavbuf, SDL_AudioDeviceID audio_device)
{
    SDL_FreeWAV(*wavbuf);
    SDL_CloseAudioDevice(audio_device);
    SDL_Quit();
}