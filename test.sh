#!/bin/bash
#eval "epos_cv_$2=\'blabla\'"
#eval epos_cv_$2="blabla"
#outdir=''
#outfil=''
#dirs=`find ./$2 -type d`

bla="cfg"

dirs=`find ./$2 -type d`
sdscr=""
sdscr="'/^.\/$2$/d'"
dirs=`find ./$2 -type d | eval sed -r $sdscr`
#find ./$bla -type d | eval sed -r $sedscr
#outdir=`find ./$bla -type d | eval sed -r $sedscr`
echo $sdscr
echo $dirs

