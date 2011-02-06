#####################################
#      Makefile for Steem           #
#####################################

# requires tools: make, gcc, g++, nasm
# requires libs: rtaudio, portaudio, zlib, minizip, urarlib, X

# symbolic links - code, inc, 3rdparty, make_...

STEEMDIR=../steem/code

# Compiler
CC=g++

# Parameters given to the compiler
STEEMFLAGS=-DUNIX -DLINUX
CFLAGS=-I../steem/inc -I../steem/code -I../3rdparty -I/usr/X11/include -L/usr/X11R6/lib -Wall 

RELEASE_CFLAGS=-O

# Output filename
OUTPUT="./steem"

LIBS=-lX11 -lXext -lpthread -lXxf86vm \
      ../3rdparty/zlib/crc32.o  \
      ../3rdparty/zlib/inflate.o \
      ../3rdparty/zlib/adler32.o \
      ../3rdparty/zlib/trees.o \
      ../3rdparty/zlib/inffast.o \
      ../3rdparty/zlib/inftrees.o \
      ../3rdparty/zlib/deflate.o \
      ../3rdparty/zlib/zutil.o \
      ../3rdparty/zlib/compress.o \
      ../3rdparty/zlib/contrib/minizip/unzip.o\
      ../3rdparty/zlib/contrib/minizip/ioapi.o\
      ../3rdparty/portaudio/pa_common/pa_lib.o \
      ../3rdparty/portaudio/pa_unix_oss/pa_unix_oss.o \
      ../3rdparty/portaudio/pa_unix_oss/pa_unix.o \
      ../3rdparty/rtaudio/RtAudio.o

# Objects
OBJS=obj/asm_draw.o obj/emu.o obj/asm_osd.o obj/main.o obj/urarlib.o obj/helper.o obj/resource.o
# OBJS=./code/draw_c/draw_c.o emu.o main.o urarlib.o helper.o resource.o

# Targets

all:
	./make_res
	$(MAKE) helper
	$(MAKE) main
	$(MAKE) emu
	$(MAKE) link

test:
	$(MAKE) all
	$(OUTPUT)

release:
	./make_res
	./make_osd
	./make_draw
	$(MAKE) release_compile
	$(MAKE) link_static
	strip -s $(OUTPUT)

helper:
	$(CC) -c $(STEEMDIR)/helper.cpp $(CFLAGS) $(STEEMFLAGS)

main:
	$(CC) -c $(STEEMDIR)/main.cpp $(CFLAGS) $(STEEMFLAGS)

emu:
	$(CC) -c $(STEEMDIR)/emu.cpp $(CFLAGS) $(STEEMFLAGS)

release_compile:
	$(CC) -c $(STEEMDIR)/helper.cpp $(CFLAGS) $(RELEASE_CFLAGS) $(STEEMFLAGS)
	$(CC) -c $(STEEMDIR)/main.cpp $(CFLAGS) $(RELEASE_CFLAGS) $(STEEMFLAGS)
	$(CC) -c $(STEEMDIR)/emu.cpp $(CFLAGS) $(RELEASE_CFLAGS) $(STEEMFLAGS)

link: 
	$(CC) -o $(OUTPUT) $(CFLAGS) $(OBJS) $(LIBS)

link_static:
	$(CC) -o $(OUTPUT) $(CFLAGS) $(OBJS) $(LIBS) -static

urarlib:
	gcc -c ../3rdparty/urarlib/urarlib.c $(CFLAGS) -ourarlib.o
