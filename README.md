# Epos

Epos is a language independent rule-driven text-to-speech system, developed
since 1996 mainly by Jirka Hanika and Petr Horák.  It ships with rules for
Czech and Slovak, runs as a daemon (`eposd`) speaking the TTSCP protocol and
comes with a small command line client (`say-epos`).  A public instance lives
at <http://epos.speech.cz>.

Epos is free software distributed under the GNU General Public License,
version 2 or later; see `doc/COPYING`.

## Status

This repository holds the whole release history, reconstructed from the
distribution archives (1998–2012).  Every release is tagged `v<version>`.

| Branch | Contents |
|---|---|
| `main` | the stable 2.4 line, continuing from the tree deployed at epos.speech.cz |
| `devel-2.5` | the development line up to 2.5.37 (2006), kept for reference |

Version 2.4.87 is the first one that builds with a current compiler again.
It still has to be built as a **32-bit** program: pointers are stored in
`int`s in many places.  A native 64-bit port is the next step.

## Building

On Debian or Ubuntu:

```sh
sudo apt install build-essential g++-multilib autoconf automake libtool
./autogen.sh
./configure CXX="g++ -m32" CC="gcc -m32"
make
sudo make install
```

`make install` puts the binaries into `/usr/local/bin` and the language
configuration into `/usr/local/share/epos`.  Use `--prefix` to change that.
PulseAudio output is compiled in when a matching (32-bit) libpulse is
installed; `--disable-pulseaudio` turns it off.

Speech segment inventories are distributed separately, see `cfg/inv/README`.
Without them Epos can transcribe text but cannot produce sound.

## Running

```sh
eposd                                  # start the daemon (TTSCP on port 8778, localhost only)
say-epos -l                            # list languages and voices
say-epos --language czech 'Ahoj.'      # print the phonetic transcription and speak
say-epos -o --language czech 'Ahoj.' > ahoj.wav
```

Text goes in and out in ISO 8859-2.  The configuration directory must come
from the same version as the binaries (`eposd` checks the `version` file).

## Tests

```sh
tests/regress.sh src/eposd src/say-epos /usr/local/share/epos
```

runs the sentences in `tests/corpus/` through a private `eposd` and compares
the phonetic transcription and the segment stream with `tests/expected/`.
The reference output was produced by the binary built in 2012 with gcc 4.6,
so the test guards against behaviour changes introduced by porting.  It needs
no inventories.  TCP port 8778 has to be free while it runs.

## Documentation

`doc/english/*.sgml` (also pre-rendered as `doc/english/epos*.html`) covers the
rule language, the configuration options, TTSCP and the internals.
`doc/Changes` is the changelog since 1997.
