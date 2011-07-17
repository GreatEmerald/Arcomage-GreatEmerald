#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <SDL.h>
#include <SDL_image.h>
#include "BFont.h"
#include "minIni.h"
#include "resize.h"
#include "cards.h"
#include "common.h"
#include "config.h"
#include "graphics.h"
#include "input.h"
#include "sound.h"

SDL_Event event;
SDL_Surface *GfxData[GFX_CNT];
Picture* PictureHead = NULL;//GE: Linked list.

int resX=640;
int resY=480;
int buttonWidth=160;
int buttonHeight=32;
int buttonDistanceX=240; //resX/2-buttonWidth/2 = 240
int buttonDistanceY=160; //resY/2-buttonHeight/2*5 = 160
int buttonNum=5;
int /*config*/ bUseOriginalMenu=0;
int /*config*/ bUseOriginalCards=0; //GE: Whether to use original graphics for the cards.
char /*config*/ OriginalDataDir[128]; //GE: You can hook up the data dir of the original Arcomage through this and get the stock graphics!
char JoinedString[128+12];

BFont_Info *numssmall=NULL;
BFont_Info *font=NULL;
BFont_Info *bigfont=NULL;

char* JoinStrings(char *FirstString, char *SecondString)
{
    strcpy(JoinedString, FirstString);
    strcat(JoinedString, SecondString);
    return JoinedString;
}

void Graphics_Init(int fullscreen)
{
	//char JoinedString[128+12];
	
  bUseOriginalMenu = ini_getl("Graphics", "bUseOriginalMenu", 0, CONFIGFILE);
  bUseOriginalCards = ini_getl("Graphics", "bUseOriginalCards", 0, CONFIGFILE);
  ini_gets("Graphics", "OriginalDataDir", "data/", OriginalDataDir, sizeof(OriginalDataDir), CONFIGFILE); //GE: 128 symbols max path. 80 is known to be too few for some Unreal installs.
  
  switch (OPERATINGSYSTEM)
	{
		case 1:
			LoadSurface(ARCODATADIR "boss_linux.png",&GfxData[BOSS]);break;
		default:
			LoadSurface(ARCODATADIR "boss_windows.png",&GfxData[BOSS]);break;
	}
	if (!bUseOriginalMenu)
	{
		LoadSurface(ARCODATADIR "menu.png",&GfxData[MENU]);
		LoadSurface(ARCODATADIR "menuitems.png",&GfxData[MENUITEMS]);
		LoadSurface(ARCODATADIR "gamebg.png",&GfxData[GAMEBG]);
	}
	LoadSurface(ARCODATADIR "credits.png",&GfxData[CREDITS]);
	if (!bUseOriginalCards)
      LoadSurface(ARCODATADIR "deck.png",&GfxData[DECK]);
  else
      LoadSurface(JoinStrings(OriginalDataDir, "SPRITES.bmp"),&GfxData[DECK]);
	SDL_SetColorKey(GfxData[DECK],SDL_SRCCOLORKEY,SDL_MapRGB(GfxData[DECK]->format,255,0,255));
	LoadSurface(ARCODATADIR "nums_big.png",&GfxData[NUMSBIG]);
	SDL_SetColorKey(GfxData[NUMSBIG],SDL_SRCCOLORKEY,SDL_MapRGB(GfxData[NUMSBIG]->format,255,0,255));
	LoadSurface(ARCODATADIR "castle.png",&GfxData[CASTLE]);

	LoadSurface(ARCODATADIR "dlgmsg.png",&GfxData[DLGMSG]);
	LoadSurface(ARCODATADIR "dlgerror.png",&GfxData[DLGERROR]);
	LoadSurface(ARCODATADIR "dlgnetwork.png",&GfxData[DLGNETWORK]);
	LoadSurface(ARCODATADIR "dlgwinner.png",&GfxData[DLGWINNER]);
	LoadSurface(ARCODATADIR "dlglooser.png",&GfxData[DLGLOOSER]);

	SDL_SetColorKey(GfxData[CASTLE],SDL_SRCCOLORKEY,SDL_MapRGB(GfxData[CASTLE]->format,255,0,255));

	numssmall=BFont_LoadFont(ARCODATADIR "nums_small.png");
	if (!numssmall)
		FatalError("Data file 'nums_small.png' is missing or corrupt.");
	bigfont=BFont_LoadFont(ARCODATADIR "bigfont.png");
	if (!bigfont)
		FatalError("Data file 'bigfont.png' is missing or corrupt.");
	font=BFont_LoadFont(ARCODATADIR "font.png");
	if (!font)
		FatalError("Data file 'font.png' is missing or corrupt.");
	BFont_SetCurrentFont(font);

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE)<0)
		FatalError("Couldn't initialize SDL");
	SDL_WM_SetCaption("Arcomage v" ARCOVER,NULL);
	GfxData[SCREEN]=SDL_SetVideoMode(resX,resY,0,SDL_SWSURFACE|(fullscreen*SDL_FULLSCREEN)|SDL_ASYNCBLIT|SDL_RESIZABLE); //GE: Enabling async blitting. It's useful for muticore PCs.
	if (!GfxData[SCREEN])
		FatalError("Couldn't set 640x480 video mode");
	GfxData[BUFFER]=SDL_AllocSurface(GfxData[SCREEN]->flags,GfxData[SCREEN]->w,GfxData[SCREEN]->h,GfxData[SCREEN]->format->BitsPerPixel,GfxData[SCREEN]->format->Rmask,GfxData[SCREEN]->format->Gmask,GfxData[SCREEN]->format->Bmask,0);
	if (!GfxData[BUFFER])
		FatalError("Unable to create double buffer!");
	if (bUseOriginalMenu) //GE: HACK
	{
		buttonWidth=125;
		buttonHeight=54;
		buttonDistanceX=257;
		buttonDistanceY=105;
		LoadSurface(ARCODATADIR "menuO.png",&GfxData[MENU]);
		LoadSurface(ARCODATADIR "menuitemsO.png",&GfxData[MENUITEMS]);
		LoadSurface(ARCODATADIR "gamebgO.png",&GfxData[GAMEBG]);
	}
}

