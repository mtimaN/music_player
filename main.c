#include <stdio.h>
#include <SDL2/SDL.h>

#define FILE_PATH "music/A Cruel Angel's Thesis (Director's Edit Version).wav"

typedef struct
{
	uint8_t *pos;
	uint32_t length;
} audiodata_t;

void MyAudioCallback(void* userdata, Uint8 *stream, int stream_len)
{
	audiodata_t *audio = (audiodata_t *)userdata;

	if(audio->length == 0)
	{
		return;
	}

	int length = stream_len;
	length = (length > audio->length ? audio->length : length);

	SDL_memcpy(stream, audio->pos, length);

	audio->pos += length;
	audio->length -= length;
}

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_AUDIO);

	SDL_AudioSpec wav_spec;
	Uint8 *wav_buffer;
	Uint32 wav_len;
	if(SDL_LoadWAV(FILE_PATH, &wav_spec, &wav_buffer, &wav_len) == NULL)
	{
        fprintf(stderr, "Error %s could not be loaded as an audio file\n", FILE_PATH);
		return 1;
	}

	/*audiodata_t audio;
	audio.pos = wav_start;
	audio.length = wav_length;

	wav_spec.callback = MyAudioCallback;
	wav_spec.userdata = &audio;*/

	SDL_AudioDeviceID device = SDL_OpenAudioDevice(NULL, 0, &wav_spec, NULL, 0);
	if(device == 0)
	{
        fprintf(stderr, "Error: %s", SDL_GetError());
		return 1;
	}
	SDL_QueueAudio(device, wav_buffer, wav_len);
	SDL_PauseAudioDevice(device, 0);

	// it only plays for 5 seconds and then it stops
	SDL_Delay(5000);
	SDL_CloseAudioDevice(device);
	SDL_FreeWAV(wav_buffer);
	SDL_Quit();
	return 0;
}