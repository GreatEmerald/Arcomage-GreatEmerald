#include <SDL/SDL_mixer.h>
#include "common.h"
#include "sound.h"

Mix_Chunk *SfxData[SFX_CNT];
int SoundEnabled=0;

void LoadSound(char *filename,Mix_Chunk **snd)
{
	*snd=Mix_LoadWAV(filename);
	if (!*snd) FatalError("File '%s' is missing or corrupt.",filename);
}

void Sound_Init()
{
	// initialize audio
	if (Mix_OpenAudio(22050,AUDIO_S16LSB,2,4096)==0)
	{
		SoundEnabled=1;
		LoadSound(DATADIR "damage.wav",&SfxData[DAMAGE]);
		LoadSound(DATADIR "deal.wav",&SfxData[DEAL]);
		LoadSound(DATADIR "defeat.wav",&SfxData[DEFEAT]);
		LoadSound(DATADIR "resb_down.wav",&SfxData[RESB_DOWN]);
		LoadSound(DATADIR "resb_up.wav",&SfxData[RESB_UP]);
		LoadSound(DATADIR "ress_down.wav",&SfxData[RESS_DOWN]);
		LoadSound(DATADIR "ress_up.wav",&SfxData[RESS_UP]);
		LoadSound(DATADIR "shuffle.wav",&SfxData[SHUFFLE]);
		LoadSound(DATADIR "title.wav",&SfxData[TITLE]);
		LoadSound(DATADIR "tower_up.wav",&SfxData[TOWER_UP]);
		LoadSound(DATADIR "victory.wav",&SfxData[VICTORY]);
		LoadSound(DATADIR "wall_up.wav",&SfxData[WALL_UP]);
	}
}

void Sound_Quit()
{
	int i;
	if (SoundEnabled)
	{
		Mix_HaltChannel(-1);
		for (i=0;i<SFX_CNT;i++)
			Mix_FreeChunk(SfxData[i]);
		Mix_CloseAudio();
	}
}

int Sound_Play(int which)
{
	if (!SoundEnabled) return -1;
	if (which<0) return which;
	return Mix_PlayChannel(-1,SfxData[which],0);
}
