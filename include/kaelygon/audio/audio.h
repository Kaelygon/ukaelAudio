//./include/kaelygon/audio/audio.h
//kaelData functions
#ifndef KAELDATA_H
#define KAELDATA_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "audioTypes.h"
#include "waveform.h"
#include "../kaelMacros.h"

void kaelAudio_init(KaelAudio* kaud){
    memset(kaud, 0, sizeof(KaelAudio));

    kaud->config.mainVolume = 255;
	kaud->config.channels = 16;

    kaud->random.size = sizeof(kaud->random.noise)/sizeof(kaud->random.noise[0]);
	
	kaud->random.index=0;
    kaud->random.noise[0] = 143;
	for(uint8_t i=1; i<kaud->random.size;i++){
   		kaud->random.noise[i] = kaelAudio_rorlcg(kaud->random.noise[i-1]);
	}
    kaud->random.rwalk = 128;
	
	
	kaud->wave.func[0]=(WaveFunc)kaelAudio_sine; 
	kaud->wave.func[1]=(WaveFunc)kaelAudio_saw;
	kaud->wave.func[2]=(WaveFunc)kaelAudio_square;
	kaud->wave.func[3]=(WaveFunc)kaelAudio_triangle;
	kaud->wave.func[4]=(WaveFunc)kaelAudio_noise;
	kaud->wave.func[5]=(WaveFunc)kaelAudio_rwalk;
	for(uint8_t i=6;i<16;i++){
		kaud->wave.func[i]=(WaveFunc)kaelAudio_sine;
	}

	kaud->wave.info.type = 0;
	kaud->wave.info.volume = 0;
	kaud->wave.info.pitch = 0;
	kaud->wave.info.u16 = (0<<12) | (63<<6) | (1); //type=0 volume=63 pitch=1
	
	//MALLOC
	kaud->wave.bufferSize = 256;
	kaud->wave.phase = calloc( kaud->config.channels, sizeof(kaud->wave.phase[0]) );
	NULL_CHECK(kaud->wave.phase);
	kaud->wave.buffer = calloc( kaud->wave.bufferSize, sizeof(kaud->wave.buffer[0]) );
	NULL_CHECK(kaud->wave.buffer);
	
}

void kaelAudio_freeData(KaelAudio* kaud){
	free(kaud->wave.phase);
	free(kaud->wave.buffer);
}

#endif