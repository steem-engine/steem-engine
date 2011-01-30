rem BCC build for Steem
rem make sure BCCROOT and PATH variables are set for your install of BCC

rmdir /S /Q "_DebugRelease"
mkdir "_DebugRelease"

rmdir /S /Q obj
mkdir obj

nasm -o obj\asm_draw.obj -fobj -dWIN32 -w+macro-params -w+macro-selfref -w+orphan-labels -i..\steem\asm ..\steem\asm\asm_draw.asm
nasm -o obj\asm_osd.obj -fobj -dWIN32 -w+macro-params -w+macro-selfref -w+orphan-labels -i..\steem\asm ..\steem\asm\asm_osd_draw.asm
nasm -o obj\asm_portio.obj -fobj -dWIN32 ..\include\asm\asm_portio.asm


make.exe -B -D_FORCE_DEBUG_BUILD -D_BCC_BUILD
del _DebugRelease\*.tds

mkdir "_DebugRelease\patches"
xcopy "..\steem\patches"             "_DebugRelease\patches\" /y /e

set DOC_DIR=..\steem\doc

copy "%DOC_DIR%\readme.txt"           "_DebugRelease\readme.txt"
copy "..\steem\lib\unzipd32.dll"         "_DebugRelease\unzipd32.dll"
copy "..\steem\steem.new"            "_DebugRelease\steem.new"
copy "%DOC_DIR%\DISK_IMG.PRG"         "_DebugRelease\DISK_IMG.PRG"
copy "%DOC_DIR%\disk image howto.txt" "_DebugRelease\disk image howto.txt"
copy "%DOC_DIR%\CART_IMG.PRG"         "_DebugRelease\CART_IMG.PRG"
copy "%DOC_DIR%\cart image howto.txt" "_DebugRelease\cart image howto.txt"
copy "%DOC_DIR%\faq.txt"              "_DebugRelease\faq.txt"

copy "steemupdate\steemupdate.exe" "_DebugRelease\steemupdate.exe"

copy "%DOC_DIR%\debug.txt"            "_DebugRelease\debug.txt"
copy "%DOC_DIR%\logsection.dat"       "_DebugRelease\logsection.dat"
