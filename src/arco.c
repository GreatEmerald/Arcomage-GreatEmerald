#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL.h>
#include <SDL_config_lib.h>
#include "common.h"
#include "graphics.h"
#include "input.h"
#include "network.h"
#include "sound.h"
#include "cards.h"

SDL_Event event;

int fullscreen=0;
int soundenabled=1;
int turn=0;
int nextturn=0;
int lastturn=0;
int aiplayer=-1;
int netplayer=-1;
struct Stats Player[2];
int TowerLevels=20;
int WallLevels=10;
int QuarryLevels=1;
int MagicLevels=1;
int DungeonLevels=1;
int BrickQuantities=15;
int GemQuantities=15;
int RecruitQuantities=15;
int TowerVictory=200;//GE: Needs to be synced with cards.c
int ResourceVictory=500;
int NumCursed=1;//GE: Number of cards that can not be discarded.
int CursedIDs[50];//GE: Bump this number up for support of more than 50 cursed cards.
int bOneResourceVictory=0;//GE: Allow victory for getting only one of the resources to required level

int Winner(int a)
{
	if (bOneResourceVictory)
		return (Player[a].t>=TowerVictory)||(Player[!a].t<=0)||
			(Player[a].b>=ResourceVictory)||(Player[a].g>=ResourceVictory)||(Player[a].r>=ResourceVictory);
	else
		return (Player[a].t>=TowerVictory)||(Player[!a].t<=0)||
			((Player[a].b>=ResourceVictory)&&(Player[a].g>=ResourceVictory)&&(Player[a].r>=ResourceVictory));
}

void DrawCards(int turn)
{
	int i,j;
	if (turn==aiplayer || turn==netplayer)
	{
		j=aiplayer;if (j==-1) j=netplayer;
		for (i=0;i<6;i++)
			DrawCard(0x100*j,8+106*i,342);
	}
	else 
		for (i=0;i<6;i++)
			if (Requisite(&Player[turn],i))
				DrawCard(Player[turn].Hand[i],8+106*i,342);
			else
				DrawCardAlpha(Player[turn].Hand[i],8+106*i,342,64);
}

void Boss()
{
	Blit(SCREEN,BUFFER);
	Blit(BOSS,SCREEN);
	UpdateScreen();
	switch (OPERATINGSYSTEM)
	{
		case 1:
			SDL_WM_SetCaption("mc - ~/.xmms",NULL);break;					// Linux
		default:
			SDL_WM_SetCaption("C:\\WINNT\\system32\\cmd.exe",NULL);break;	// Windows
	}
	WaitForKey(0);	
	WaitForKey(SDLK_b);
	Blit(BUFFER,SCREEN);
	UpdateScreen();
	SDL_WM_SetCaption("Arcomage v"ARCOVER,NULL);
	WaitForKey(0);	
}

void Init()
{
	CFG_File CF;

	if (CFG_OK != CFG_OpenFile(CONFIGFILE, &CF))
	{
		printf("Error reading config file! Falling back to defaults.\n");
	}

	if ( CFG_OK == CFG_SelectGroup("Arcomage", 0) )
	{
		CursedIDs[0]=CFG_ReadInt("CursedIDs", 6+(1<<8));
	}

	CFG_CloseFile(0);

	CursedIDs[0]=6+(1<<8); //LodeStone

	atexit(SDL_Quit);
	if (soundenabled) Sound_Init();
	Graphics_Init(fullscreen);
}

void Quit()
{
	Graphics_Quit();
	Sound_Quit();
}

void PlayCard(int c,int discrd)
{
#define STEPS 10
	int sound;
	double d,x,y;
	int bGiveResources=0, i;
	
	for (i=0; i<NumCursed; i++)
	{
		if (Player[turn].Hand[c] == CursedIDs[i] && discrd)
			return;		// Cursed cards like LodeStone can't be discarded
	}
	
	FillRect(8+106*c,342,96,128,0,0,0);
	Blit(SCREEN,BUFFER);
	for (d=0.0;d<=1.0;d+=1.0/STEPS)
	{
		x=(8.0+106.0*c)+d*(272.0-(8.0+106.0*c));
		y=342.0+d*(96.0-342.0);
		Blit(BUFFER,SCREEN);
		DrawCardAlpha(Player[turn].Hand[c],(int)x,(int)y,64);
		if (discrd)
			DrawCard(0x200,(int)x,(int)y);
		UpdateScreen();
		SDL_Delay(20);
	}
	sound=-1;
	if (discrd) nextturn=!turn;
			else nextturn=Turn(&Player[turn],&Player[!turn],Player[turn].Hand[c],turn);
	if (turn != nextturn)
		bGiveResources = 1;
	Blit(GAMEBG,SCREEN);
	DrawCard(Player[turn].Hand[c],272,96);
	if (discrd) DrawCard(0x200,272,96);
	PutCard(Player[turn].Hand[c]);
	Player[turn].Hand[c]=GetCard();
	lastturn=turn;
	turn=nextturn;
		
	DrawStatus(turn,Player);
	if (bGiveResources) //GE: if you didn't put a play again card or you have discarded
	{
		Player[lastturn].b+=Player[lastturn].q; //GE: The enemy gets resources.
		Player[lastturn].g+=Player[lastturn].m;
		Player[lastturn].r+=Player[lastturn].d;
	}	
	DrawCards(turn);
	UpdateScreen();
}

