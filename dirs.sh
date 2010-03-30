#!/bin/bash

for var in $@; do
	outdir=''
	outfil=''
	sedscript="'/^.\/$var$/d; s:^./$var/::; s:$:\t\\\:; \$s:\\\\$::; '"
	outdir=`find ./$var -type d | eval sed -r $sedscript`
		
	sedscript="'/^.\/$var$/d; /^.\/$var\/[a-z0-9]*\//d'"
	dirs=`find ./$var -type d | eval sed -r $sedscript`
#	echo "$dirs" > 1
	sedscript="'s:^./$var/::; s:$:\t\\\:; \$s:\\\\$::'"
	for d in $dirs; do
		fls=`find $d -type f | eval sed -r $sedscript`
		if test -n "$fls"; then
			if test -z "$outfil"; then
				outfil="$fls"
			else
				outfil="$outfil\\
$fls"
			fi
		fi
	done
	echo "DATA_DIRS=$outdir" > $var/data_dirs.am
	echo "DATA_FILES=$outfil" > $var/data_files.am
done

#if test -n "$outdir"; then 
#	if test -n "$outfil"; then
		#epos_cv_$var="checked"
#	fi
#fi

#if test "x${epos_cv_$var}" = "xchecked"; then
	
	
#fi
