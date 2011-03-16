#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL.h>
//#include <SDL_Config.lib>
//#include <libSDL_Config.a>
//#include <SDL_config_lib.h>
//#include "gmCall.h"
//#include <lua.h>
#include "common.h"
#include "graphics.h"
#include "input.h"
#include "network.h"
#include "minIni.h"
#include "sound.h"
#include "cards.h"

SDL_Event event;

int /*config*/ fullscreen=0;
int /*config*/ soundenabled=1;
int turn=0;
int nextturn=0;
int lastturn=0;
int bSpecialTurn=0; //GE: used for determining whether or not this is a discarding turn.
int bRefreshNeeded=0; //GE: True if we need to refresh the screen. Used in the input loop.
int aiplayer=-1;
int netplayer=-1;
struct Stats Player[2];
int /*config*/ TowerLevels=20;
int /*config*/ WallLevels=10;
int /*config*/ QuarryLevels=1;
int /*config*/ MagicLevels=1;
int /*config*/ DungeonLevels=1;
int /*config*/ BrickQuantities=15;
int /*config*/ GemQuantities=15;
int /*config*/ RecruitQuantities=15;
int /*config*/ TowerVictory=200;
int /*config*/ ResourceVictory=500;
int NumCursed=1;//GE: Number of cards that can not be discarded.
int CursedIDs[1];//GE: Bump this number up for support of more than 50 cursed cards.
int /*config*/ bOneResourceVictory=0;//GE: Allow victory for getting only one of the resources to required level
int /*config*/ CardTranslucency=64;//GE: Controls the level of alpha channel on the inactive cards.

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
		//GE: This is info on where to put in on the screen.
			DrawCard(0x100*j,8+106*i,342);
	}
	else 
		for (i=0;i<6;i++)
			if (Requisite(&Player[turn],i))
				DrawCard(Player[turn].Hand[i],8+106*i,342);
			else
				DrawCardAlpha(Player[turn].Hand[i],8+106*i,342,CardTranslucency);
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

void ReadConfig()
{
    fullscreen=ini_getl("Engine", "Fullscreen", 0, CONFIGFILE);
    soundenabled=ini_getl("Engine", "SoundEnabled", 1, CONFIGFILE);
    
    TowerLevels=ini_getl("StartingConditions", "TowerLevels", 20, CONFIGFILE);
    WallLevels=ini_getl("StartingConditions", "WallLevels", 10, CONFIGFILE);
    QuarryLevels=ini_getl("StartingConditions", "QuarryLevels", 1, CONFIGFILE);
    MagicLevels=ini_getl("StartingConditions", "MagicLevels", 1, CONFIGFILE);
    DungeonLevels=ini_getl("StartingConditions", "DungeonLevels", 1, CONFIGFILE);
    BrickQuantities=ini_getl("StartingConditions", "BrickQuantities", 15, CONFIGFILE);
    GemQuantities=ini_getl("StartingConditions", "GemQuantities", 15, CONFIGFILE);
    RecruitQuantities=ini_getl("StartingConditions", "RecruitQuantities", 15, CONFIGFILE);
    TowerVictory=ini_getl("VictoryConditions", "TowerVictory", 200, CONFIGFILE);
    ResourceVictory=ini_getl("VictoryConditions", "ResourceVictory", 500, CONFIGFILE);
    bOneResourceVictory=ini_getl("VictoryConditions", "bOneResourceVictory", 0, CONFIGFILE);
    
    CardTranslucency=ini_getl("Graphics", "CardTranslucency", 64, CONFIGFILE);
}

