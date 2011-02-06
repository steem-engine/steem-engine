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
CFLAGS=-I$(ROOT)/include -I$(ROOT)/steem/code -I$(ROOT)/3rdparty -Wall 

RELEASE_CFLAGS=-O

# Output filename
OUTPUT="./output/steem"

LIBS=-lX11 -lXext -lpthread -lXxf86vm \
      $(ROOT)/3rdparty/zlib/crc32.o  \
      $(ROOT)/3rdparty/zlib/inflate.o \
      $(ROOT)/3rdparty/zlib/adler32.o \
      $(ROOT)/3rdparty/zlib/trees.o \
      $(ROOT)/3rdparty/zlib/inffast.o \
      $(ROOT)/3rdparty/zlib/inftrees.o \
      $(ROOT)/3rdparty/zlib/deflate.o \
      $(ROOT)/3rdparty/zlib/zutil.o \
      $(ROOT)/3rdparty/zlib/compress.o \
      $(ROOT)/3rdparty/zlib/contrib/minizip/unzip.o \
      $(ROOT)/3rdparty/zlib/contrib/minizip/ioapi.o \
			\
      $(ROOT)/3rdparty/portaudio/pa_common/pa_lib.o \
      $(ROOT)/3rdparty/portaudio/pa_unix_oss/pa_unix_oss.o \
      $(ROOT)/3rdparty/portaudio/pa_unix_oss/pa_unix.o \
       \
      $(ROOT)/3rdparty/rtaudio/RtAudio.o

# Objects
OBJS=./obj/asm_draw.o ./obj/emu.o ./obj/asm_osd.o ./obj/main.o ./obj/urarlib.o ./obj/helper.o ./obj/resource.o
# OBJS=./code/draw_c/draw_c.o emu.o main.o urarlib.o helper.o resource.o

# Targets

all:
	rm -rf obj output
	mkdir obj output
	$(MAKE) res
	$(MAKE) helper
	$(MAKE) main
	$(MAKE) emu
	$(MAKE) link

clean:
	rm -rf obj output

fresh:
	$(MAKE) clean
	$(MAKE) urarlib
	$(MAKE) asm
	$(MAKE) all

test:
	$(MAKE) all
	$(OUTPUT)

release:
	$(MAKE) clean
	$(MAKE) res
	$(MAKE) urarlib
	$(MAKE) asm
	$(MAKE) release_compile
	$(MAKE) link_static
	strip -s $(OUTPUT)

helper:
	$(CC) -c $(ROOT)/steem/helper.cpp $(CFLAGS) $(STEEMFLAGS)

main:
	$(CC) -c $(ROOT)/steem/code/main.cpp $(CFLAGS) $(STEEMFLAGS)

emu:
	$(CC) -c $(ROOT)/steem/emu.cpp $(CFLAGS) $(STEEMFLAGS)

release_compile:
	$(CC) -c $(ROOT)/steem/helper.cpp $(CFLAGS) $(RELEASE_CFLAGS) $(STEEMFLAGS)
	$(CC) -c $(ROOT)/steem/code/main.cpp $(CFLAGS) $(RELEASE_CFLAGS) $(STEEMFLAGS)
	$(CC) -c $(ROOT)/steem/emu.cpp $(CFLAGS) $(RELEASE_CFLAGS) $(STEEMFLAGS)

asm:
	nasm -felf -o./obj/asm_draw.o -i$(ROOT)/steem/asm/ $(ROOT)/steem/asm/asm_draw.asm
	nasm -felf -o./obj/asm_osd.o -i$(ROOT)/steem/asm/ $(ROOT)/steem/asm/asm_osd_draw.asm

res:
	nasm -felf -o./obj/resource.o -i$(ROOT)/steem/ $(ROOT)/steem/rc/resource.asm

link: 
	$(CC) -o $(OUTPUT) $(CFLAGS) $(OBJS) $(LIBS)

link_static:
	$(CC) -o $(OUTPUT) $(CFLAGS) $(OBJS) $(LIBS) -static



urarlib:
	gcc -c $(ROOT)/3rdparty/urarlib/urarlib.c $(CFLAGS) -o./obj/urarlib.o

