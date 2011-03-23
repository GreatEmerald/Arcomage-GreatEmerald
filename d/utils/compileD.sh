#!/bin/sh
# Script for rebuilding the D library on Linux.
# GreatEmerald, 2011
cd ../src
dmd -m32 -c -lib ArcomageD.d
cp ArcomageD.a ../../lib
echo "Finished rebuilding the D library."
read
