/***********************************************************
 *
 * epos/src/tdpsyn.cc
 *
 * Copyright (c) 2000-2001 , Petr Horak (petr.horak@click.cz).
 *
 * Version 1.09 (11.6.2001)
 *
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License in doc/COPYING for more details.
 *	
 *
 */

#include "common.h"
#include "tdpsyn.h"
#include <math.h>

int hamkoe(int winlen, double *data)
{
	int i;
	double fn;
	fn = 2 * pii / (winlen - 1);
	for (i=0;i<winlen;i++) data[i] = 0.54 - 0.46 * cos(fn * i);
	return 0;
}

int median(int lold, int lact, int lnext, int ibonus)
{
  int lm;
  if (lold>lact && lold<lnext) lm = lold;
  else if (lold<lact && lold>lnext) lm = lold;
       else if (lnext>lold && lnext<lact) lm = lnext;
            else if (lnext<lold && lnext>lact) lm = lnext;
                 else lm = lact;
  return (abs(lact-lm)>ibonus) ? lm : lact;
}

tdpsyn::tdpsyn(voice *v)
{
//	const char *ppuls = "ppuls";
	int i, bufpos, pppos, pom;

	wwin = (double *)xmalloc(sizeof(double) * 2048);
	tdp_buff = (t_samp *)xmalloc(sizeof(t_samp) * v->n_segs * 16 * 768);  //memory for inventory
	out_buff = (t_samp *)xmalloc(sizeof(t_samp) * 2048);
	for (i=0;i<2048;i++) { out_buff[i] = 0; wwin[i] = 0; }
	ppulses = (int *)xmalloc(sizeof(int) * v->n_segs * 64);
	diph_offs = (int *)xmalloc(sizeof(int) * v->n_segs);
	diph_len = (int *)xmalloc(sizeof(int) * v->n_segs);
	bufpos = 0;
	outpos = 1024;
	pppos = 0;
	difpos = 0;
	for (i=0;i<v->n_segs;i++) { diph_offs[i] = 0; diph_len[i] = 0; }
	diph_offs[0] = 0;

	FILE *ustream;

	char *filename = compose_pathname(v->models, v->loc, cfg->inv_base_dir);
	ustream = fopen(filename, "rb" );
	if(!ustream) shriek(445, fmt("cannot find tdi units file %s", filename));
    else
    {
		pom = getw(ustream); //if (pi != 0) ; //tdp
		pom = getw(ustream); //v->samp_rate
		pom = getw(ustream); //sizeof(t_samp)
		bufpos = getw(ustream);

		pom = getw(ustream); //diphs v->n_segs
		pom = getw(ustream); //diph_offs
		pom = getw(ustream); //diph_len
		pom = getw(ustream); //res

		pom = getw(ustream); //res
		pppos = getw(ustream); //ppulses
		pom = getw(ustream); //res
		pom = getw(ustream); //res

		fread(tdp_buff, sizeof(t_samp), bufpos, ustream);
		fread(diph_offs, sizeof(int), v->n_segs, ustream);
		fread(diph_len, sizeof(int), v->n_segs, ustream);
		fread(ppulses, sizeof(int), pppos, ustream);
  		fclose(ustream);
		//printf("TDI OK! bufpos = %d, pppos = %d\n",bufpos,pppos);
	} 
}

tdpsyn::~tdpsyn(void)
{
	free(diph_len);
	free(diph_offs);
	free(ppulses);
	free(out_buff);
	free(tdp_buff);
	free(wwin);
}

void tdpsyn::synseg(voice *, segment d, wavefm *w)
{
	const int npitch = 145;
	int i, j, pitch, avpitch, pompitch, pitchlen, origlen, newlen, diflen, maxwin, step;
	//int lpitch, rpitch;
	double ene_k, tim_k;
	t_samp poms;

	if (diph_len[d.code] == 0) printf("missing diphone: %d - T:%d I:%d L:%d\n",d.code,d.t,d.e,d.f);
	//else printf("diphone: %d - T:%d I:%d L:%d\n",d.code,d.t,d.e,d.f);

	step = 1;;
	avpitch = 0; maxwin = 0;
	i = 0;
	for (j=0;j<=diph_len[d.code]+1;j++) {
		pompitch = ppulses[diph_offs[d.code]+j] - ppulses[diph_offs[d.code]+j-1];
		if (pompitch < 2*npitch) {
			avpitch += pompitch;
//			if (pompitch > maxwin) maxwin = pompitch;
			i++;
		}
	}
	if (i > 0) avpitch /= i; else {
		avpitch = 160;
		if (cfg->paranoid) shriek(463,"pitch marks not found");
    }
	maxwin = avpitch + 30; //(int)(w->samp_rate / 500);
	pitch = d.f;
	maxwin = (pitch > maxwin) ? maxwin : pitch;
	origlen = avpitch * diph_len[d.code];
	pitchlen = pitch * diph_len[d.code];
	tim_k = (double)origlen * 100 / pitchlen / d.t;
	newlen = (int)(pitchlen * tim_k);
	diflen = (int)((double)(newlen - origlen) / diph_len[d.code]);
	ene_k = (double)d.e / 100;
//	printf("\navp=%d oril=%d newl=%d difl=%d| l:%d t:%f(%f) e:%f\n",avpitch,origlen,newlen,diflen,pitch,tim_k,(double)d.t/100,ene_k);
//	printf("\navp=%d L=%d oril=%d pil=%d tk:%f newl=%d difl=%d| %d (%d)\n",avpitch,pitch,origlen,pitchlen,tim_k,newlen,diflen,diph_len[d.code],d.code);

	hamkoe(2 * maxwin + 1, wwin);
//	hamkoe(2 * avpitch + 1, wwin);
    while (diflen > pitch) { step++; diflen -= pitch; }
    while (-diflen > pitch) { step++; diflen += pitch; }
//	while (diflen > pitch || -diflen > pitch) { step++; diflen = diflen % 2; }
	for (j=1;j<=diph_len[d.code];j=j+step) {
		for (i=1024;i<=1024+pitch;i++) out_buff[i-pitch] = out_buff[i];
		for (i=1025;i<2048;i++) out_buff[i] = 0;
		for (i=-maxwin;i<=maxwin;i++) {
//		lpitch = ppulses[diph_offs[d.code]+j] - ppulses[diph_offs[d.code]+j-1];
//		rpitch = ppulses[diph_offs[d.code]+j+1] - ppulses[diph_offs[d.code]+j];
//		for (i=-lpitch;i<=rpitch;i++) {
			poms = (t_samp)(wwin[i+pitch]*tdp_buff[i+ppulses[diph_offs[d.code]+j-1]]);
			poms = (t_samp)(poms * ene_k);
			if (i<=0) out_buff[1024+i] = out_buff[1024+i] + poms;
			else out_buff[1024+i] = poms;
		}
		for (i=1024-pitch;i<1024;i++) w->sample(out_buff[i]);
		for (i=0;i<1024;i++) out_buff[i] = 0;
		difpos += diflen;
		if (difpos < -pitch) { j--; difpos += pitch; }
		if (difpos > pitch) { j++; difpos -= pitch; }
		//printf("dp=%d ",difpos);
	}
}
