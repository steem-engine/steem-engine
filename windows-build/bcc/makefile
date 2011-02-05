#BCCROOT = C:\Code\Compile\BCC
#BCBROOT = C:\Code\Compile\BCB
STEEMROOT = ..\..\steem
ASMINCOBJDIR = obj
INCDIR = ..\..\include
ASMOBJDIR = obj
3RDPARTYROOT = ..\..\3rdparty

3RDPARTYOBJS = $(3RDPARTYROOT)\urarlib\urarlib.obj

INCLUDEPATHS = $(STEEMROOT)\code;$(BCCROOT)\Include;$(INCDIR);$(3RDPARTYROOT)

ALLOBJS = $(ASMOBJDIR)\asm_draw.obj obj\emu.obj c0w32.obj obj\Steem.obj $(ASMOBJS) obj\helper.obj
ALLOBJS = $(ALLOBJS) $(ASMOBJDIR)\asm_osd_draw.obj $(ASMINCOBJDIR)\asm_portio.obj

!ifndef ONEGAME

ALLOBJS = $(ALLOBJS) $(3RDPARTYOBJS)
NAME = Steem
RC_PATH = $(STEEMROOT)\rc\resource.rc

!else

NAME = $(ONEGAME_NAME)
RC_PATH = $(STEEMROOT)\rc\onegame\$(ONEGAME_NAME).rc

!endif

###################################################################
CPP        = $(BCCROOT)\Bin\bcc32
CPP32      = $(BCCROOT)\Bin\cpp32
LIBRARIAN  = $(BCCROOT)\Bin\tlib /P128
LINKER     = $(BCCROOT)\Bin\ilink32
RC         = $(BCCROOT)\Bin\brcc32

#    Set the linker options
LINKOPTS = -L$(BCCROOT)\Lib;$(BCCROOT)\Lib\PSDK -aa -Tpe -t -x -w -v- -C -Gn

#    Set the compiler options
STEEMFLAGS = -D_BCC_BUILD -DWIN32

CCOPTS = -D_NO_VCL -DWINVER=0x0400 -D_WIN32_WINNT=0x0300
CCOPTS = -w  -O2 -OS -Oi -Ov -O -Oc -K-                  $(CCOPTS)
CCOPTS = -jb -j10 -tWM  -r -a4  -6  -vi -d -ff -fp       $(CCOPTS)
CCOPTS = -v- -x -xf -xp- -xd -xs- -H- -y- -X             $(CCOPTS)


!ifndef ONEGAME

OUTPUTDIR = obj

!ifdef _DONT_ALLOW_DEBUG
CCOPTS = -D_NO_DEBUG_BUILD                                $(CCOPTS)
OUTPUTDIR = _UserRelease
!endif

!ifdef _FORCE_DEBUG_BUILD
CCOPTS = -D_DEBUG_BUILD                                   $(CCOPTS)
OUTPUTDIR = _DebugRelease
!endif

!else

ONEGAME_IDX=0
!if $(ONEGAME_NAME)==nm1
ONEGAME_IDX=1
!elif $(ONEGAME_NAME)==nm2
ONEGAME_IDX=2
!elif $(ONEGAME_NAME)==aw1
ONEGAME_IDX=3
!elif $(ONEGAME_NAME)==aw2
ONEGAME_IDX=4
!elif $(ONEGAME_NAME)==sat1
ONEGAME_IDX=5
!elif $(ONEGAME_NAME)==sat2
ONEGAME_IDX=6
!endif

CCOPTS = -DOG_NM1_IDX=1;OG_NM2_IDX=2;OG_AW1_IDX=3        $(CCOPTS)
CCOPTS = -DOG_AW2_IDX=4;OG_SAT1_IDX=5;OG_SAT2_IDX=6      $(CCOPTS)
CCOPTS = -DONEGAME;ONEGAME_NAME="\"$(ONEGAME_NAME)\""    $(CCOPTS)
CCOPTS = -DONEGAME_IDX=$(ONEGAME_IDX)                    $(CCOPTS)
OUTPUTDIR = $(ONEGAME_PATH)

!endif

!ifdef _BCC_BUILD
CCOPTS = -D_BCC_BUILD                                     $(CCOPTS)
!endif

CPPFLAGS= -I$(INCLUDEPATHS) $(CCOPTS)

######################################################################################
######################################################################################
######################################################################################

ALL: Steem.res Steem.obj helper.obj emu.obj $(NAME).exe

Steem.res:
           $(RC) $(RC_PATH) -foobj\Steem.res

Steem.obj:
           $(CPP) $(CPPFLAGS) $(STEEMFLAGS) -c -oobj\Steem.obj $(STEEMROOT)\Steem.cpp

helper.obj:
           $(CPP) $(CPPFLAGS) $(STEEMFLAGS) -c -oobj\helper.obj $(STEEMROOT)\helper.cpp

emu.obj:
           $(CPP) $(CPPFLAGS) $(STEEMFLAGS) -c -oobj\emu.obj $(STEEMROOT)\emu.cpp

$(NAME).exe:
           $(LINKER) $(LINKOPTS) $(ALLOBJS), "$(OUTPUTDIR)\$(NAME).exe", ,import32.lib cw32mt.lib, ,obj\Steem.res
