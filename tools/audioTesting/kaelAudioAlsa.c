#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define SAMPLE_RATE 32768
#define BUFFER_SIZE 64
#define CHANNEL_COUNT 2
#define DEBUG_PRINTING 1

static inline uint8_t kaelAudio_Sine(uint8_t phase){
	uint8_t secondHalf = phase & 0b10000000;
	phase <<= 1;
	uint16_t buf = ((uint16_t)(phase) << 1) - UINT8_MAX;
	phase = (uint8_t)((buf * buf) >> 9);
	phase = secondHalf ? phase : ~phase;
	return phase;
}

void alsa_globalVolume(long volume){
	snd_mixer_t *handle;
	snd_mixer_selem_id_t *sid;
	static const char *card = "default";
	static const char *selem_name = "Master";
	long min, max;

	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, card);
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name);

	snd_mixer_elem_t *elem = snd_mixer_find_selem(handle, sid);

	if (!elem)
	{
#if DEBUG_PRINTING
		fprintf(stderr, "Unable to find simple control '%s',%i\n", selem_name, 0);
#endif
		snd_mixer_close(handle);
		exit(-1);
	}

	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	long alsa_globalVolume = (volume * (max - min) / 100) + min;
	snd_mixer_selem_set_playback_volume_all(elem, alsa_globalVolume);
	snd_mixer_close(handle);
}

void alsa_quit(snd_pcm_t *handle){
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
}

void alsa_openPCM(snd_pcm_t **pcm, snd_pcm_hw_params_t **params, snd_pcm_uframes_t *buffer_size, unsigned int sample_rate){
#if DEBUG_PRINTING
	int err = snd_pcm_open(pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0)
	{
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(-1);
	}
#else
	snd_pcm_open(pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
#endif

	snd_pcm_hw_params_alloca(params);
	snd_pcm_hw_params_any(*pcm, *params);

	snd_pcm_hw_params_set_access(*pcm, *params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(*pcm, *params, SND_PCM_FORMAT_U8);
	snd_pcm_hw_params_set_channels(*pcm, *params, CHANNEL_COUNT); // Stereo
	snd_pcm_hw_params_set_rate_near(*pcm, *params, &sample_rate, 0);
	snd_pcm_hw_params_set_buffer_size_near(*pcm, *params, buffer_size);

#if DEBUG_PRINTING
	err = snd_pcm_hw_params(*pcm, *params);
	if (err < 0)
	{
		printf("Unable to set HW parameters: %s\n", snd_strerror(err));
		exit(-1);
	}
#else
	snd_pcm_hw_params(*pcm, *params);
#endif
}

void alsa_writePCM(snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t buffer_size){
#if DEBUG_PRINTING
	int err;
	err = snd_pcm_writei(pcm, buffer, buffer_size);
	if (err == -EPIPE)
	{
		// Buffer underrun occurred
		printf("Buffer underrun occurred\n");
		snd_pcm_prepare(pcm);
	}
	else if (err < 0)
	{
		printf("Error from writei: %s\n", snd_strerror(err));
		return;
	}
#else
	snd_pcm_writei(pcm, buffer, buffer_size);
#endif
}

int main(){
	uint16_t sample_rate = SAMPLE_RATE;
	uint8_t *buffer;
	
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	snd_pcm_uframes_t buffer_size = BUFFER_SIZE;

	// Set global volume to 16% (range is 0-100)
	alsa_globalVolume(16);

	alsa_openPCM(&handle, &params, &buffer_size, sample_rate);

	// Allocate buffer for PCM samples
	buffer = (uint8_t *)malloc(buffer_size * sizeof(uint8_t) * CHANNEL_COUNT);

	// Write buffer to PCM device
	uint8_t phase = 0;
	uint8_t amplitude = 255;
	for (uint i = 0; i < 128; i++)
	{
		for (snd_pcm_uframes_t i = 0; i < buffer_size * CHANNEL_COUNT; i += CHANNEL_COUNT)
		{
			uint16_t value = kaelAudio_Sine(phase);
			value = ((value * amplitude) / 255);
			value += ((UINT8_MAX - amplitude) >> 1);
			for (uint n = 0; n < CHANNEL_COUNT; n++)
			{
				buffer[i + n] = value;
			}
			phase++;
		}
		alsa_writePCM(handle, buffer, buffer_size);
	}
	free(buffer);

	alsa_quit(handle);

	return 0;
}
