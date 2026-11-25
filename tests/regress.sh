#!/bin/bash
# Regression test of the text processing front end.
#
# Starts a private eposd, feeds it the sentences in tests/corpus/*.txt and
# compares the phonetic transcription and the segment stream with the
# reference in tests/expected/.  The reference was produced by the 2012
# production binary (gcc 4.6, Ubuntu 12.04) running the same configuration.
#
# usage: tests/regress.sh [--update] [EPOSD [SAY [BASE_DIR]]]
#
# The pitch column of the segment dump is dropped before comparing: prosody
# rules pick among alternatives at random (srand(time) in src/interf.cc).
# The server is run with ---forking (i.e. forking off) to stay in the foreground.
# No speech inventories are needed.  The TTSCP port (8778) must be free,
# because the say-epos client has it compiled in.

set -u
here=$(cd "$(dirname "$0")" && pwd)
top=$(dirname "$here")
update=no
[ "${1:-}" = "--update" ] && { update=yes; shift; }
EPOSD=${1:-$top/src/eposd}
SAY=${2:-$top/src/say-epos}
BASE=${3:-$top/cfg}
out=$here/out
mkdir -p "$out"

fail() { echo "regress: $*" >&2; exit 2; }
[ -x "$EPOSD" ] || fail "no server binary at $EPOSD"
[ -x "$SAY" ] || fail "no client binary at $SAY"
[ -d "$BASE/lng" ] || fail "$BASE does not look like an Epos configuration directory"

probe() { (exec 3<>/dev/tcp/127.0.0.1/8778) 2>/dev/null; }
probe && fail "something already listens on TCP port 8778"

"$EPOSD" --base_dir "$BASE" ---forking --local_sound_device /dev/null >"$out/eposd.log" 2>&1 &
server=$!
trap 'kill $server 2>/dev/null; wait $server 2>/dev/null' EXIT INT TERM
i=0
until probe; do
	i=$((i + 1))
	kill -0 $server 2>/dev/null || { cat "$out/eposd.log" >&2; fail "eposd exited during startup"; }
	[ $i -gt 100 ] && fail "eposd did not start listening within 20 s"
	sleep 0.2
done

# After transcribing, the client also asks for the sentence to be spoken, which
# fails (with the installation path in the message) when inventories are absent.
noise='^Failed to read inventory from '
status=0
for corpus in "$here"/corpus/*.txt; do
	lang=$(basename "$corpus" .txt)
	result=$out/$lang.out
	: >"$result"
	n=0
	iconv -f UTF-8 -t ISO-8859-2 "$corpus" >"$out/$lang.l2" || fail "cannot recode $corpus"
	while IFS= read -r line; do
		n=$((n + 1))
		echo "### $n text" >>"$result"
		"$SAY" --language "$lang" "$line" 2>/dev/null | grep -a -v "$noise" >>"$result"
		echo "### $n segments" >>"$result"
		"$SAY" -d --language "$lang" "$line" 2>/dev/null | grep -a -v "$noise" |
			LC_ALL=C awk '/^ *[0-9]+ +- +[0-9]+ +[0-9]+ +[0-9]+ *$/ { print $1, $4, $5; next } { print }' >>"$result"
	done <"$out/$lang.l2"
	kill -0 $server 2>/dev/null || { status=1; echo "FAIL $lang: eposd died" >&2; break; }
	if [ $update = yes ]; then
		mkdir -p "$here/expected"
		cp "$result" "$here/expected/$lang.out" || fail "cannot write the reference"
		echo "updated tests/expected/$lang.out ($n sentences)"
	elif cmp -s "$result" "$here/expected/$lang.out"; then
		echo "PASS $lang ($n sentences)"
	else
		status=1
		echo "FAIL $lang: output differs from tests/expected/$lang.out" >&2
		LC_ALL=C diff "$here/expected/$lang.out" "$result" | head -20 | iconv -f ISO-8859-2 -t UTF-8 >&2
	fi
done
exit $status
