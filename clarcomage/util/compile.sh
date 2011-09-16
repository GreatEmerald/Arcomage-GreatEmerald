#!/bin/sh
#rdmd -ILuaD -Isrc -L-llua --build-only -ofclarcomage clarcomage.d ../../libarcomage/include/libarcomage.a 
cd ../src
dmd -ILuaD -Isrc -L-llua -ofclarcomage clarcomage.d ../../libarcomage/src/*.d LuaD/luad/*.d LuaD/luad/conversions/*.d LuaD/luad/c/*.d
mv clarcomage ../bin/pos64
rm ../bin/pos64/*.o
