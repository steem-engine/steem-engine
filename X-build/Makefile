#####################################
#      Makefile for Steem           #
#####################################

# requires tools: make, gcc, g++, nasm
# requires libs: rtaudio, portaudio, zlib, minizip, urarlib, X

# symbolic links - code, inc, 3rdparty

ROOT=..

# Compiler
CC=g++

# Parameters given to the compiler
STEEMFLAGS=-DUNIX -DLINUX
CFLAGS=-I$(ROOT)/include -I$(ROOT)/steem/code -I$(ROOT)/3rdparty -Wall -w

RELEASE_CFLAGS=-O

# Output filename
OUTPUT="./output/steem"

LIBS=-lX11 -lXext -lpthread -lXxf86vm \
      $(ROOT)/3rdparty/zlib/adler32.o  \
      $(ROOT)/3rdparty/zlib/compress.o  \
      $(ROOT)/3rdparty/zlib/crc32.o  \
      $(ROOT)/3rdparty/zlib/deflate.o  \
      $(ROOT)/3rdparty/zlib/gzclose.o  \
      $(ROOT)/3rdparty/zlib/gzlib.o  \
      $(ROOT)/3rdparty/zlib/gzread.o  \
      $(ROOT)/3rdparty/zlib/gzwrite.o  \
      $(ROOT)/3rdparty/zlib/infback.o  \
      $(ROOT)/3rdparty/zlib/inffast.o  \
      $(ROOT)/3rdparty/zlib/inflate.o  \
      $(ROOT)/3rdparty/zlib/inftrees.o  \
      $(ROOT)/3rdparty/zlib/trees.o  \
      $(ROOT)/3rdparty/zlib/uncompr.o  \
      $(ROOT)/3rdparty/zlib/zutil.o  \
      $(ROOT)/3rdparty/zlib/contrib/minizip/unzip.o \
      $(ROOT)/3rdparty/zlib/contrib/minizip/zip.o \
      $(ROOT)/3rdparty/zlib/contrib/minizip/ioapi.o \
      $(ROOT)/3rdparty/urarlib/urarlib.o  \
			\
      $(ROOT)/3rdparty/portaudio/pa_common/pa_lib.o \
      $(ROOT)/3rdparty/portaudio/pa_unix_oss/pa_unix_oss.o \
      $(ROOT)/3rdparty/portaudio/pa_unix_oss/pa_unix.o \
       \
      $(ROOT)/3rdparty/rtaudio/RtAudio.o

# Objects
OBJS=./obj/asm_draw.o ./obj/emu.o ./obj/asm_osd.o ./obj/main.o ./obj/helper.o ./obj/resource.o
# OBJS=./code/draw_c/draw_c.o emu.o main.o helper.o resource.o

# Targets

all:
	$(MAKE) clean
	mkdir obj output
	$(MAKE) res
	$(MAKE) asm
	$(MAKE) helper
	$(MAKE) main
	$(MAKE) emu
	$(MAKE) link

clean:
	rm -rf obj output

test:
	$(MAKE) all
	$(OUTPUT)

release:
	$(MAKE) clean
	$(MAKE) res
	$(MAKE) asm
	$(MAKE) release_compile
	$(MAKE) link_static
	strip -s $(OUTPUT)

helper:
	$(CC) -o ./obj/helper.o -c $(ROOT)/steem/helper.cpp $(CFLAGS) $(STEEMFLAGS)

main:
	$(CC) -o ./obj/main.o -c $(ROOT)/steem/code/main.cpp $(CFLAGS) $(STEEMFLAGS)

emu:
	$(CC) -o ./obj/emu.o -c $(ROOT)/steem/emu.cpp $(CFLAGS) $(STEEMFLAGS)

release_compile:
	$(CC) -o ./obj/helper.o -c $(ROOT)/steem/helper.cpp $(CFLAGS) $(RELEASE_CFLAGS) $(STEEMFLAGS)
	$(CC) -o ./obj/main.o -c $(ROOT)/steem/code/main.cpp $(CFLAGS) $(RELEASE_CFLAGS) $(STEEMFLAGS)
	$(CC) -o ./obj/emu.o -c $(ROOT)/steem/emu.cpp $(CFLAGS) $(RELEASE_CFLAGS) $(STEEMFLAGS)

asm:
	nasm -felf -o./obj/asm_draw.o -i$(ROOT)/steem/asm/ $(ROOT)/steem/asm/asm_draw.asm
	nasm -felf -o./obj/asm_osd.o -i$(ROOT)/steem/asm/ $(ROOT)/steem/asm/asm_osd_draw.asm

res:
	nasm -felf -o./obj/resource.o -i$(ROOT)/steem/ $(ROOT)/steem/rc/resource.asm

link: 
	$(CC) -o $(OUTPUT) $(CFLAGS) $(OBJS) $(LIBS)

link_static:
	$(CC) -o $(OUTPUT) $(CFLAGS) $(OBJS) $(LIBS) -static
