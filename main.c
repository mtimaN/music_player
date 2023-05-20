#include <stdio.h>
#include <SDL2/SDL.h>

int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		printf("Error: %s", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_AudioSpec wav_spec;
	Uint8 *wav_buffer = NULL;
	Uint32 wav_len = 0;
	SDL_AudioDeviceID device;

	SDL_Window *window = SDL_CreateWindow("Lopata Music Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
	if (!window) {
		printf("Error: %s", SDL_GetError());
		return 1;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		printf("Error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

	SDL_bool paused = SDL_TRUE;
	const SDL_Rect rewind_rect = {100, 100, 100, 100};
	const SDL_Rect pause_rect = {400, 100, 100, 100};

	int green = 0;
	SDL_bool keep_going = SDL_TRUE;
	while (keep_going) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					keep_going = SDL_FALSE;
					break;
				case SDL_MOUSEBUTTONDOWN: {
					const SDL_Point point = { event.button.x, event.button.y};
					if (device) {
						if (SDL_PointInRect(&point, &rewind_rect)) {
							SDL_ClearQueuedAudio(device);
							SDL_QueueAudio(device, wav_buffer, wav_len);
						} else if (SDL_PointInRect(&point, &pause_rect)) {
							paused = !paused;
							SDL_PauseAudioDevice(device, paused);
						}
					}
					break;
				}
				case SDL_DROPFILE: {
					if (device) {
						SDL_CloseAudioDevice(device);
						device = 0;
					}

					SDL_FreeWAV(wav_buffer);
					wav_buffer = NULL;
					wav_len = 0;
					if (SDL_LoadWAV(event.drop.file, &wav_spec, &wav_buffer, &wav_len) == NULL)
						SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Uhoh!", SDL_GetError(), window);


					/*
					SDL_AudioSpec desired;
					SDL_zero(desired);
					desired.freq = 48000;
					desired.format = AUDIO_F32;
					desired.channels = 2;
					desired.samples = 4096;
					desired.callback = NULL;
					*/
					device = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, 0);
					if (device == 0) {
						SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Uhoh!", SDL_GetError(), window);
						SDL_FreeWAV(wav_buffer);
						wav_buffer = NULL;
						wav_len = 0;
					} else {
						SDL_QueueAudio(device, wav_buffer, wav_len);
						SDL_PauseAudioDevice(device, paused);
					}
					SDL_free(event.drop.file);
					break;
				}
			}
		}
									//    R    G   B  Alpha
		SDL_SetRenderDrawColor(renderer, 128, green / 100, 0, 128);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		SDL_RenderFillRect(renderer, &rewind_rect);
		SDL_RenderFillRect(renderer, &pause_rect);
		SDL_RenderPresent(renderer);
		green++;
		green %= 25600;
	}

	SDL_CloseAudioDevice(device);
	SDL_FreeWAV(wav_buffer);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