//GE: Add to the linked list.
void PrecacheCard(const char* File, size_t Size)
{
    Picture* CurrentPicture;
    Picture* CheckedPicture = PictureHead;
    int bNew=0;
    
    //printf(File);
    if (!PictureHead)//GE: No cards precached, so don't overdo this.
        bNew = 1;
    else //GE: Let's find out if we already have this somewhere.
    {
        while (CheckedPicture)
        {
            //printf("PrecacheCard: Is %s = %s?\n", File, CheckedPicture->File);
            if (!strcmp(File, CheckedPicture->File)) //GE: I thought strcpy() was counterintuitive. This takes the cake!
                return; //GE: It's already been precached. Nothing to do.
            //printf("No it's not.");
            CheckedPicture = CheckedPicture->Next;
        }
    }
    
    CurrentPicture = malloc(sizeof(Picture));
    if (CurrentPicture == NULL) //GE: Allocate the memory to store this picture.
        FatalError("Out of memory to allocate the image linked list! Please use fewer cards."); //GE: Oh noes, out of memory to allocate! ...actually they are but pointers, so I doubt you'd ever run out of it.
    CurrentPicture->File = malloc(Size);
    if (CurrentPicture->File == NULL) //GE: Allocate the memory to store this picture and string.
        FatalError("Out of memory to allocate the image filename! Please use fewer cards."); //GE: Oh noes, out of memory to allocate! This one's quite a bit bigger.
    strcpy(CurrentPicture->File, File);//GE: Set file and surface.
    LoadSurface(CurrentPicture->File, &CurrentPicture->Surface);
    if (bNew)
    {
        CurrentPicture->Next = NULL; //GE: This is added to the end of the list, so next is NULL.
        PictureHead = CurrentPicture;
    }
    else
    {
        CurrentPicture->Next = PictureHead->Next;
        PictureHead->Next = CurrentPicture;
    }
}

void Graphics_Quit()
{
	int i;
	BFont_FreeFont(numssmall);
	BFont_FreeFont(font);
	BFont_FreeFont(bigfont);
	for (i=0;i<GFX_CNT;i++)
		SDL_FreeSurface(GfxData[i]);
}

