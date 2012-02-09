@echo off

echo These are the BCC (Borland C++ Compiler) build scripts
echo for Steem (user / release build).
echo You need BCC from https://downloads.embarcadero.com/item/24778
echo and NASM from http://www.nasm.us/

echo Make sure the BCCROOT environment variable is set to point to
echo the bcc5.5 directory and the NASMROOT environment variable is set
echo to the nasm directory or nasm.exe is in your PATH environment variable.

set NASMPATH=
if "%NASMROOT%" NEQ "" (
    set NASMPATH=%NASMROOT%\
)

set BCCPATH=
IF "%BCCROOT%" EQU "" (
    echo -----------------------------------
    echo -----------------------------------
    echo ERROR - BCCROOT variable is not set
    echo -----------------------------------
    echo -----------------------------------
    goto END_BUILD
)
set BCCPATH=%BCCROOT%\Bin\

set OUT=_UserRelease
set ROOT=..\..

rmdir /S /Q "%OUT%"
mkdir "%OUT%"

rmdir /S /Q obj
mkdir obj

"%NASMPATH%nasm" -o obj\asm_draw.obj -fobj -dWIN32 -w+macro-params -w+macro-selfref -w+orphan-labels -i%ROOT%\steem\asm\ %ROOT%\steem\asm\asm_draw.asm
"%NASMPATH%nasm" -o obj\asm_osd_draw.obj -fobj -dWIN32 -w+macro-params -w+macro-selfref -w+orphan-labels -i%ROOT%\steem\asm\ %ROOT%\steem\asm\asm_osd_draw.asm
"%NASMPATH%nasm" -o obj\asm_portio.obj -fobj -dWIN32 %ROOT%\include\asm\asm_portio.asm

"%BCCPATH%make.exe" 3rdparty
"%BCCPATH%make.exe" -B -D_DONT_ALLOW_DEBUG -D_BCC_BUILD

del "%OUT%\*.tds"

mkdir "%OUT%\patches"
xcopy "%ROOT%\steem\patches"              "%OUT%\patches\" /y /e

set DOC_DIR=%ROOT%\steem\doc

copy "%DOC_DIR%\readme.txt"           "%OUT%\readme.txt"
copy "%ROOT%\steem\lib\unzipd32.dll"      "%OUT%\unzipd32.dll"
copy "%ROOT%\steem\steem.new"             "%OUT%\steem.new"
copy "%DOC_DIR%\DISK_IMG.PRG"         "%OUT%\DISK_IMG.PRG"
copy "%DOC_DIR%\disk image howto.txt" "%OUT%\disk image howto.txt"
copy "%DOC_DIR%\CART_IMG.PRG"         "%OUT%\CART_IMG.PRG"
copy "%DOC_DIR%\cart image howto.txt" "%OUT%\cart image howto.txt"
copy "%DOC_DIR%\faq.txt"              "%OUT%\faq.txt"

copy "..\steemupdate\steemupdate.exe"    "%OUT%\steemupdate.exe"
