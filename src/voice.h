/*
 *	ss/src/voice.h
 *	(c) geo@ff.cuni.cz
 *
 *	This class represents a single front end diphone inventory.
 */

class rules;

enum SYNTH_TYPE {
	S_NONE = 0,
	S_TCP = 1,
	S_LPC_FLOAT = 4,
	S_LPC_INT = 5,
	S_LPC_VQ = 6,
	S_KTD = 8,
};

#define STstr "none:tcpip:::lpc-float:lpc-int:lpc-vq::ktd:"
#define ST_MAX (S_KTD+1)

enum CHANNEL_TYPE {CT_MONO, CT_FIRST, CT_SECOND, CT_BOTH};
#define CHANNEL_TYPEstr "mono:first:second:both:"


struct voice;

struct lang
{
   #define CONFIG_LANG_DECLARE
   #include "options.cc"

	char *lang_name;
	rules *ruleset;
	int   n_voices;
	voice **voices;
	voice *default_voice;

   public:
	lang::lang(const char *filename, const char *dirname);
	lang::~lang();
	void add_voice(const char *voice_name);
	void add_voices(const char *voice_name);
	void compile_rules();		// could be made delayed....
};


struct voice
{
   #define CONFIG_INV_DECLARE
   #include "options.cc"

	int buffer_idx;
	char *buffer;
	int fd;		/* This is a file descriptor to write the samples to.
			 * Can also be an open device or a socket.
			 */

/*	int init_f;
 *	int init_i;
 *	int init_t;
 *	SYNTH_TYPE inv_type;
 *	int inv_number;		// I've forgotten its why - back end number?
 *	int samp_rate;
 *	int samp_size;		// in bytes: 1,2,3
 *	CHANNEL_TYPE channel;	// mono, first, second, both
 *
 *	char *counts;
 *	char *models;
 *	char *book;
 *	char *dphfile;
 *	char *namesfile;
 *
 */

   public:
   	char diphone_names[441][4];
   
	voice(const char *filename, const char *dirname, lang *parent_lang);
	~voice();

	void flush();
};

extern int n_langs;

extern lang  **langs;
extern lang  *this_lang;
extern voice *this_voice;

struct option;

extern option langoptlist[];
extern option voiceoptlist[];