void Blit(int a,int b)
{
	SDL_BlitSurface(GfxData[a],NULL,GfxData[b],NULL);
}

//GE: This function updates the screen. Nothing is being remade. Fast.
void UpdateScreen()
{
	SDL_UpdateRect(GfxData[SCREEN],0,0,0,0);
}

//GE: This function redraws the screen elements. Slow.
void RedrawScreen(int turn, struct Stats* Player)
{
    SDL_Flip(GfxData[SCREEN]);
    //Blit(BUFFER,SCREEN);
    //Blit(GAMEBG,SCREEN);
    //DrawStatus(turn,Player);
	//DrawCards(turn);
	//UpdateScreen();
}

inline void UpdateScreenRect(int x1,int y1,int x2,int y2)
{
	SDL_UpdateRect(GfxData[SCREEN],x1,y1,x2,y2);
}

void FillRect(int x,int y,int w,int h,Uint8 r,Uint8 g,Uint8 b)
{
	SDL_Rect rect;
	rect.x=x;rect.y=y;rect.w=w;rect.h=h;
	SDL_FillRect(GfxData[SCREEN],&rect,SDL_MapRGB(GfxData[SCREEN]->format,r,g,b));
}

void NewDrawCard(int C, int X, int Y, SDL_Surface* Sourface, Uint8 Alpha)//GE: SOURFACE! :(
{
    SDL_Rect ScreenPosition, DeckPosition;
    //printf("Incoming crash!\n");
    //printf("We got the coordinates: %d:%d; %d:%d\n", D_getPictureCoords(0,C).x, D_getPictureCoords(0,C).y, D_getPictureCoords(0,C).w, D_getPictureCoords(0,C).h);
    //DeckPosition = D_getPictureCoords(0,C);
    printf("On this C platform, ints are sized %d!\n", sizeof(int));
    printf("On this C platform, shorts are sized %d!", sizeof(int16_t));
    //GE: HACK!
    DeckPosition.x = (int16_t) D_getPictureCoordX(0,C);
    DeckPosition.y = (int16_t) D_getPictureCoordY(0,C);
    DeckPosition.w = (uint16_t) D_getPictureCoordW(0,C);
    DeckPosition.h = (uint16_t) D_getPictureCoordH(0,C);
    //printf("Entered DrawNewCard.\n");
    ScreenPosition.x = X;
    ScreenPosition.y = Y;
    ScreenPosition.w = DeckPosition.w;
    ScreenPosition.h = DeckPosition.h;
    
    SDL_SetAlpha(Sourface,SDL_SRCALPHA,Alpha);
    SDL_BlitSurface(Sourface,&DeckPosition,GfxData[SCREEN],&ScreenPosition);
    SDL_SetAlpha(Sourface,SDL_SRCALPHA,255);
    printf("Finished drawing the card.\n");
}

/*
 * GE: Lua reform: We want the modder in Lua to give the picture and coordinates
 * and parse those to load the required surfaces in C. Then when sending to D,
 * we want to send the things already parsed - instead of a string and four
 * doubles, we want a pointer and four ints/struct of coords. When we attempt
 * to draw a card, we pass the card number and pool, from where C code asks D
 * for the pointer and coords, which it then uses to display the thing.     
 */
