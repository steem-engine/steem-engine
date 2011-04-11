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
CFLAGS=-I$(ROOT)/include -I$(ROOT)/steem/code \
-I$(ROOT)/3rdparty/unrarlib \
-I$(ROOT)/3rdparty/zlib/contrib/minizip \
-I$(ROOT)/3rdparty/portaudio/src/common \
-I$(ROOT)/3rdparty/portaudio/include \
-I$(ROOT)/3rdparty/rtaudio \
-w -Wfatal-errors

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
      $(ROOT)/3rdparty/unrarlib/unrarlib.o \
			\
      $(ROOT)/3rdparty/portaudio/lib/libportaudio.la \
       \
      $(ROOT)/3rdparty/rtaudio/tests/Release/RtAudio.o

LIBTOOL=$(ROOT)/3rdparty/portaudio/libtool

# Objects
OBJS=./obj/asm_draw.o ./obj/emu.o ./obj/asm_osd.o ./obj/main.o ./obj/helper.o ./obj/resource.o
# OBJS=./code/draw_c/draw_c.o emu.o main.o urarlib.o helper.o resource.o

# Targets

all:	asm
	mkdir -p obj output
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
	$(CC) -o ./obj/helper.o -c $(ROOT)/steem/helper.cpp $(CFLAGS) $(STEEMFLAGS)

main:
	$(CC) -o ./obj/main.o -c $(ROOT)/steem/code/main.cpp $(CFLAGS) $(STEEMFLAGS)

emu:
	$(CC) -o ./obj/emu.o -c $(ROOT)/steem/emu.cpp $(CFLAGS) $(STEEMFLAGS)


asm:
	mkdir -p ./obj
	nasm -felf -o./obj/asm_draw.o -i$(ROOT)/steem/asm/ $(ROOT)/steem/asm/asm_draw.asm
	nasm -felf -o./obj/asm_osd.o -i$(ROOT)/steem/asm/ $(ROOT)/steem/asm/asm_osd_draw.asm

res:
	nasm -felf -o./obj/resource.o -i$(ROOT)/steem/ $(ROOT)/steem/rc/resource.asm

link: 
	$(LIBTOOL) --mode=link $(CC) -o $(OUTPUT) $(CFLAGS) $(OBJS) $(LIBS)

link_static:
	$(CC) -o $(OUTPUT) $(CFLAGS) $(OBJS) $(LIBS) -static


3rdparty:	zlib unrarlib portaudio rtaudio

unrarlib:
	gcc -c $(ROOT)/3rdparty/unrarlib/unrarlib/unrarlib.c $(CFLAGS) -o$(ROOT)/3rdparty/unrarlib/unrarlib.o

zlib:
	cd $(ROOT)/3rdparty/zlib; sh configure; make test
	cd $(ROOT)/3rdparty/zlib/contrib/minizip; make all

portaudio:
	cd $(ROOT)/3rdparty/portaudio; sh configure; make

rtaudio:
	cd $(ROOT)/3rdparty/rtaudio; sh configure --with-oss; cd tests; make

