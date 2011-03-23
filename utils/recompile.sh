#!/bin/sh
# Script for recompiling the game and putting all the required files in their proper places.
# By GreatEmerald, 2011

echo "Attempting to recompile the game, please wait..."
cd ../src
make arcomage
#cp arcomage ../bin/linux
#make clean
echo "Finished compiling. Return to continue."
read
dmd -m32 minIni.o arco.o common.o cards.o graphics.o input.o network.o sound.o BFont.o ../lib/ArcomageD.a /usr/lib/libSDL.a /usr/lib/libSDL_image.a /usr/lib/libSDL_net.a /usr/lib/libSDL_mixer.a /usr/lib/liblua5.1.a -of../bin/linux/arcomage 
echo "Linking complete."
read