//GE: c - card ID, x,y - position on the screen.
void DrawCard(int c,int x,int y, Uint8 a)
{
	SDL_Rect recta,rectb;
	int RawX, RawY;
	
	char* File;
	Picture* CurrentPicture = PictureHead;
	
	if (D_getPictureFileSize(0,c) == 1)
	   return;
	
  File = malloc(D_getPictureFileSize(0,c));
  //getchar();
  printf("Drawing picture with size: %d\n", D_getPictureFileSize(0,c));
  //getchar();
  strcpy(File, D_getPictureFile(0,c));
  printf("Allocation complete.\n");
  //getchar();
	
	while (CurrentPicture)
	{
     if (!strcmp(CurrentPicture->File, File))
	   {
	     printf("Attempting to draw card.\n");  
	     NewDrawCard(c,x,y,CurrentPicture->Surface, a);
         free(File);
         printf("Freeing complete.\n");
         return;
     }
     CurrentPicture = CurrentPicture->Next;
  }
	getchar();
	if (!bUseOriginalCards)
	{
	   recta.x=x;recta.y=y;recta.w=96;recta.h=128;
     rectb.x=(c&0xFF)*96;rectb.y=(c>>8)*128;rectb.w=96;rectb.h=128;
	}
	else
	{
    	/*
    	 * GE: Transform.
    	 * Got 1*96, 1*128
    	 * Got 11*96, 1*128 -> 1*96, 2*128
    	 * Got 1*96, 2*128 -> 1*96, 5*128
    	 * Three special cards: 0,0 is red, 0,1 is blue, 0,3 is Discard.
    	 * Original doesn't have blue, and Discard is not in a shape of a card.             
    	 */
    	
      RawX=(c&0xFF);
    	RawY=(c>>8);
    	
    	//GE: Start special case handling --------------------------------------------
    	if (RawX == 0)
    	{
          if (RawY == 0 || RawY == 1) //GE: back of the card
          {
              rectb.x=192;rectb.y=0;rectb.w=96;rectb.h=128;
              recta.x=x;recta.y=y;
          }
          else //GE: Discard
          {
              rectb.x=843;rectb.y=200;rectb.w=73;rectb.h=16;
              SDL_SetColorKey(GfxData[DECK], SDL_SRCCOLORKEY, 0x000000FF);
              recta.x=x+11;recta.y=y+56;
          }
      }
      else //GE: End special case handling -----------------------------------------
      {
        	RawX-=1;
          RawY=RawX/10+(RawY*4);
          RawX=RawX%10;
        	
        	recta.x=x;recta.y=y;recta.w=96;recta.h=128;
          rectb.x=RawX*96;rectb.y=RawY*128+220;rectb.w=96;rectb.h=128;
      }
  }
	SDL_BlitSurface(GfxData[DECK],&rectb,GfxData[SCREEN],&recta);
}

void DrawFolded(int Team, int X, int Y)
{
    SDL_Rect ScreenPosition, DeckPosition;
    
    ScreenPosition.x = X; ScreenPosition.y = Y;
    ScreenPosition.w = 96; ScreenPosition.h = 128;
    
    if (!bUseOriginalCards) //GE: New
    {
        DeckPosition.x = 0;
        if (Team) //GE: Blue
            DeckPosition.y = 128;
        else //GE: Red
            DeckPosition.y = 0;
    }
    else //GE: Original
        DeckPosition.x = 192; DeckPosition.y = 0;
    DeckPosition.w = ScreenPosition.w; DeckPosition.h = ScreenPosition.h;
    
    SDL_BlitSurface(GfxData[DECK],&DeckPosition,GfxData[SCREEN],&ScreenPosition);
}

void DrawDiscard(int X, int Y)
{
    SDL_Rect ScreenPosition, DeckPosition;
    
    ScreenPosition.x = X; ScreenPosition.y = Y;
    ScreenPosition.w = 96; ScreenPosition.h = 128;
    
    DeckPosition.x = 0; DeckPosition.y = 256;
    DeckPosition.w = ScreenPosition.w; DeckPosition.h = ScreenPosition.h;
    
    SDL_BlitSurface(GfxData[DECK],&DeckPosition,GfxData[SCREEN],&ScreenPosition);
}

/**
 * Draws the cards on the screen.
 *
 * This function is only used for drawing the cards at the bottom of the screen.
 * Works for all types of players.
 *
 * Authors: GreatEmerald, STiCK.
 *
 * \param turn Player number.
 */
void DrawCards(int turn)
{
    int i,j;

    if (turn==aiplayer || turn==netplayer)
    {
        j=aiplayer;if (j==-1) j=netplayer;
        for (i=0;i<6;i++)
        //GE: This is info on where to put in on the screen.
            DrawFolded(j,8+106*i,342);
    }
    else
        for (i=0;i<6;i++)
            if (Requisite(&Player[turn],i))
                DrawCard(Player[turn].Hand[i],8+106*i,342,255);
            else
                DrawCard(Player[turn].Hand[i],8+106*i,342,CardTranslucency);
}

