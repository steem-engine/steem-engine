############################################
#      Makefile for Cygwin Steem           #
############################################

# Requires tools: gcc, g++, nasm
# Requires symlinks: ./code, ./inc, ./3rdparty, ./make_...
# Requires libraries: RtAudio, Zlib, PortAudio, X, winmm, urarlib

STEEMDIR=./code

# Compiler
CC=g++

# Parameters given to the compiler
STEEMFLAGS=-DUNIX -DCYGWIN -DNO_RTAUDIO
# STEEMFLAGS=-DNO_486_ASM -DENABLE_LOGFILE -DUNIX -DCYGWIN
CFLAGS=-I./inc -I$(STEEMDIR) -I/usr/X11R6/include -I./3rdparty -Wall -L/usr/X11R6/lib/

RELEASE_CFLAGS=-O 
# RELEASE_CFLAGS=-Wall $(RELEASE_CFLAGS)

# Output filename
OUTPUT="steem.exe"

# Libraries
LIBS=-lX11 -lXext ./3rdparty/portaudio/lib/libportaudio.a -lwinmm  \
      ./3rdparty/zlib/crc32.o  \
      ./3rdparty/zlib/inflate.o \
      ./3rdparty/zlib/adler32.o \
      ./3rdparty/zlib/trees.o \
      ./3rdparty/zlib/inffast.o \
      ./3rdparty/zlib/inftrees.o \
      ./3rdparty/zlib/deflate.o \
      ./3rdparty/zlib/zutil.o \
      ./3rdparty/zlib/compress.o \
      ./3rdparty/zlib/contrib/minizip/unzip.o \
      ./3rdparty/zlib/contrib/minizip/ioapi.o \
			

# Objects
OBJS=asm_draw.o emu.o asm_osd.o main.o urarlib.o helper.o resource.o
DRAWCOBJS=draw_c.o emu.o main.o urarlib.o helper.o resource.o

# Targets

all: 
	$(MAKE) res
	$(MAKE) helper
	$(MAKE) main
	$(MAKE) emu
	$(MAKE) link

link:
	$(MAKE) link_draw_c

a:
	$(MAKE) res
	$(MAKE) helper
	$(MAKE) main
	$(MAKE) emu
	$(MAKE) link_asm

res:	
	$(MAKE) res_c

clean:
	rm -f *.o
	rm -f $(OUTPUT)

fresh: 
	$(MAKE) clean
	$(MAKE) urarlib
	$(MAKE) draw_c
	$(MAKE) asm
	make 

test: 
	$(MAKE) all
	./$(OUTPUT)

release: 
	$(MAKE) clean
	$(MAKE) urarlib
	$(MAKE) draw_c
	$(MAKE) asm
	$(CC) -c $(STEEMDIR)/helper.cpp $(CFLAGS) $(RELEASE_CFLAGS) $(STEEMFLAGS)
	$(CC) -c $(STEEMDIR)/main.cpp $(CFLAGS) $(RELEASE_CFLAGS) $(STEEMFLAGS)
	$(CC) -c $(STEEMDIR)/emu.cpp $(CFLAGS) $(RELEASE_CFLAGS) $(STEEMFLAGS)
	$(MAKE) link
	strip -s $(OUTPUT)

helper:
	$(CC) -c $(STEEMDIR)/helper.cpp $(CFLAGS) $(STEEMFLAGS)

main:
	$(CC) -c $(STEEMDIR)/main.cpp $(CFLAGS) $(STEEMFLAGS)

emu:
	$(CC) -c $(STEEMDIR)/emu.cpp $(CFLAGS) $(STEEMFLAGS)

asm:
	nasm -fcoff -dUNDERSCORES -o./asm_draw.o -i$(STEEMDIR)/asm/ $(STEEMDIR)/asm/asm_draw.asm
	nasm -fcoff -dUNDERSCORES -o./asm_osd.o -i$(STEEMDIR)/asm/ $(STEEMDIR)/asm/asm_osd_draw.asm

res_c:
	$(CC) -c  -o./resource.o $(STEEMDIR)/rc/resource.cpp

res_asm:
	nasm -fcoff -dUNDERSCORES -o./resource.o $(STEEMDIR)/rc/resource.asm

draw_c:
	$(CC) -o ./draw_c.o -c "$(STEEMDIR)/draw_c/draw_c.cpp" $(CFLAGS) $(STEEMFLAGS) $(RELEASE_CFLAGS)



link_asm: 
	$(CC) -o $(OUTPUT) $(CFLAGS) $(OBJS) $(LIBS)

link_draw_c: 
	$(CC) -o $(OUTPUT) $(CFLAGS) $(DRAWCOBJS) $(LIBS)



mmgr: 
	$(CC) -o $(OUTPUT) $(CFLAGS) $(OBJS) $(LIBS) ./3rdparty/mmgr/mmgr.o

urarlib:
	gcc -c ./3rdparty/urarlib/urarlib.c $(CFLAGS) -ourarlib.o