void AIPlay(int *t,int *d)
{
	extern int req[3][35];
	float fuzzy[6],maxf=-10000.0;
	int max=0,i,c;
	for (i=0;i<6;i++)
	{
		c=Player[turn].Hand[i];
		if (Requisite(&Player[turn],i))
			fuzzy[i]=(float) ( (req[c>>8][c&0xFF]+1) );
		else
			fuzzy[i]=(float) ( (req[c>>8][c&0xFF]+1)-100.0 );
	}
	for (i=0;i<5;i++)
		if (fuzzy[i]>maxf)
		{
			maxf=fuzzy[i];
			max=i;		
		}
	*t=max;
	*d=(maxf<0);
}

void InitGame()
{
	int i;

	turn=0;
	if (netplayer==-1)
		InitDeck();
	for (i=0;i<6;i++)
	{
		Player[0].Hand[i]=GetCard();
		Player[1].Hand[i]=GetCard();
	}
	for (i=0;i<2;i++)//GE: Set up conditions here.
	{
		Player[i].b=BrickQuantities;Player[i].g=GemQuantities;Player[i].r=RecruitQuantities;
		Player[i].q=QuarryLevels;Player[i].m=MagicLevels;Player[i].d=DungeonLevels;
		Player[i].t=TowerLevels;Player[i].w=WallLevels;
	}
}

int NetRemPlay(int *t,int *d)
{
	char *s;
	int r;

	r=Recv(&s,0);
	*t=s[0];*d=s[1];
	
	return r;
}

void NetLocPlay(int t,int d,int card)
{
	char s[5];
	s[0]=t;s[1]=d;s[2]=turn;
	s[3]=card >> 8;
	s[4]=card & 0xFF;
	Send(s,5,0);
}

