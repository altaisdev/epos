/*
 *	ss/src/daemon.cc
 *	(c) 1998 geo@ff.cuni.cz
 *
 *	Preliminary TCP daemon implementation.
 *	Delete, rewrite, edit.
 */

#include "common.h"

#define __UNIX__

#ifdef HAVE_UNISTD_H			// fork() only
	#include <unistd.h>	// in DOS, fork(){return -1;} is supplied in interf.cc
#endif

#ifdef HAVE_SYS_SOCKET_H
	#include <sys/socket.h>
#endif

#ifdef HAVE_NETINET_IN_H
	#include <netinet/in.h>
#endif

#ifdef HAVE_LINUX_IN_H
	#include <linux/in.h>
#endif



rules *ruleset = NULL;

int serve(int argc, char **args)
{
	unit *root;
	PARSER *parsie;

	ss_init(argc, args);
	parsie = cfg.input_text ? new PARSER(cfg.input_text, 1)
		: new PARSER(cfg.input_file, 0);
	root=new unit(U_TEXT, parsie);
	delete parsie;
	if (!ruleset) ruleset=new rules(cfg.rules_file, 0);  //int gets ignored
	ruleset->apply(root);
//	delete ruleset;
	fprintf(stddbg,"*********************************************\n");
	root->fout(NULL);

/*	if (cfg.use_diph) {
		synth *ds = new synth(cfg.inv_type, cfg.inv_counts,
			cfg.inv_models, cfg.inv_book, cfg.inv_dpt,
			cfg.inv_f0, cfg.inv_i0, cfg.inv_t0, cfg.inv_hz);
//		int ds_used_cnt = 0;
		if (cfg.play_diph) {
			if (cfg.forking) {
				switch (fork()) {
					case 0:	//ds_used_cnt++;
						play_diphones(root, ds);
						delete ds;
						return 0;
					case -1:play_diphones(root, ds);
					default:;
				}
			} else play_diphones(root, ds);
		}
		if (cfg.show_diph) show_diphones(root, ds);
		delete ds;
	}
	
*/

	if (cfg.use_diph) {
//		int ds_used_cnt = 0;
		if (cfg.play_diph) {
			if (cfg.forking) {
				switch (fork()) {
					case 0:	//ds_used_cnt++;
						play_diphones(root, syn[this_voice->type], this_voice);
						return 0;
					case -1:play_diphones(root, syn[this_voice->type], this_voice);
					default:;
				}
			} else play_diphones(root, syn[this_voice->type], this_voice);
		}
		if (cfg.show_diph) show_diphones(root);
		
	}

	if (cfg.neuronet) root->nnet_out("matlab.in", ".");
	delete(root);
	fprintf(stddbg,"***** The End. ******************************\n");
	ss_done();
	return 0;
}

#define LISTEN_PORT 8778
#define IPPROTO_TCP 6

int main()
{
	int s, f;
	unsigned int sia = sizeof(sockaddr);
	sockaddr_in sa;
	sockaddr_in ia;

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(0x7f000001);
	sa.sin_port = htons(LISTEN_PORT);
	if (bind(s, (sockaddr *)&sa, sizeof (sa))) shriek("Could not bind");
	if (listen(s, 5)) shriek("Could not listen");
	ia.sin_family = AF_INET;
	ia.sin_addr.s_addr = INADDR_ANY;
	ia.sin_port = 0;
	
	while(1) {
		f = accept(s, (sockaddr *)&ia, &sia);
		dup2(f, fileno(stderr));
		char buff [4000];
		read(f, buff, 3000);
		char *a[100];
		a[1] = buff;
		a[0] = "";
		char *p = buff;
		int i;
		printf("cyklus %s\n", p);
		for (i=1; *p; p++) {
			if (*p == '\034') {
				a[++i]=p+1; *p=0;
			}
		}
		serve(i, a);
		
		write(f,buff,1);
	}
	
	return 0;
}
