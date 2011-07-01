SUBDIRS = upnp

bin_PROGRAMS = avremote

avremote_SOURCES = avremote.c cmdline.c discover.c parsers.c
avremote_LDADD = $(UPNP_LIBS)
avremote_CPPFLAGS = $(UPNP_CPPFLAGS)

noinst_HEADERS = avremote.h parsers.h
