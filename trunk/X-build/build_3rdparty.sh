cd ../3rdparty/portaudio
sh configure
make

cd ../rtaudio
g++ -c RtAudio.cpp

cd ../urarlib
make

cd ../zlib
sh configure
make

cd contrib/minizip
make
