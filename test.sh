#!/bin/bash
#eval "epos_cv_$2=\'blabla\'"
#eval epos_cv_$2="blabla"
#outdir=''
#outfil=''
#dirs=`find ./$2 -type d`

bla="cfg"

dirs=`find ./$2 -type d`
sdscr="'s:^./$2/::; s:$:\t:; s:$:\\\:; \$s:\\\\$::'"
evaled="sed -r $sdscr"
dirs=`find ./$2 -type d | eval sed -r $sdscr`
#find ./$bla -type d | eval sed -r $sedscr
#outdir=`find ./$bla -type d | eval sed -r $sedscr`
echo $sdscr
echo $evaled
echo $dirs

