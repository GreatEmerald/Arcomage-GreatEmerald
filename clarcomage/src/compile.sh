#/bin/sh
rdmd -ILuaD -Isrc -L-llua --build-only -ofclarcomage clarcomage.d ../../libarcomage/include/libarcomage.a 
#dmd -ILuaD -Isrc -L-llua -ofclarcomage clarcomage.d ../../libarcomage/include/libarcomage.a LuaD/luad/*.d LuaD/luad/conversions/*.d LuaD/luad/c/*.d