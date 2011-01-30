rem BCC build for Steem
rem make sure BCCROOT and PATH variables are set for your install of BCC


rmdir /S /Q "_UserRelease"
mkdir "_UserRelease"

make -B -D_DONT_ALLOW_DEBUG -D_BCC_BUILD
del Steem.obj
del _UserRelease\*.tds

mkdir "_UserRelease\patches"
xcopy "..\steem\patches" "_UserRelease\patches\" /y /e

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
