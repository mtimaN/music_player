#include <stdio.h>
#include <SDL2/SDL.h>

#define FILE_PATH "../songs/【東方】Bad Apple!! ＰＶ【影絵】.wav"

int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		printf("Error: %s", SDL_GetError());
		SDL_Quit();
		return 1;
	}

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

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
	}
								//    R   G  B   A
	SDL_SetRenderDrawColor(renderer, 128, 128, 0, 128);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	SDL_AudioSpec wav_spec;
	Uint8 *wav_buffer;
	Uint32 wav_len;
	if (SDL_LoadWAV(FILE_PATH, &wav_spec, &wav_buffer, &wav_len) == NULL)
	{
        fprintf(stderr, "Error: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_AudioDeviceID device = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, 0);
	if (device == 0)
	{
        fprintf(stderr, "Error: %s", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_QueueAudio(device, wav_buffer, wav_len);
	SDL_PauseAudioDevice(device, 0);

	// it only plays for 50 seconds and then it stops
	SDL_Delay(50000);

	SDL_CloseAudioDevice(device);
	SDL_FreeWAV(wav_buffer);
	SDL_Quit();
	return 0;
}
