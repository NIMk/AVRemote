SUBDIRS = upnp

bin_PROGRAMS = avremote

avremote_SOURCES = avremote.c avremote.h cmdline.c discover.c discover.h parsers.c parsers.h
avremote_LDADD = $(UPNP_LIBS)
avremote_CPPFLAGS = $(UPNP_CPPFLAGS)

