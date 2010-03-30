/***
  This file is part of PulseAudio.

  Copyright 2004-2006 Lennart Poettering
  Copyright 2006 Pierre Ossman <ossman@cendio.se> for Cendio AB

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2 of the License,
  or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with PulseAudio; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <locale.h>
#include <pulse/pulseaudio.h>

typedef struct pa_asyn_user {
   pa_sample_spec *ss;
   char *data;
   int32_t length;
} params;

static pa_context *cntxt = NULL;
static pa_stream *pa_strm = NULL;
static pa_mainloop* m = NULL;
static pa_mainloop_api *mainloop_api = NULL;

static const char *stream_name = "TTS-Epos", *client_name = "Epos";

static pa_volume_t volume = PA_VOLUME_NORM;
static pa_sample_spec sample_spec = { (pa_sample_format_t)0, 0, 0};

/* Channel mapping and sample buffering options not used */
//static pa_channel_map channel_map;
//static pa_buffer_attr sample_buff = {(uint32_t)-1, 0, (uint32_t)-1, (uint32_t)-1, (uint32_t)-1};

/* A shortcut for terminating the application */
static void quit(int ret) {
    assert(mainloop_api);
    mainloop_api->quit(mainloop_api, ret);
}

/* Connection draining complete */
static void context_drain_complete(pa_context*c, void *userdata) {
    pa_context_disconnect(c);
}

/* Stream draining complete */
static void stream_drain_complete(pa_stream*s, int success, void *userdata) {
    pa_operation *o;

    if (!success) {
        shriek(812, fmt(("Failed to drain stream: %s\n"), pa_strerror(pa_context_errno(cntxt))) );
    }
    DEBUG(3,10,fprintf(STDDBG, ("Playback stream drained.\n")););

    pa_stream_disconnect(pa_strm);
    pa_stream_unref(pa_strm);
    pa_strm = NULL;

    if (!(o = pa_context_drain(cntxt, context_drain_complete, NULL)))
        pa_context_disconnect(cntxt);
    else {
        pa_operation_unref(o);
        DEBUG(3,10,fprintf(STDDBG, ("Draining connection to server.\n")););
    }
}

/* This is called whenever new data may be written to the stream */
static void stream_write_callback(pa_stream *s, size_t length, void *userdata) {
    pa_asyn_user *prms = (pa_asyn_user*)userdata;
 
    assert(s && length);
    if (!prms->length)
        return;
        
    //data = pa_xmemdup(prms->data, length);
    pa_stream_write(s, prms->data, (prms->length < length) ? prms->length : length, NULL, 0, PA_SEEK_RELATIVE);
    (*prms).data += length;
    (*prms).length -= length;
    
    DEBUG(2,1, fprintf(STDDBG, "Written %d samples to server.\n", length););
    if (prms->length < 0) {
        prms->length = 0;
        pa_operation_unref(pa_stream_drain(s, stream_drain_complete, NULL));
    }
}

/* This routine is called whenever the stream state changes */
static void stream_state_callback(pa_stream *s, void *userdata) {
    assert(s);

    switch (pa_stream_get_state(s)) {
        case PA_STREAM_CREATING:
        case PA_STREAM_TERMINATED:
            break;

        case PA_STREAM_READY:
            DEBUG(3,10,fprintf(STDDBG, ("Stream successfully created\n")););
            break;

        case PA_STREAM_FAILED:
        default:
            shriek(812, fmt( ("Stream errror: %s\n"), pa_strerror(pa_context_errno(pa_stream_get_context(s)))) );
    }
}

/* This is called whenever the context status changes */
static void context_state_callback(pa_context *c, void *userdata) {
    assert(c);

    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY: {
            pa_cvolume cv;

            assert(c && !pa_strm);
            DEBUG(3,10,fprintf(STDDBG, ("Connection established.\n")););
            
            sample_spec.format = ((pa_asyn_user*)userdata)->ss->format;
            sample_spec.rate = ((pa_asyn_user*)userdata)->ss->rate;
            sample_spec.channels = ((pa_asyn_user*)userdata)->ss->channels;
            //pa_channel_map_init_auto(&channel_map, sample_spec.channels, PA_CHANNEL_MAP_OSS);
            //sample_buff.tlength = (uint32_t)(((pa_asyn_user*)userdata)->length);
            //pa_strm = pa_stream_new(c, stream_name, &sample_spec, &channel_map);

            pa_strm = pa_stream_new(c, stream_name, &sample_spec, NULL);
            assert(pa_strm);

            pa_stream_set_state_callback(pa_strm, stream_state_callback, NULL);
            pa_stream_set_write_callback(pa_strm, stream_write_callback, userdata);
            pa_stream_connect_playback(pa_strm, NULL, 0, (pa_stream_flags_t)0, pa_cvolume_set(&cv, sample_spec.channels, volume), NULL);
            //pa_stream_connect_playback(pa_strm, NULL, &sample_buff, (pa_stream_flags_t)0, pa_cvolume_set(&cv, sample_spec.channels, volume), NULL);
            break;
        }

        case PA_CONTEXT_TERMINATED:
            quit(0);
            break;

        case PA_CONTEXT_FAILED:
        default:
            shriek(812, fmt(("Connection failure: %s\n"), pa_strerror(pa_context_errno(c))) );
    }
}

/* UNIX signal to quit recieved */
static void exit_signal_callback(pa_mainloop_api*m, pa_signal_event *e, int sig, void *userdata) {
    DEBUG(3,10,fprintf(STDDBG, ("Got SIGINT, exiting.\n")););
    quit(0);
}

enum {
    ARG_VERSION = 256,
    ARG_STREAM_NAME,
    ARG_VOLUME,
    ARG_CHANNELMAP
};