/**
 * Draws the 'boss' screen.
 *
 * Easter egg, can be used to protect players from angry bosses. Activated by
 * pressing the B button.
 *
 * Authors: STiCK.
 */
void Boss()
{
    Blit(SCREEN,BUFFER);
    Blit(BOSS,SCREEN);
    UpdateScreen();
    switch (OPERATINGSYSTEM)
    {
        case 1:
            SDL_WM_SetCaption("mc - ~/.xmms",NULL);break;                   // Linux
        default:
            SDL_WM_SetCaption("C:\\WINNT\\system32\\cmd.exe",NULL);break;   // Windows
    }
    WaitForKey(0);
    WaitForKey(SDLK_b);
    Blit(BUFFER,SCREEN);
    UpdateScreen();
    SDL_WM_SetCaption("Arcomage v"ARCOVER,NULL);
    WaitForKey(0);
}

void DrawSmallNumber(int Resource, int X, int Y, int Offset)
{
    char str[4];
    int i;
    sprintf(str, "%d", Resource);
    for (i=0; str[i]!=0; i++)
        str[i]=str[i]-'0'+Offset;
    BFont_PutStringFont(GfxData[SCREEN],numssmall,X,Y,str);
}

void DrawBigNumber(int Resource, int X, int Y)
{
    SDL_Rect recta,rectb;
    int d1, d2;
    d1 = Resource/10;
    d2 = Resource%10;
    
    recta.w=22;recta.h=17;
    recta.x=X;
    recta.y=Y;
    
    rectb.y=0;
    rectb.w=22;rectb.h=17;
    
    if (d1)
    {
       rectb.x=22*d1;
       SDL_BlitSurface(GfxData[NUMSBIG],&rectb,GfxData[SCREEN],&recta);
       recta.x += 22;
    }
    rectb.x=22*d2;
    SDL_BlitSurface(GfxData[NUMSBIG],&rectb,GfxData[SCREEN],&recta);
}

