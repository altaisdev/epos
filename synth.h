/*
 *	ss/src/synth.h
 *	(c) 1998 Jirka Hanika, geo@ff.cuni.cz
 */

#define  sample_t		short

typedef struct
{
	char string1[4];
	long flen;
	char string2[8];
	long xnone;
	short int  datform, numchan, sf1, sf2, avr1, avr2, wlenB, wlenb;
	char string3[4];
	long dlen;
 } wavehead;            // wav file header

class synth
{
	unsigned int crc;
//	wavehead wavh;
   public:
	synth();
/*	synth(SYNTH_TYPE type, const char *counts_file, const char *models_file,
			const char *codebook_file, const char *dpt_file,
			int f, int i, int t, int sampling_rate);*/
	virtual ~synth(void);
	virtual void syndiph(voice *v, diphone d) = 0;  /* Any subclass must define syndiph or syndiphs */
	void syndiphs(voice *v, diphone *d, int count);
};

extern synth *syn[];

void play_diphones(unit *root, voice *v);
void show_diphones(unit *root);

