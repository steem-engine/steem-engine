#URLGET=wget
#URLGET=curl -O
URLGET=$(shell which wget || echo "`which curl` -O")
TAR=tar
MD5SUM=md5sum

UNRARLIBVER=040
UNRARLIBARCHIVE=unrarlib$(UNRARLIBVER).tar.bz2
UNRARLIBURL=http://www.unrarlib.org/download/$(UNRARLIBARCHIVE)

PORTAUDIOVER=v19_20140130
PORTAUDIOARCHIVE=pa_stable_$(PORTAUDIOVER).tgz
PORTAUDIOURL=http://www.portaudio.com/archives/$(PORTAUDIOARCHIVE)

#RTAUDIOVER=4.1.2
RTAUDIOVER=3.0.3
RTAUDIOARCHIVE=rtaudio-$(RTAUDIOVER).tar.gz
RTAUDIOURL=http://www.music.mcgill.ca/~gary/rtaudio/release/$(RTAUDIOARCHIVE)

ZLIBVER=1.2.8
ZLIBARCHIVE=zlib-$(ZLIBVER).tar.xz
ZLIBARCHIVEMD5=28f1205d8dd2001f26fec1e8c2cebe37
ZLIBURL=http://zlib.net/$(ZLIBARCHIVE)

all:	unrarlib portaudio rtaudio zlib

clean:
	$(RM) -r unrarlib* portaudio* rtaudio* zlib*

unrarlib:	unrarlib.dir
	ln -s $(shell cat unrarlib.dir) unrarlib

unrarlib.dir:	$(UNRARLIBARCHIVE)
	$(TAR) xvjf $^ | tail -n 1 | cut -d / -f 1 > $@

$(UNRARLIBARCHIVE):
	$(URLGET) $(UNRARLIBURL)

portaudio:	portaudio.ready

portaudio.ready:	$(PORTAUDIOARCHIVE)
	$(TAR) xvzf $^
	touch portaudio.ready

$(PORTAUDIOARCHIVE):
	$(URLGET) $(PORTAUDIOURL)

rtaudio:	rtaudio.dir
	ln -s $(shell cat $^) $@

rtaudio.dir:	$(RTAUDIOARCHIVE)
	$(TAR) xvzf $^ | tail -n 1 | cut -d / -f 1 > $@

$(RTAUDIOARCHIVE):
	$(URLGET) $(RTAUDIOURL)

zlib:	zlib.dir
	ln -s $(shell cat $^) $@

zlib.dir:	$(ZLIBARCHIVE)
	$(TAR) xvJf $^ | tail -n 1 | cut -d / -f 1 > $@

$(ZLIBARCHIVE):
	$(URLGET) $(ZLIBURL)
	test "$(ZLIBARCHIVEMD5)" = "`$(MD5SUM) $(ZLIBARCHIVE) | cut -f 1 -d ' '`" || (echo "ERROR : MD5 mismatch" ; rm $(ZLIBARCHIVE) )
