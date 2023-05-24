#include <stdio.h>
#include <SDL2/SDL.h>

static SDL_AudioDeviceID audio_device = 0;
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static void panic_and_abort(const char *title, const char *text)
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

static void SDLCALL feed_audio_device_callback(void *userdata, Uint8 *output_stream, int len)
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

static void stop_audio(void)
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

static SDL_bool open_new_audio_file(const char *fname)
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

static void init_everything(int argc, char **argv)
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

static Uint8 *wavbuf = NULL;
static Uint32 wavlen = 0;
static SDL_AudioSpec wavspec;
static SDL_AudioStream *stream = NULL;

void deinit_audio(Uint8 **wavbuf, SDL_AudioDeviceID audio_device)
{
    SDL_FreeWAV(*wavbuf);
    SDL_CloseAudioDevice(audio_device);
    SDL_Quit();
}

static SDL_bool handle_events()
{
	SDL_bool paused = SDL_TRUE; 
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                return SDL_FALSE;  // don't keep going.

            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEBUTTONDOWN: {
                const SDL_bool pressed = (e.button.state == SDL_PRESSED) ? SDL_TRUE : SDL_FALSE;
                const SDL_Point pt = { e.button.x, e.button.y };
                int i;

                for (i = 0; i < SDL_arraysize(skin->buttons); i++) {
                    WinAmpSkinButton *btn = &skin->buttons[i];
                    btn->pressed = (pressed && SDL_PointInRect(&pt, &btn->dstrect)) ? SDL_TRUE : SDL_FALSE;
                    if (btn->pressed) {
                        switch ((WinAmpSkinButtonId) i) {
                            case WASBTN_PREV:
                                SDL_AudioStreamClear(stream);
                                if (SDL_AudioStreamPut(stream, wavbuf, wavlen) == -1) {
                                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Audio stream put failed", SDL_GetError(), window);
                                    stop_audio();
                                } else if (SDL_AudioStreamFlush(stream) == -1) {
                                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Audio stream flush failed", SDL_GetError(), window);
                                    stop_audio();
                                }
                                break;

                            case WASBTN_PAUSE:
                                paused = paused ? SDL_FALSE : SDL_TRUE;
                                SDL_PauseAudioDevice(audio_device, paused);
                                break;

                            case WASBTN_STOP:
                                stop_audio();
                                break;
                        }
                    }
                }
                break;
            }

            #if 0
            case SDL_MOUSEMOTION: {
                const SDL_Point pt = { e.motion.x, e.motion.y };
                if (SDL_PointInRect(&pt, &volume_rect) && (e.motion.state & SDL_BUTTON_LMASK)) {  // mouse is pressed inside the "volume" "slider"?
                    const float fx = (float) (pt.x - volume_rect.x);
                    volume_slider_value = (fx / ((float) volume_rect.w));  // a value between 0.0f and 1.0f
                    //printf("SLIDING! At %dx%d (%d percent)\n", pt.x, pt.y, (int) SDL_round(volume_slider_value * 100.0f));
                    volume_knob.x = pt.x - (volume_knob.w / 2);
                    volume_knob.x = SDL_max(volume_knob.x, volume_rect.x);
                    volume_knob.x = SDL_min(volume_knob.x, (volume_rect.x + volume_rect.w) - volume_knob.w);
                } else if (SDL_PointInRect(&pt, &balance_rect) && (e.motion.state & SDL_BUTTON_LMASK)) {  // mouse is pressed inside the "balance" "slider"?
                    const float fx = (float) (pt.x - balance_rect.x);
                    balance_slider_value = (fx / ((float) balance_rect.w));  // a value between 0.0f and 1.0f
                    //printf("SLIDING! At %dx%d (%d percent)\n", pt.x, pt.y, (int) SDL_round(balance_slider_value * 100.0f));
                    balance_knob.x = pt.x - (balance_knob.w / 2);
                    balance_knob.x = SDL_max(balance_knob.x, balance_rect.x);
                    balance_knob.x = SDL_min(balance_knob.x, (balance_rect.x + balance_rect.w) - balance_knob.w);
                }
                break;
            }
            #endif

            case SDL_DROPFILE: {
                open_new_audio_file(e.drop.file);
                SDL_free(e.drop.file);
                break;
            }
        }
    }

    return SDL_TRUE;  // keep going.
}

int main(int argc, char **argv)
{
    init_everything(argc, argv);  // will panic_and_abort on issues.

    while (handle_events(&skin)) {
        draw_frame(renderer, &skin);
    }

    deinit_everything();

    return 0;
}