void DrawStatus(int turn,struct Stats *Players)
{
	SDL_Rect recta,rectb;
	int i,j;
	char b[4];
//	print players' names
	recta.w=70;recta.h=16;recta.y=13+4;
	if (turn) recta.x=555+4;
		else recta.x=8+4;
	SDL_FillRect(GfxData[SCREEN],&recta,SDL_MapRGB(GfxData[SCREEN]->format,(Uint8)(0xC0*!turn),0,(Uint8)(0xC0*turn)));
	BFont_PutString(GfxData[SCREEN], 15+32-BFont_TextWidth(Players[0].Name)/2,20,Players[0].Name);
	BFont_PutString(GfxData[SCREEN],562+32-BFont_TextWidth(Players[1].Name)/2,20,Players[1].Name);
	int Offset[3]={33,43,53};
    for (i=0;i<2;i++)
	{
	    for (j=0;j<3;j++)
	    {
            DrawSmallNumber((&Players[i].b)[j], 10+i*547, 115+j*72, Offset[j]);
            DrawBigNumber((&Players[i].q)[j], 13+547*i, 91+j*72);
        }
        
        
        /*int d1, d2;
        d1 = Players[i].q/10;
        d2 = Players[i].q%10;
        
        j=0;
        recta.w=22;recta.h=17;
	    recta.x=13+547*i;recta.y=91+j*72;
        rectb.y=0;
        rectb.w=22;rectb.h=17;
        
        if (d1)
        {
           rectb.x=22*d1;
           SDL_BlitSurface(GfxData[NUMSBIG],&rectb,GfxData[SCREEN],&recta);
           recta.x += 22;
        }
        rectb.x=22*d2;
        SDL_BlitSurface(GfxData[NUMSBIG],&rectb,GfxData[SCREEN],&recta);*/
    	
        //for (j=0;j<3;j++)
		//{
            
            /*recta.w=22;recta.h=17;
			recta.x=13+547*i;recta.y=91+j*72;
			if (j==0) rectb.x=22*Players[i].q;
			if (j==1) rectb.x=22*Players[i].m;
			if (j==2) rectb.x=22*Players[i].d;
            rectb.y=0;
			rectb.w=22;rectb.h=17;
		//	print big (yellow) numbers (quarry,magic,dungeons)
			SDL_BlitSurface(GfxData[NUMSBIG],&rectb,GfxData[SCREEN],&recta);*/
			/*if (j==0) {sprintf(b,"%d",Players[i].b);b[0]-='0'-'!';if (b[1]) b[1]-='0'-'!';if (b[2]) b[2]-='0'-'!';}
			if (j==1) {sprintf(b,"%d",Players[i].g);b[0]-='0'-'+';if (b[1]) b[1]-='0'-'+';if (b[2]) b[2]-='0'-'+';}
			if (j==2) {sprintf(b,"%d",Players[i].r);b[0]-='0'-'5';if (b[1]) b[1]-='0'-'5';if (b[2]) b[2]-='0'-'5';}*/
			//sprintf(b,"%d",Players[i].b);
			
            //b[0]='!'+1; b[1]='!'+5; b[2]='!'+15; b[3]=0;
            //b[0]-='0'-1; b[1]-='0'-1; b[2]-='0'-1; b[3]=0;
		//	print small numbers (bricks,gems,recruits)
			//BFont_PutStringFont(GfxData[SCREEN],numssmall,10+i*547,115+j*72,b);
		//}
    }
//	print tower/wall numbers
	sprintf(b,"%d",Players[0].t);BFont_PutString(GfxData[SCREEN],160-BFont_TextWidth(b)/2,317,b);
	sprintf(b,"%d",Players[0].w);BFont_PutString(GfxData[SCREEN],242-BFont_TextWidth(b)/2,317,b);
	sprintf(b,"%d",Players[1].w);BFont_PutString(GfxData[SCREEN],398-BFont_TextWidth(b)/2,317,b);
	sprintf(b,"%d",Players[1].t);BFont_PutString(GfxData[SCREEN],resY-BFont_TextWidth(b)/2,317,b);
//	draw left tower
	rectb.x=0;rectb.y=0;rectb.w=68;rectb.h=292-(200-Players[0].t);
	recta.x=126;recta.y=16+(200-Players[0].t);recta.w=68;recta.h=292-(200-Players[0].t);
	SDL_BlitSurface(GfxData[CASTLE],&rectb,GfxData[SCREEN],&recta);
//	draw right tower
	rectb.x=68;rectb.y=0;rectb.w=68;rectb.h=292-(200-Players[1].t);
	recta.x=446;recta.y=16+(200-Players[1].t);recta.w=68;recta.h=292-(200-Players[1].t);
	SDL_BlitSurface(GfxData[CASTLE],&rectb,GfxData[SCREEN],&recta);
//	draw left wall
	rectb.x=68*2;rectb.y=0;rectb.w=68;rectb.h=292-(200-Players[0].w);
	recta.x=230;recta.y=16+(200-Players[0].w);recta.w=68;recta.h=292-(200-Players[0].w);
	SDL_BlitSurface(GfxData[CASTLE],&rectb,GfxData[SCREEN],&recta);
//	draw right wall
	rectb.x=68*2;rectb.y=0;rectb.w=68;rectb.h=292-(200-Players[1].w);
	recta.x=386;recta.y=16+(200-Players[1].w);recta.w=68;recta.h=292-(200-Players[1].w);
	SDL_BlitSurface(GfxData[CASTLE],&rectb,GfxData[SCREEN],&recta);
}

void DrawMenuItem(int i,int active)
{
	SDL_Rect recta,rectb;

	//GE: x=distance from left border, y=number*height+distance from top border
	//w=width, h=height; rectb is location in file, x=activated*width, y=number*height
	//w=width, h=height.
	//recta.x=240;recta.y=i*32+160;recta.w=160;recta.h=32;
	//rectb.x=active*160;rectb.y=i*32;rectb.w=160;rectb.h=32;
	recta.x=buttonDistanceX;recta.y=i*buttonHeight+buttonDistanceY;recta.w=buttonWidth;recta.h=buttonHeight;
	rectb.x=active*buttonWidth;rectb.y=i*buttonHeight;rectb.w=buttonWidth;rectb.h=buttonHeight;

	if (active)
		SDL_BlitSurface(GfxData[MENUITEMS],&rectb,GfxData[SCREEN],&recta);
	else 
		SDL_BlitSurface(GfxData[MENUITEMS],&rectb,GfxData[SCREEN],&recta);
}

