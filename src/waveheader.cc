
Neco s tim musim udelat


/*	if (cfg.wav_header) {					//naplneni hlavicky wav
		strcpy(wavh.string1,"RIFF");
		strcpy(wavh.string2,"WAVEfmt ");
		strcpy(wavh.string3,"data");
		wavh.datform=1;
		wavh.numchan=1;
		wavh.sf1=cfg.samp_hz; wavh.sf2=0;
		wavh.avr1=2*cfg.samp_hz; wavh.avr2=0;
		wavh.wlenB=2; wavh.wlenb=8;
		wavh.xnone=0x010;
		wavh.dlen=0;
		wavh.flen=0+0x24;
		write(wavout, &wavh, 44);         //zapsani prazdne wav hlavicky na zacatek souboru
	}





	if (cfg.wav_header) {
		lseek(wavout, 0, SEEK_SET);                 //navrat na zacatek
		wavh.dlen=sizeof(sample2_t) * pocet_vzorku;  //a doplneni udaju o delce
		wavh.flen=wavh.dlen+0x24;                   //do wav hlavicky
		write(wavout, &wavh, 44);
	}
