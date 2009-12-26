#include <SDL.h>
#include "common.h"

SDL_Event event;

void WaitForKey(int sym)
{
	if (!sym)
	{
		do
			{
				SDL_PollEvent(&event);
				SDL_Delay(CPUWAIT);
			} while (event.type!=SDL_KEYUP);
	} else {
		do
		{
			SDL_PollEvent(&event);
			SDL_Delay(CPUWAIT);
		} while (event.type!=SDL_KEYDOWN || event.key.keysym.sym!=sym);
	}
}

void WaitForInput()
{
	/*
	do
	{
		SDL_PollEvent(&event);
		SDL_Delay(CPUWAIT);
	} while (!((event.type==SDL_KEYDOWN)||((event.type==SDL_MOUSEBUTTONDOWN)&&(event.button.button==SDL_BUTTON_LEFT))));

	if (event.type!=SDL_KEYDOWN)
	{
		do
		{
			SDL_PollEvent(&event);
			SDL_Delay(CPUWAIT);
		} while (event.type!=SDL_MOUSEBUTTONUP||event.button.button!=SDL_BUTTON_LEFT);
	}
	*/

	SDL_PumpEvents();
	do {
		SDL_PollEvent(&event);
		SDL_Delay(CPUWAIT);
	} while (!((event.type==SDL_KEYDOWN)||((event.type==SDL_MOUSEBUTTONDOWN)&&(event.button.button==SDL_BUTTON_LEFT))));
	SDL_PumpEvents();
}
