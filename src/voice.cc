/*
 *	ss/src/voice.cc
 *	(c) 1998 geo@ff.cuni.cz
 *
 */
 
#include "common.h"


//#ifdef HAVE_FCNTL_H
#include <fcntl.h>
//#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_SYS_AUDIO_H
#include <sys/audio.h>
#endif

#ifdef HAVE_SYS_SOUNDCARD_H
#include <sys/soundcard.h>
#endif

#ifdef HAVE_IO_H
#include <io.h>		/* open, write, (ioctl,) ... */
#endif

#ifndef O_BINARY	/* open */
#define O_BINARY  0
#endif


#pragma hdrstop



int n_langs = 0;
int allocated_langs = 0;
lang **langs = NULL;
voice *this_voice = NULL;
lang *this_lang = NULL;

#define CONFIG_LANG_DESCRIBE
option langoptlist[] = {
	#include "options.cc"
	{NULL}
};

#define CONFIG_INV_DESCRIBE
option voiceoptlist[] = {
	#include "options.cc"
	{NULL}
};


#define CONFIG_LANG_INITIALIZE
lang::lang(const char *filename, const char *dirname)
{
	#include "options.cc"
	lang_name = "(unnamed)";
	ruleset = NULL;
	n_voices = 0;
	voices = NULL;
	default_voice = NULL;
	load_config(filename, dirname, "language", this, "Unknown language", langoptlist);
	if (!this_lang) this_lang = this;
	add_voices(voice_names);
}

lang::~lang()
{
}

void
lang::add_voice(const char *voice_name)
{
	char *filename = (char *)malloc(strlen(voice_name) + 6);
	char *dirname = (char *)malloc(strlen(voice_name) + 6);
	sprintf(filename, "%s.ini", voice_name);
	sprintf(dirname, "%s%c%s", cfg.lang_dir, SLASH, voice_name);
	if (*voice_name) {
		if (!voices) voices = (voice **)malloc(sizeof (void *));
		else if (!(n_voices-1 & n_voices))
			voices = (voice **)realloc(voices, n_voices << 1);
		voices[n_voices++] = new voice(filename, dirname, this_lang);
	}
	free(filename);
	free(dirname);
	if (!default_voice && voices) default_voice = voices[0];
}

void
lang::add_voices(const char *voice_names)
{
	int i, j;
	char *tmp = (char *)malloc(strlen(voice_names)+1);

	for (i=0, j=0; voice_names[i]; i++, j++) {
		if ((tmp[j] = voice_names[i]) == ':' ) {
			tmp[j] = 0;
			add_voice(tmp);
			j = 0;
		}
	}
	free(tmp);
}

void
lang::compile_rules()
{
	this_lang = this;
	DEBUG(3,10,fprintf(stddbg,"Compiling %s language rules, hash dir %s\n", name, hash_dir);)
	ruleset = new rules(rules_file, rules_dir);
}

#define CONFIG_INV_INITIALIZE
voice::voice(const char *filename, const char *dirname, lang *parent_lang)
{
	#include "options.cc"
	
	load_config(filename, dirname, "voice", this, "Unknown voice", voiceoptlist);

	FILE *soubor=fopen(dptfile, "rt", "diphone names");
	for (int i=0;i<441;i++) fscanf(soubor,"%3s\n", diphone_names[i]);
	fclose(soubor);
	
	char *output;
	if (!cfg.play_diph) cfg.wav_file = NULL_FILE;
	output = compose_pathname(cfg.wav_file, cfg.wav_dir);

#ifdef S_IRGRP
	fd = open(output, O_WRONLY | O_CREAT | O_TRUNC, MODE_MASK);
#else
	fd = open(output, O_RDWR | O_CREAT | O_TRUNC | O_BINARY);
#endif
	if (fd == -1) shriek("Failed to %s %s", strncmp(output, "/dev/", 5)
			? "create output file" : "open audio device", output);
	free(output);
	buffer_idx = 0;
	int stereo = channel==CT_MONO ? 0 : 1;
#ifdef SOUND_PCM_GETBLKSIZE
	stereo++;	/* 1 or 2 channels */
	ioctl (fd, SOUND_PCM_WRITE_CHANNELS, &stereo);  // keep it mono (disabled)
	ioctl (fd, SOUND_PCM_WRITE_RATE, &samp_rate);
	ioctl (fd, SOUND_PCM_WRITE_BITS, &samp_size);
	ioctl (fd, SOUND_PCM_GETBLKSIZE, &buff_size);
#else
   #ifdef SNDCTL_DSP_SPEED
	ioctl (fd, SNDCTL_DSP_STEREO, &stereo);		// keep it mono (disabled)
	ioctl (fd, SNDCTL_DSP_SPEED, &samp_rate);
	ioctl (fd, SNDCTL_DSP_SAMPLESIZE, &samp_size);
	ioctl (fd, SNDCTL_DSP_GETBLKSIZE, &buff_size);

	printf("st=%d Fs=%d samp_size=%d buf=%d ", stereo, samp_rate, samp_size, buff_size);

   #else
	DEBUG(3,9,fprintf(stddbg, "Sound ioctl's absent\n");)
   #endif
#endif
	buffer = (char *)malloc(buff_size);
}

voice::~voice()
{
	if (buffer_idx != 0) shriek("unimp voice::flush");
#ifdef SOUND_SYNCH
	if (ioctlable) ioctl (wavout, SOUND_SYNCH, 0);
#endif
	close(fd);
	free(buffer);
}


void
voice::flush()
{
	DEBUG(1,9,fprintf(stddbg, "Flushing the signal\n");)
	write(fd, buffer, samp_size * buffer_idx);
	buffer_idx = 0;
}
