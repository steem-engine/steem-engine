rem These are the BCC (Borland C++ Compiler) build scripts
rem for Steem (user / release build).
rem You need BCC from https://downloads.embarcadero.com/item/24778
rem and NASM from http://www.nasm.us/

rem make sure BCCROOT environment variable is set to point to
rem the bcc5.5 directory and the PATH variable points to
rem bcc5.5\bin\make.exe and nasm.exe

set OUT=_UserRelease
set ROOT=..\..

rmdir /S /Q %OUT%
mkdir %OUT%

rmdir /S /Q obj
mkdir obj

nasm -o obj\asm_draw.obj -fobj -dWIN32 -w+macro-params -w+macro-selfref -w+orphan-labels -i%ROOT%\steem\asm\ %ROOT%\steem\asm\asm_draw.asm
nasm -o obj\asm_osd_draw.obj -fobj -dWIN32 -w+macro-params -w+macro-selfref -w+orphan-labels -i%ROOT%\steem\asm\ %ROOT%\steem\asm\asm_osd_draw.asm
nasm -o obj\asm_portio.obj -fobj -dWIN32 %ROOT%\include\asm\asm_portio.asm

make -B -D_DONT_ALLOW_DEBUG -D_BCC_BUILD

del %OUT%\*.tds

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
