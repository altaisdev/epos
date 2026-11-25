##	Process this file with automake run in the top directory to yield Makefile.in

include $(top_srcdir)/Makefile.common

#SUBDIRS = nnet . tests
SUBDIRS = .

bin_PROGRAMS = eposd say-epos
# noinst_PROGRAMS = eposm gauge

#hashd.cc
SRC_BASE = parser.cc unit.cc rule.cc \
text.cc encoding.cc interf.cc function.cc options.cc \
synth.cc waveform.cc voice.cc client.cc \
lpcsyn.cc tdpsyn.cc tcpsyn.cc ktdsyn.cc ptdsyn.cc \
common.h defaults.h unit.h parser.h rule.h text.h encoding.h interf.h \
function.h options.h hash.h synth.h voice.h exc.h \
waveform.h lpcsyn.h tdpsyn.h tcpsyn.h ktdsyn.h ptdsyn.h \
slab.h client.h agent.h pa_asyn.h

# hashd.cc includes hash.cc, which in turn includes hashtmpl.cc; compiling
# hash.cc separately as well would define everything twice.
eposd_SOURCES = daemon.cc agent.cc ttscp.cc nonblock.cc hashd.cc $(SRC_BASE)
#eposm_SOURCES = monolith.cc hash.cc $(SRC_BASE)

say_epos_SOURCES = say.cc

#AM_CXXFLAGS+=@EXTRA_PORTAUDIO_DEFS@
#AM_CXXFLAGS+=@INCLUDE_PULSE_LIBS@

#LDADD_OTHER = @EXTRA_REGEX_OBJ@ @EXTRA_PORTAUDIO_LIBS@
LDADD_OTHER = @EXTRA_REGEX_OBJ@

eposd_LDADD = $(LDADD_OTHER)
#eposm_LDADD = $(LDADD_OTHER)

#nnet/libeposnnet.la:
#	@cd nnet && $(MAKE) $(AM_MAKEFLAGS) libeposnnet.la

EXTRA_DIST = options.lst block.cc exc.cc hash.cc hashtmpl.cc \
qnxipc.cc

cfginst:
	cd .. && make install
