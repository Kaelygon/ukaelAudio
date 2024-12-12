/* PipeWire */
/* SPDX-FileCopyrightText: Copyright © 2018 Wim Taymans */
/* SPDX-License-Identifier: MIT */

/*
 [title]
 Audio source using \ref pw_stream "pw_stream".
 [title]
 */

#include <stdio.h>

#include <spa/param/audio/format-utils.h>
#include <spa/param/props.h>

#include <pipewire/pipewire.h>

#define M_PI_M2 ( M_PI + M_PI )

#define DEFAULT_RATE            32768
#define DEFAULT_CHANNELS        2
#define DEFAULT_VOLUME          0.7

struct data {
        struct pw_main_loop *loop;
        struct pw_stream *stream;

        uint8_t accumulator;
};

/*
	Sine approximation of 127.5*sin(pi*x/2^7)+127.5
	Based on quadratic (3x-x^3)/2 [0,0.25]
	Detailed functions: https://desmos.com/calculator/sqllbjao14
	Max error = ~1.8%. Identical to sine at 0,64,128,192,255
*/

uint8_t kaelAudio_sine(uint8_t n){

		uint8_t q = n>>6; //quarter phase 0b00=1st 0b01=2nd 0b10=3rd 0b11=4th   
		n = n&0b00111111; //repeat quarters
		n = q&0b01 ? 64-n : n; //mirror 2nd and 4th quarters by x-axis

		uint16_t p = (((uint16_t)n*n)>>6)+1; //calculate 6x-n^3/2^11 cube in two parts to prevent overflow //+1 compensates flooring
		p = 6*n - (((uint16_t)n*p)>>5);

		uint8_t o = (p>>1)+128; //scale and offset to match sine wave
		o = q&0b10 ? ~o : o; //mirror 3rd and 4th quarters by y-axis
	
		return o;
}
/*
// Sine approximation function (adjust as needed)
static inline uint8_t kaelAudio_sine(uint8_t phase) {	
	uint8_t secondHalf = phase & 0b10000000;
	phase <<= 1;
	uint16_t buf = ((uint16_t)(phase) << 1) - UINT8_MAX;
	phase = (uint8_t)((buf * buf) >> 9);
	phase = secondHalf ? phase : ~phase;
	return phase;
}*/

/* our data processing function is in general:
 *
 *  struct pw_buffer *b;
 *  b = pw_stream_dequeue_buffer(stream);
 *
 *  .. generate stuff in the buffer ...
 *
 *  pw_stream_queue_buffer(stream, b);
 */
static void on_process(void *userdata)
{
    struct data *data = userdata;
    struct pw_buffer *b;
    struct spa_buffer *buf;
    int n_frames, stride;
    uint8_t *p;

    if ((b = pw_stream_dequeue_buffer(data->stream)) == NULL) {
        pw_log_warn("out of buffers: ");
        return;
    }

    buf = b->buffer;
    if ((p = buf->datas[0].data) == NULL)
        return;

    stride = sizeof(uint8_t) * DEFAULT_CHANNELS;

    n_frames = buf->datas[0].maxsize / stride; // Start with maximum size

    // Limit n_frames to what is requested or what can fit in the buffer
    n_frames = SPA_MIN(n_frames, buf->datas[0].maxsize / stride);

        uint8_t amplitude = 16;
    // Generate audio data
    for (int i = 0; i < n_frames; i++) {
        uint8_t val = kaelAudio_sine(data->accumulator);
        data->accumulator++;
        val =  (((uint16_t)(val * amplitude))/255) ;
        val += ((UINT8_MAX - amplitude)>>1);
        for (int c = 0; c < DEFAULT_CHANNELS; c++)
            *p++ = val;
    }

    // Update buffer metadata
    buf->datas[0].chunk->offset = 0;
    buf->datas[0].chunk->stride = stride;
    buf->datas[0].chunk->size = n_frames * stride;

    // Queue the buffer for playback
    pw_stream_queue_buffer(data->stream, b);
}


static const struct pw_stream_events stream_events = {
        PW_VERSION_STREAM_EVENTS,
        .process = on_process,
};

int main(int argc, char *argv[])
{
        struct data data = { 0, };
        const struct spa_pod *params[1];
        uint8_t buffer[1024];
        struct pw_properties *props;
        struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

        pw_init(&argc, &argv);

        /* make a main loop. If you already have another main loop, you can add
         * the fd of this pipewire mainloop to it. */
        data.loop = pw_main_loop_new(NULL);

        /* Create a simple stream, the simple stream manages the core and remote
         * objects for you if you don't need to deal with them.
         *
         * If you plan to autoconnect your stream, you need to provide at least
         * media, category and role properties.
         *
         * Pass your events and a user_data pointer as the last arguments. This
         * will inform you about the stream state. The most important event
         * you need to listen to is the process event where you need to produce
         * the data.
         */
        props = pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio",
                        PW_KEY_MEDIA_CATEGORY, "Playback",
                        PW_KEY_MEDIA_ROLE, "Music",
                        NULL);
        if (argc > 1)
                /* Set stream target if given on command line */
                pw_properties_set(props, PW_KEY_TARGET_OBJECT, argv[1]);
        data.stream = pw_stream_new_simple(
                        pw_main_loop_get_loop(data.loop),
                        "audio-src",
                        props,
                        &stream_events,
                        &data);

        /* Make one parameter with the supported formats. The SPA_PARAM_EnumFormat
         * id means that this is a format enumeration (of 1 value). */
        params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat,
                        &SPA_AUDIO_INFO_RAW_INIT(
                                .format = SPA_AUDIO_FORMAT_U8,
                                .channels = DEFAULT_CHANNELS,
                                .rate = DEFAULT_RATE ));

     //   pw_stream_set_control(data.stream, SPA_PROP_channelVolumes, DEFAULT_CHANNELS, 0, 0);

        /* Now connect this stream. We ask that our process function is
         * called in a realtime thread. */
        pw_stream_connect(data.stream,
                          PW_DIRECTION_OUTPUT,
                          PW_ID_ANY,
                          PW_STREAM_FLAG_AUTOCONNECT |
                          PW_STREAM_FLAG_MAP_BUFFERS |
                          PW_STREAM_FLAG_RT_PROCESS,
                          params, 1);
        
        float volume = 0.0;
        for(int i=0;i<1280000;i++){
        //    pw_stream_set_control(data.stream, i, 1, &volume, 0);
        }


/*
        pw_stream_set_control(data.stream,
                              pw_stream_get_node_id (data.stream), // Use your defined control ID here
                              1,
                              &volume);
*/

        /* and wait while we let things run */
        pw_main_loop_run(data.loop);

        pw_stream_destroy(data.stream);
        pw_main_loop_destroy(data.loop);
        pw_deinit();

        return 0;
}