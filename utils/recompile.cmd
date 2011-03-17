@ECHO OFF
cd ..\src
dmc minIni.c arco.c common.c cards.c graphics.c input.c network.c sound.c BFont.c ..\lib\SDL.lib ..\lib\SDL_image.lib ..\lib\SDL_net.lib ..\lib\SDL_mixer.lib ..\lib\lua51.lib -I..\include -o..\bin\windows\Arcomage.exe
pause
del *.obj
del *.map
echo Rebuild is complete. The new binaries are stored in /bin/windows.
pause