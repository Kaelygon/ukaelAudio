//./include/kaelygon/audio/kaelTone.h
//tone generating functions
#ifndef KAELTONE_H
	#define KAELTONE_H

#include <stdint.h>
#include <limits.h>

#include "audioTypes.h"
#include "tables.h"


uint8_t kaelAudio_waveVolume(uint8_t sample, const uint16_t volume){
	sample = ( (volume+1) * (uint16_t)sample )>>6; //volume
	sample += (( UINT8_MAX - (volume<<kaelAudio_const.invVolumeBits) )>>1)-1; // Amplitude
	return sample;
}

void kaelAudio_wavePitch(KaelAudio* kaud, const uint8_t pitch, uint8_t* accumulator){
	*accumulator += kaelAudio_pitchTab[pitch][0];
	uint8_t units = *accumulator/kaelAudio_pitchTab[pitch][1];
	if( units > 0 ){
		*accumulator -= units*kaelAudio_pitchTab[pitch][1];
		*kaud->wave.curPhase += units;
	}
}

void kaelAudio_toneGen(KaelAudio* kaud, uint8_t channel){
	uint8_t type = kaud->wave.info.type; //0-15 : wave function index
	uint8_t volume = kaud->wave.info.volume; //0-63 : volume multiplier (volume+1)/64 
	uint8_t pitch = kaud->wave.info.pitch; //0-63
	kaud->wave.curPhase = &kaud->wave.phase[channel];

	uint8_t pitchAcc = 0;
	uint16_t *bi=&kaud->wave.bufferInc;
	for(*bi=0; *bi < kaud->wave.bufferSize ;*bi+=1){
		kaud->wave.buffer[*bi] = kaud->wave.func[type](kaud);
		kaud->wave.buffer[*bi] = kaelAudio_waveVolume(kaud->wave.buffer[*bi], volume);
		kaelAudio_wavePitch(kaud, pitch, &pitchAcc);
	};
}

#endif