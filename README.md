# SDL Music Player

### Description
The program plays local audio files, being compatible with .wav, .flac and .mp3. From the graphical interface the songs can be switched, paused and volume can be changed.
    
The backend is made using SDL and SDL_mixer(SDL alone provides only .wav support). In short, the file is loaded in a buffer and the audio data is feeded by a callback function every time sound has to be played. Volume and panning options are implemented by multiplying the value of the samples with scalars corresponding to a ratio(ex: 50% volume -> every sample is multiplied by 1/2). Even though the panning options are set, they aren't implemented in the GUI.

The frontend is realized using functions from GTK. It displays the audio files stored locally and allows the user to select, pause and adjust the volume.