int Menu()
{
	int i,j,value=0;

	SDL_BlitSurface(GfxData[MENU],NULL,GfxData[SCREEN],NULL);
	for (i=0;i<buttonNum;i++)
		DrawMenuItem(i,0);
	
	UpdateScreen();
	
	Sound_Play(TITLE);

	while (!value)
	{
		SDL_PollEvent(&event);
		switch (event.type)
		{
		case SDL_QUIT:
			value=5;
			break;
		case SDL_MOUSEMOTION:
			j=(event.motion.y-buttonDistanceY)/buttonHeight;
			for (i=0;i<buttonNum;i++)
				DrawMenuItem(i,(i==j)&&(event.motion.x>=buttonDistanceX) && (event.motion.x<=buttonDistanceX+buttonWidth) && (event.motion.y>=buttonDistanceY) && (event.motion.y<=buttonDistanceY+buttonHeight*buttonNum));
			SDL_UpdateRect(GfxData[SCREEN],buttonDistanceX,buttonDistanceY,buttonDistanceX+buttonWidth,buttonDistanceY+buttonHeight*buttonNum);
			break;
		case SDL_MOUSEBUTTONUP:
			if ((event.button.button==SDL_BUTTON_LEFT) && InRect(event.button.x, event.button.y,buttonDistanceX,buttonDistanceY,buttonDistanceX+buttonWidth,buttonDistanceY+buttonHeight*buttonNum))
			{	// menuitem
				j=(event.button.y-buttonDistanceY)/buttonHeight;
				value=j+1;
			}
			break;
		}
		SDL_Delay(CPUWAIT);
	}
	return value;
}

////////////////////////////////////////////////////////////////////////////////

int ValidInputChar(int c)
{
	if ((c>='a')&&(c<='z')) return c;
	if ((c=='.')||(c=='-')) return c;
	if ((c>='0')&&(c<='9')) return c;
	if ((c>='A')&&(c<='Z')) return c;
	if (c==' ') return c;
	if ((c>=SDLK_KP0)&&(c<=SDLK_KP9)) return c-SDLK_KP0+'0';
	if (c==SDLK_KP_PERIOD) return '.';
	return 0;
}

char *DialogBox(int type,const char *fmt,...)
{
	SDL_Rect rect;
	char *val;
	int vallen;
	va_list args;
	char *buf;
	int i,h,cnt=0;
	char *ptr[20];
	char *p;

	buf=(char *)malloc(4096);
	val=(char *)malloc(4096);
	va_start(args,fmt);
	vsnprintf(buf,4095,fmt,args);
	va_end(args);

	p=buf;
	ptr[0]=p;cnt=1;
	while (*p)
	{
		if (*p=='\n')
		{
			ptr[cnt++]=p+1;
			*p=0;
		}
		p++;
	}
	rect.w=352;
	rect.h=128;
	rect.x=(resX-rect.w) >> 1;
	rect.y=(resY-rect.h) >> 1;
	SDL_BlitSurface(GfxData[type],NULL,GfxData[SCREEN],&rect);
	rect.w-=4;
	rect.h-=4;
	rect.x+=2;
	rect.y+=2;
	
	if (type==DLGWINNER || type==DLGLOOSER)
		BFont_SetCurrentFont(bigfont);
	
	h=BFont_FontHeight(BFont_GetCurrentFont());
	for (i=0;i<cnt;i++)
		BFont_CenteredPutString(GfxData[SCREEN],240-h*cnt/2+h*i,ptr[i]);
	UpdateScreenRect(rect.x-2,rect.y-2,rect.w+4,rect.h+4);
	
	if (type==DLGWINNER || type==DLGLOOSER)
		BFont_SetCurrentFont(font);

	free(buf);

	if (type!=DLGNETWORK) return NULL;

	val[0]='_';val[1]=0;vallen=1;h=0;

	while (!h)
	{
		rect.x=160;
		rect.y=272;
		rect.w=320;
		rect.h=16;
		SDL_FillRect(GfxData[SCREEN],&rect,0);
		i=BFont_TextWidth(val);
		if (i<312)
			BFont_PutString(GfxData[SCREEN],164,276,val);
		else
			BFont_PutString(GfxData[SCREEN],164+(312-i),276,val);
		SDL_UpdateRect(GfxData[SCREEN],160,272,320,16);

		while (event.type!=SDL_KEYDOWN)
		{
			SDL_PollEvent(&event);		// wait for keypress
			SDL_Delay(CPUWAIT);
		}
		if (event.type==SDL_KEYDOWN)
		{
			i=ValidInputChar(event.key.keysym.sym);
			if (i&&(vallen<4095))
			{
				val[vallen-1]=i;
				val[vallen++]='_';
				val[vallen]=0;
			}
			if (((vallen>1)&&(event.key.keysym.sym==SDLK_BACKSPACE))||(event.key.keysym.sym==SDLK_DELETE))
			{
				val[--vallen]=0;
				val[vallen-1]='_';
			}
			if ((event.key.keysym.sym==SDLK_KP_ENTER)||(event.key.keysym.sym==SDLK_RETURN)) h=1;
			if (event.key.keysym.sym==SDLK_ESCAPE) h=2;
			while (event.type!=SDL_KEYUP)
			{
				SDL_PollEvent(&event);	// wait for keyrelease
					SDL_Delay(CPUWAIT);
			}
		}
	}
	if (h==2)
		return NULL;
	else
	{
		val[vallen-1]=0;
		return val;
	}
}

