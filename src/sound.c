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
		LoadSound("data/damage.wav",&SfxData[DAMAGE]);
		LoadSound("data/deal.wav",&SfxData[DEAL]);
		LoadSound("data/defeat.wav",&SfxData[DEFEAT]);
		LoadSound("data/resb_down.wav",&SfxData[RESB_DOWN]);
		LoadSound("data/resb_up.wav",&SfxData[RESB_UP]);
		LoadSound("data/ress_down.wav",&SfxData[RESS_DOWN]);
		LoadSound("data/ress_up.wav",&SfxData[RESS_UP]);
		LoadSound("data/shuffle.wav",&SfxData[SHUFFLE]);
		LoadSound("data/title.wav",&SfxData[TITLE]);
		LoadSound("data/tower_up.wav",&SfxData[TOWER_UP]);
		LoadSound("data/victory.wav",&SfxData[VICTORY]);
		LoadSound("data/wall_up.wav",&SfxData[WALL_UP]);
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
