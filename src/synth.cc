/*
 *	ss/src/synth.cc
 *	(c) 1998 geo@ff.cuni.cz
 *
 */

#include "common.h"
#include "ktdsyn.h"
//#include "lpcsyn.h"

synth *syn[ST_MAX];

void setup_syntheses()		//FIXME
{
	if (syn[S_KTD]) delete syn[S_KTD];
	syn[S_KTD] = new ktdsyn(cfg.ktd_pitch, cfg.ktd_speed);


/*		dsynth *ds = new dsynth(cfg.inv_type, cfg.inv_counts,
			cfg.inv_models, cfg.inv_book, cfg.inv_dpt,
			cfg.inv_f0, cfg.inv_i0, cfg.inv_t0, cfg.inv_hz);
*/	// FIXME!!!

}

#define BUFF_SIZE 1024	//every item is a quadruple of ints

void play_diphones(unit *root, synth *ds, voice *v)
{
	static diphone d[BUFF_SIZE]; 
	int i=BUFF_SIZE;
	int j;

	if (!cfg.play_diph && !cfg.show_diph) return;
	for (int k=0; i==BUFF_SIZE; k+=BUFF_SIZE) {
		i=root->write_diphs(d,k,BUFF_SIZE);
		for (j=0;j<i;j++) {
			d[j].f=this_voice->samp_rate * 100 / (d[j].f*cfg.inv_f0);
		}
		ds->syndiphs(v, d, i);
	}
}

void show_diphones(unit *root)
{
	static diphone d[BUFF_SIZE]; 
	int i=BUFF_SIZE;
	
	if (!cfg.show_diph) return;
	for (int k=0; i==BUFF_SIZE; k+=BUFF_SIZE) {
		i=root->write_diphs(d,k,BUFF_SIZE);
		for (int j=0;j<i;j++) {
			if (cfg.diph_raw) fprintf(stddbg,  "%5d", d[j].code);
			fprintf(stddbg," %3s f=%d t=%d i=%d\n", d[j].code<441 ? this_voice->diphone_names[d[j].code] : "?!", d[j].f, d[j].t, d[j].e);
		}
	}
}

#undef BUFF_SIZE

synth::synth()
{
	printf("FIXME synth::synth\n");
}

synth::~synth()
{
	printf("FIXME synth::~synth\n");
}

void
synth::syndiph(voice *v, diphone d)
{
	shriek("synth::syndiph is abstract\n");
}

void
synth::syndiphs(voice *v, diphone *d, int n)
{
	for (int i=0; i<n; i++) syndiph(v, d[i]);
}

//--------------------------------------

/*		dsynth *ds = new dsynth(cfg.inv_type, cfg.inv_counts,
			cfg.inv_models, cfg.inv_book, cfg.inv_dpt,
			cfg.inv_f0, cfg.inv_i0, cfg.inv_t0, cfg.inv_hz);
*/	// FIXME!!!
