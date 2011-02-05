rem These are the BCC (Borland C++ Compiler) build scripts
rem for Steem (debug / developers' build).
rem You need BCC from https://downloads.embarcadero.com/item/24778
rem and NASM from http://www.nasm.us/

rem make sure BCCROOT environment variable is set to point to
rem the bcc5.5 directory and the PATH variable points to
rem bcc5.5\bin\make.exe and nasm.exe

set OUT=_DebugRelease

rmdir /S /Q %OUT%
mkdir "%OUT%"

rmdir /S /Q obj
mkdir obj

nasm -o obj\asm_draw.obj -fobj -dWIN32 -w+macro-params -w+macro-selfref -w+orphan-labels -i..\steem\asm\ ..\steem\asm\asm_draw.asm
nasm -o obj\asm_osd_draw.obj -fobj -dWIN32 -w+macro-params -w+macro-selfref -w+orphan-labels -i..\steem\asm\ ..\steem\asm\asm_osd_draw.asm
nasm -o obj\asm_portio.obj -fobj -dWIN32 ..\include\asm\asm_portio.asm

make.exe -B -D_FORCE_DEBUG_BUILD -D_BCC_BUILD

del %OUT%\*.tds

mkdir "%OUT%\patches"
xcopy "..\steem\patches"              "%OUT%\patches\" /y /e

set DOC_DIR=..\steem\doc

copy "%DOC_DIR%\readme.txt"           "%OUT%\readme.txt"
copy "..\steem\lib\unzipd32.dll"      "%OUT%\unzipd32.dll"
copy "..\steem\steem.new"             "%OUT%\steem.new"
copy "%DOC_DIR%\DISK_IMG.PRG"         "%OUT%\DISK_IMG.PRG"
copy "%DOC_DIR%\disk image howto.txt" "%OUT%\disk image howto.txt"
copy "%DOC_DIR%\CART_IMG.PRG"         "%OUT%\CART_IMG.PRG"
copy "%DOC_DIR%\cart image howto.txt" "%OUT%\cart image howto.txt"
copy "%DOC_DIR%\faq.txt"              "%OUT%\faq.txt"

copy "steemupdate\steemupdate.exe"    "%OUT%\steemupdate.exe"

copy "%DOC_DIR%\debug.txt"            "%OUT%\debug.txt"
copy "%DOC_DIR%\logsection.dat"       "%OUT%\logsection.dat"