void DoGame()
{
	int i;
	int crd,netcard,discrd;
	int quit=0;
	//GE: Stupid C not supporting string functions :(
	//GE: I correct myself: stupid C not supporting strings at all! Who designed this?!

	InitGame();
	// init screen
	Blit(GAMEBG,SCREEN);
	DrawStatus(turn,Player);
	DrawCards(turn);
	UpdateScreen();
	
	while (!quit && !Winner(0) && !Winner(1))
	{
		if (turn==aiplayer)
		{
			AIPlay(&i,&discrd);
			SDL_Delay(500);
			PlayCard(i,discrd);
		} else 
		if (turn==netplayer)
		{
			if (NetRemPlay(&i,&discrd))
				PlayCard(i,discrd);
			else {
				DialogBox(DLGERROR,"Server dropped connection ...");
				WaitForInput();
				return;
			}
		} else {
			while (!quit)
   			{
				SDL_Delay(CPUWAIT);
				SDL_PollEvent(&event);
				quit=(event.type==SDL_KEYDOWN&&event.key.keysym.sym==SDLK_ESCAPE);
				if (event.type==SDL_KEYDOWN&&event.key.keysym.sym==SDLK_b)
				    Boss();
				if (event.type!=SDL_MOUSEBUTTONDOWN || event.button.button>3) continue;
				discrd=(event.button.button==2)||(event.button.button==3);
				if (InRect(event.button,  8,342,  8+94,468)&&(discrd||Requisite(&Player[turn],0))) {crd=0;break;}
				if (InRect(event.button,114,342,114+94,468)&&(discrd||Requisite(&Player[turn],1))) {crd=1;break;}
				if (InRect(event.button,220,342,220+94,468)&&(discrd||Requisite(&Player[turn],2))) {crd=2;break;}
				if (InRect(event.button,326,342,326+94,468)&&(discrd||Requisite(&Player[turn],3))) {crd=3;break;}
				if (InRect(event.button,432,342,432+94,468)&&(discrd||Requisite(&Player[turn],4))) {crd=4;break;}
				if (InRect(event.button,535,342,532+94,468)&&(discrd||Requisite(&Player[turn],5))) {crd=5;break;}
			}
			netcard = Player[turn].Hand[crd];
			PlayCard(crd,discrd);
			if (netplayer!=-1)
				NetLocPlay(crd,discrd,netcard);
		}
		SDL_Delay(CPUWAIT);
	}
	SDL_Delay(1000);
	if (Winner(0) || Winner(1))
	{
		if (Winner(0)&&Winner(1))
		{
			DialogBox(DLGWINNER,"Draw!");
			Sound_Play(VICTORY);
		}
		else
		{
			if (aiplayer!=-1 || netplayer!=-1)				// 1 local Player
			{
				i=aiplayer;if (i==-1) i=netplayer;i=!i;
				if (Winner(i))
				{
					if (Player[i].t>=TowerVictory)
						DialogBox(DLGWINNER, "You win by a tower building victory!");
					else if (Player[!i].t<=0)
						DialogBox(DLGWINNER, "You win by a tower destruction victory!");
					else DialogBox(DLGWINNER, "You win by a resource victory!");
					Sound_Play(VICTORY);
				}
				else
				{
					if (Player[!i].t>=TowerVictory)
						DialogBox(DLGLOOSER, "You lose by a tower building defeat!");
					else if (Player[i].t<=0)
						DialogBox(DLGLOOSER, "You lose by a tower destruction defeat!");
					else DialogBox(DLGLOOSER, "You lose by a resource defeat!");
					Sound_Play(DEFEAT);
		 		}
			} else {										 // 2 local Players
				DialogBox(DLGWINNER,"Winner is\n%s !",Player[Winner(1)].Name);
				Sound_Play(VICTORY);
			}
		}
		WaitForInput();
	}
}

void DoNetwork()
{
	char *host,*str,*name,remname[17];
	int i,deck[102];

	host=DialogBox(DLGNETWORK,"Connect to server:");

	if (!host) return;

	name = DialogBox(DLGNETWORK,"Your Name");
	if (!name) return;
	name[16]=0;
	
	DialogBox(DLGMSG,"Connecting to '%s'...",host);
	str=ConnectToServer(host);
	if (str)
	{
		DialogBox(DLGERROR,str);
		WaitForInput();
		Network_Quit();
		return;
	}

	Recv(&str,0);
	if (strncmp(str,"ARCOMAGE v",10))
	{
		DialogBox(DLGERROR,"Unknown Arcomage version\nor\nserver is not Arcomage server!");
		WaitForInput();
		return;
	}
	if (strncmp(str,"ARCOMAGE v" ARCOVER,15))
	{
		DialogBox(DLGERROR,"Versions doesn't match!");
		WaitForInput();
	}
	DialogBox(DLGMSG,"Waiting for Game Start ...");
	Send(name,17,0);

	Recv(&str,0);
	for (i=0;i<102;i++)
		deck[i]=str[i]-1;
	SetDeck(deck);
	netplayer=!(str[102]-1);
	Recv(&str,0);
	strcpy(remname,str);

	Player[ netplayer ].Name = remname;
	Player[ !netplayer ].Name = name;
	
	DoGame();
}

void ParseArgs(int argc,char *argv[])
{	
	int i;
	for (i=0;i<argc;i++)
	{
		if (argv[i][0]!='-') continue;
		if (!strcmp(argv[i],"-f")||!strcmp(argv[i],"--fullscreen")) fullscreen=1;
		if (!strcmp(argv[i],"-nosound")||!strcmp(argv[i],"--disable-sound")) soundenabled=0;
	}
}

int main(int argc,char *argv[])
{
	int m;
	srand((unsigned)time(NULL));
	
	ParseArgs(argc,argv);
	
	Init();

	Sound_Play(TITLE);

	while ((m=Menu())!=5)
	{
		switch (m)
		{
		case 1:
			aiplayer=1;
			Player[0].Name = "Player";
			Player[1].Name = "A.I.";
			DoGame();
			break;
		case 2:
			aiplayer=-1;
			Player[0].Name = "Player 1";
			Player[1].Name = "Player 2";
			DoGame();
			break;
		case 3:
			Network_Init();

			aiplayer=-1;
			DoNetwork();

			Network_Quit();
			break;
		case 4:
			DoCredits();
			break;
		}
	}

	Quit();

	return 0;
}
