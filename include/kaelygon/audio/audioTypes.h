//./include/kaelygon/audio/audioTypes.h
//type definitions and structures
#ifndef KAELDATATYPES_H
#define KAELDATATYPES_H

#include <stdint.h>

//forward declaration
typedef struct KaelAudio KaelAudio;

//waveform function pointer
typedef uint8_t (*WaveFunc)(KaelAudio*);

typedef struct {
    uint8_t mainVolume;
    uint8_t isStereo ;
    uint8_t channels;
} KaelAudio_config;

typedef struct {
    uint8_t noise[3];
    uint8_t size;
    uint8_t index;
    uint8_t rwalk;
} KaelAudio_random;

typedef struct {
    union {
        struct {
            uint16_t pitch : 6;
            uint16_t volume : 6;
            uint16_t type : 4;
        };
        uint16_t u16; // type<<12 | volume<<6 | pitch<<0
    } info;

    uint8_t* phase;
    uint8_t* curPhase;

    uint8_t* buffer;
    uint16_t bufferInc;
    uint16_t bufferSize;

    WaveFunc func[16];

} KaelAudio_waveData;

struct KaelAudio {
    KaelAudio_config config;
    KaelAudio_random random;
    KaelAudio_waveData wave;
};

//pre computed constants
typedef struct {
    uint8_t silentValue;

    uint8_t typeBits;
    uint8_t invTypeBits;

    uint8_t volumeBits;
    uint8_t invVolumeBits;

    uint8_t pitchBits;
    uint8_t invPitchBits;

    uint8_t lcg[4];
}KaelAudio_constant;

const KaelAudio_constant kaelAudio_const = {
    .silentValue = 128,

	.typeBits = 2,
	.invTypeBits = 6,

	.volumeBits = 6,
	.invVolumeBits = 2,

	.pitchBits = 6,
	.invPitchBits = 2,

	.lcg = {61,41,5,3}

};

#endif // KAELDATATYPES_H