int InRect(int x, int y, int x1, int y1, int x2, int y2)
{
	return (x>=x1)&&(x<=x2)&&(y>=y1)&&(y<=y2);
}

void LoadSurface(char *filename,SDL_Surface **surface)
{
	*surface=IMG_Load(filename);
	if (!*surface) FatalError("File '%s' is missing or corrupt.",filename);
}

void DrawRectangle(int x, int y, int w, int h, int Colour)
{
    SDL_Rect rec;
    
    //GE: 4 "fill" rects.
	rec.x=x; rec.y=y; rec.w=w; rec.h=1;
	SDL_FillRect(GfxData[SCREEN], &rec, Colour);
	rec.x=x; rec.y=y; rec.w=1; rec.h=h;
	SDL_FillRect(GfxData[SCREEN], &rec, Colour);
	rec.x=x+w-1; rec.y=y; rec.w=1; rec.h=h;
	SDL_FillRect(GfxData[SCREEN], &rec, Colour);
	rec.x=x; rec.y=y+h-1; rec.w=w; rec.h=1;
	SDL_FillRect(GfxData[SCREEN], &rec, Colour);
}

void DoCredits()
{
	#define HGHT 30
	char *text[]={
		"Arcomage v" ARCOVER,
		"by STiCK and GreatEmerald (2005-2009)",
		"",
		"This program was originally created",
		"as Individual Software Project",
		"at Charles University",
		"Prague, Czech Republic",
		"",
		"Since 2009 it became easier to access",
		"and its development was continued.",
		"",
		"http://stick.gk2.sk/projects/arcomage/",
		"",
		"This is a clone of Arcomage, a card game",
		"originally released by New World Computing",
		"and the 3DO Company as a part of",
		"Might and Magic VII: For Blood and Honor",
		"and re-released in 2001 as a stand-alone",
		"application.",
		"Since it didn't support any kind of",
		"modifications, this open source project",
		"aims to completely remake the original",
		"and make it more flexible.",
		"",
		"Original credits follow.",
		"",
		NULL
	};
	int i,ypos=resY;
	BFont_SetCurrentFont(bigfont);
	while (event.type!=SDL_KEYDOWN || event.key.keysym.sym!=SDLK_ESCAPE)
	{
		Blit(CREDITS,SCREEN);
		i=0;
		while (text[i])
		{
			if (ypos+i*HGHT>=-20 && ypos+i*HGHT<=resX)
				BFont_CenteredPutString(GfxData[SCREEN],ypos+i*HGHT,text[i]);
			i++;
		}
		UpdateScreen();
		SDL_Delay(20);
		ypos--;
		SDL_PollEvent(&event);
	}
	BFont_SetCurrentFont(font);
}
