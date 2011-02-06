#####################################
#      Makefile for Steem           #
#####################################

MINGWDIR=C:\Code\Compile\MinGW
STEEMDIR=C:\Code\Projects\Steem
3RDDIR=C:\Code\3rdparty
INCDIR=C:\Code\include
RCDIR=$(STEEMDIR)\rc
ASMDIR=$(STEEMDIR)\asm

# Compiler
CC=g++

# Parameters given to the compiler
STEEMFLAGS=-D_MINGW_BUILD -DWIN32

CFLAGS=-mwindows -mcpu=pentium -march=i386
CFLAGS=-I$(MINGWDIR)\INCLUDE\ -L$(MINGWDIR)\LIB\ -B$(MINGWDIR)\BIN\  $(CFLAGS)
CFLAGS=-I$(INCDIR) -I$(3RDDIR) -I$(STEEMDIR) -I$(STEEMDIR)\code      $(CFLAGS)

# RELEASE_CFLAGS=-O 
# RELEASE_CFLAGS=-Wall $(RELEASE_CFLAGS)

!if $(DEBUG)==1
STEEMFLAGS=$(STEEMFLAGS) -D_DEBUG_BUILD
!endif

!if $(USER)==1
STEEMFLAGS=$(STEEMFLAGS) -D_NO_DEBUG_BUILD
!endif

!if $(RELEASE)==1
STEEMFLAGS=$(STEEMFLAGS) $(RELEASE_CFLAGS)
!endif

!if $(GDB)==1
STEEMFLAGS=$(STEEMFLAGS) -ggdb
!endif

# Output filename
OUTPUT="Steem.exe"

# Libraries
OBJS=-lwinmm -luuid -lcomctl32 -lole32 

OBJS=asm_draw.o emu.o asm_osd.o asm_int.o urarlib.o        $(OBJS)
OBJS=Steem.o helper.o resource.o portio.o                  $(OBJS)

# Targets

all: 
	$(MAKE) clean
	$(MAKE) rc
	$(MAKE) -DDEBUG=$(DEBUG) -DGDB=$(GDB) -DRELEASE=$(RELEASE) -DUSER=$(USER) helper
	$(MAKE) -DDEBUG=$(DEBUG) -DGDB=$(GDB) -DRELEASE=$(RELEASE) -DUSER=$(USER) emu
	$(MAKE) -DDEBUG=$(DEBUG) -DGDB=$(GDB) -DRELEASE=$(RELEASE) -DUSER=$(USER) main 
	$(MAKE) link

release:
	$(MAKE) clean
	make_port
	make_draw
	make_osd
	make_int
	$(MAKE) urarlib
	$(MAKE) rc
	$(MAKE) -DDEBUG=$(DEBUG) -DGDB=$(GDB) -DRELEASE=$(RELEASE) -DUSER=$(USER) helper
	$(MAKE) -DDEBUG=$(DEBUG) -DGDB=$(GDB) -DRELEASE=$(RELEASE) -DUSER=$(USER) main
	$(MAKE) -DDEBUG=$(DEBUG) -DGDB=$(GDB) -DRELEASE=$(RELEASE) -DUSER=$(USER) emu
	$(MAKE) link
	strip -s $(OUTPUT)

user: 
	$(MAKE) -DUSER=1 all

debug: 
	$(MAKE) -DDEBUG=1 all

all_gdb:
	$(MAKE) -DGDB=1 all

user_release:
	$(MAKE) -DUSER=1 -DRELEASE=1 release

debug_release: 
	$(MAKE) -DDEBUG=1 -DRELEASE=1 release


helper:
	$(CC) -c $(STEEMDIR)\helper.cpp $(CFLAGS) $(STEEMFLAGS)

main:
	$(CC) -c $(STEEMDIR)\Steem.cpp $(CFLAGS) $(STEEMFLAGS)

emu:
	$(CC) -c $(STEEMDIR)\emu.cpp $(CFLAGS) $(STEEMFLAGS)


link: 
	$(CC) -o $(OUTPUT) $(CFLAGS) $(OBJS) 


rc:
	windres --include-dir=$(MINGWDIR)\INCLUDE\ --include-dir=$(STEEMDIR) --use-temp-file -O coff -i "$(RCDIR)\resource.rc" -o "resource.o"

urarlib:
	gcc -c "$(3RDDIR)\urarlib\urarlib.c" $(CFLAGS) -ourarlib.o $(RELEASE_CFLAGS)

draw_c:
	$(CC) -o draw_c.o -c "$(STEEMDIR)\code\draw_c\draw_c.cpp" $(CFLAGS) $(STEEMFLAGS) $(RELEASE_CFLAGS)

clean:
	del Steem.o
	del emu.o
	del helper.o
	del resource.o
