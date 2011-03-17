#!/bin/sh
# Script for recompiling the game and putting all the required files in their proper places.
# By GreatEmerald, 2011

echo "Attempting to recompile the game, please wait..."
cd ../src
make arcomage
cp arcomage ../bin/linux
make clean
echo "Build is successful. Return to continue."
read