void Init()
{
	//lua_State *L;
	//L = luaL_newstate();
	//lua_close(L);
  ReadConfig();

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
	
	//GE: You get resources when you use a card and next up is the enemy's turn.
	
	for (i=0; i<NumCursed; i++)
	{
		if (Player[turn].Hand[c] == CursedIDs[i] && discrd)
			return;		// Cursed cards like LodeStone can't be discarded
	}
	
	if (bSpecialTurn && !discrd) //GE: You're trying to play a card during a discard round. Bad.
	   return;
	
	FillRect(8+106*c,342,96,128,0,0,0);
	Blit(SCREEN,BUFFER);
	for (d=0.0;d<=1.0;d+=1.0/STEPS)
	{
		x=(8.0+106.0*c)+d*(272.0-(8.0+106.0*c));
		y=342.0+d*(96.0-342.0);
		Blit(BUFFER,SCREEN);
		DrawCardAlpha(Player[turn].Hand[c],(int)x,(int)y,CardTranslucency);
		if (discrd)
			DrawCard(0x200,(int)x,(int)y);
		UpdateScreen();
		SDL_Delay(20);
	}
	sound=-1;
	if (discrd)
  { 
	    if (!bSpecialTurn)
          nextturn=!turn;
      else
      {
          nextturn=turn;
          bSpecialTurn=0;
      }
  }
	else 
      nextturn=Turn(&Player[turn],&Player[!turn],Player[turn].Hand[c],turn);
  if (nextturn == -1) //GE: If the card inits a discard turn.
	{
	    bSpecialTurn=1;
	    nextturn=turn;
  }
	if (turn != nextturn)
		bGiveResources = 1;
	Blit(GAMEBG,SCREEN);
	if (discrd)
  {
     DrawCardAlpha(Player[turn].Hand[c],272,96,CardTranslucency);
	   DrawCard(0x200,272,96);
	}
	else
	   DrawCard(Player[turn].Hand[c],272,96);
	PutCard(Player[turn].Hand[c]);
	if (bGiveResources) //GE: if you didn't put a play again card or you have discarded
	{
		Player[nextturn].b+=Player[nextturn].q; //GE: The enemy gets resources.
		Player[nextturn].g+=Player[nextturn].m;
		Player[nextturn].r+=Player[nextturn].d;
	}
    Player[turn].Hand[c]=GetCard();
	lastturn=turn;
	turn=nextturn;
		
	DrawStatus(turn,Player);
		
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
		if (Requisite(&Player[turn],i) && !bSpecialTurn)
			fuzzy[i]=(float) ( (req[c>>8][c&0xFF]+1) ); //GE: AI wants to play this?
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
		
        while(SDL_PollEvent(&event));//GE: Delete all events from the event queue before our turn.
		
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
				if (!SDL_PollEvent(&event))
                    continue;
                SDL_Delay(CPUWAIT);
				quit=(event.type==SDL_KEYUP&&event.key.keysym.sym==SDLK_ESCAPE);
				if (event.type==SDL_KEYDOWN&&event.key.keysym.sym==SDLK_b) //GE: Keeping as "down" since it's urgent ;)
				    Boss();
                if ( event.type == SDL_MOUSEMOTION && InRect(event.motion.x, event.motion.y,   8,342,  8+94,468) )
                {
				    Blit(SCREEN, BUFFER);
                    DrawRectangle(8,342,96,128, 0xFF0000);
				    UpdateScreen();
				    bRefreshNeeded=1;
                }
                else if(bRefreshNeeded)
                {
                    RedrawScreen(turn, Player);
                    //Blit(BUFFER, SCREEN);
                    bRefreshNeeded=0;
                }
                //Blit(SCREEN,BUFFER);
                
                if (event.type!=SDL_MOUSEBUTTONUP || event.button.button>3) continue;
				discrd=(event.button.button==2)||(event.button.button==3);
				if (InRect(event.button.x,event.button.y,  8,342,  8+94,468)&&(discrd||Requisite(&Player[turn],0))) {crd=0;break;}
				if (InRect(event.button.x,event.button.y,114,342,114+94,468)&&(discrd||Requisite(&Player[turn],1))) {crd=1;break;}
				if (InRect(event.button.x,event.button.y,220,342,220+94,468)&&(discrd||Requisite(&Player[turn],2))) {crd=2;break;}
				if (InRect(event.button.x,event.button.y,326,342,326+94,468)&&(discrd||Requisite(&Player[turn],3))) {crd=3;break;}
				if (InRect(event.button.x,event.button.y,432,342,432+94,468)&&(discrd||Requisite(&Player[turn],4))) {crd=4;break;}
				if (InRect(event.button.x,event.button.y,535,342,532+94,468)&&(discrd||Requisite(&Player[turn],5))) {crd=5;break;}
			}
			if (!quit)
            {
                netcard = Player[turn].Hand[crd];
    			PlayCard(crd,discrd);
    			if (netplayer!=-1)
    				NetLocPlay(crd,discrd,netcard);
            }
		}
		SDL_Delay(CPUWAIT);
	}
	if (!quit)
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
						DialogBox(DLGWINNER, "You win by a\ntower building victory!");
					else if (Player[!i].t<=0)
						DialogBox(DLGWINNER, "You win by a tower\ndestruction victory!");
					else DialogBox(DLGWINNER, "You win by a\nresource victory!");
					Sound_Play(VICTORY);
				}
				else
				{
					if (Player[!i].t>=TowerVictory)
						DialogBox(DLGLOOSER, "You lose by a\ntower building defeat!");
					else if (Player[i].t<=0)
						DialogBox(DLGLOOSER, "You lose by a\ntower destruction defeat!");
					else DialogBox(DLGLOOSER, "You lose by a\nresource defeat!");
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
