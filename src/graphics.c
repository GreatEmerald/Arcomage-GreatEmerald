#include <stdlib.h>
#include <stdarg.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "BFont.h"
#include "cards.h"
#include "common.h"
#include "graphics.h"
#include "input.h"

SDL_Event event;
SDL_Surface *GfxData[GFX_CNT];

int resX=640;
int resY=480;
int buttonWidth=125;
int buttonHeight=54;
int buttonDistanceX=257; //resX/2-buttonWidth/2 = 240
int buttonDistanceY=105; //resY/2-(buttonHeight/2)*5 = 70
int buttonNum=5;

BFont_Info *numssmall=NULL;
BFont_Info *font=NULL;
BFont_Info *bigfont=NULL;

void Graphics_Init(int fullscreen)
{
	switch (OPERATINGSYSTEM)
	{
		case 1:
			LoadSurface(ARCODATADIR "boss_linux.png",&GfxData[BOSS]);break;
		default:
			LoadSurface(ARCODATADIR "boss_windows.png",&GfxData[BOSS]);break;
	}
	LoadSurface(ARCODATADIR "menu.png",&GfxData[MENU]);
	LoadSurface(ARCODATADIR "menuitems.png",&GfxData[MENUITEMS]);
	LoadSurface(ARCODATADIR "credits.png",&GfxData[CREDITS]);
	LoadSurface(ARCODATADIR "deck.png",&GfxData[DECK]);
	SDL_SetColorKey(GfxData[DECK],SDL_SRCCOLORKEY,SDL_MapRGB(GfxData[DECK]->format,255,0,255));
	LoadSurface(ARCODATADIR "nums_big.png",&GfxData[NUMSBIG]);
	SDL_SetColorKey(GfxData[NUMSBIG],SDL_SRCCOLORKEY,SDL_MapRGB(GfxData[NUMSBIG]->format,255,0,255));
	LoadSurface(ARCODATADIR "gamebg.png",&GfxData[GAMEBG]);
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
	GfxData[SCREEN]=SDL_SetVideoMode(resX,resY,0,SDL_SWSURFACE|(fullscreen*SDL_FULLSCREEN));
	if (!GfxData[SCREEN])
		FatalError("Couldn't set 640x480 video mode");
	GfxData[BUFFER]=SDL_AllocSurface(GfxData[SCREEN]->flags,GfxData[SCREEN]->w,GfxData[SCREEN]->h,GfxData[SCREEN]->format->BitsPerPixel,GfxData[SCREEN]->format->Rmask,GfxData[SCREEN]->format->Gmask,GfxData[SCREEN]->format->Bmask,0);
	if (!GfxData[BUFFER])
		FatalError("Unable to create double buffer!");
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

inline void Blit(int a,int b)
{
	SDL_BlitSurface(GfxData[a],NULL,GfxData[b],NULL);
}

inline void UpdateScreen()
{
	SDL_UpdateRect(GfxData[SCREEN],0,0,0,0);
}

inline void UpdateScreenRect(int x1,int y1,int x2,int y2)
{
	SDL_UpdateRect(GfxData[SCREEN],x1,y1,x2,y2);
}

inline void FillRect(int x,int y,int w,int h,Uint8 r,Uint8 g,Uint8 b)
{
	SDL_Rect rect;
	rect.x=x;rect.y=y;rect.w=w;rect.h=h;
	SDL_FillRect(GfxData[SCREEN],&rect,SDL_MapRGB(GfxData[SCREEN]->format,r,g,b));
}


void DrawCard(int c,int x,int y)
{
	SDL_Rect recta,rectb;
	recta.x=x;recta.y=y;recta.w=96;recta.h=128;
	rectb.x=(c&0xFF)*96;rectb.y=(c>>8)*128;rectb.w=96;rectb.h=128;
	SDL_BlitSurface(GfxData[DECK],&rectb,GfxData[SCREEN],&recta);
}

void DrawCardAlpha(int c,int x,int y,Uint8 a)
{
	SDL_SetAlpha(GfxData[DECK],SDL_SRCALPHA,a);
	DrawCard(c,x,y);
	SDL_SetAlpha(GfxData[DECK],SDL_SRCALPHA,255);
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
	for (i=0;i<2;i++)
		for (j=0;j<3;j++)
		{
			recta.w=22;recta.h=17;
			recta.x=13+547*i;recta.y=91+j*72;
			if (j==0) rectb.x=22*Players[i].q;
			if (j==1) rectb.x=22*Players[i].m;
			if (j==2) rectb.x=22*Players[i].d;
			rectb.y=0;
			rectb.w=22;rectb.h=17;
		//	print big (yellow) numbers (quarry,magic,dungeons)
			SDL_BlitSurface(GfxData[NUMSBIG],&rectb,GfxData[SCREEN],&recta);
			if (j==0) {sprintf(b,"%d",Players[i].b);b[0]-='0'-'!';if (b[1]) b[1]-='0'-'!';if (b[2]) b[2]-='0'-'!';}
			if (j==1) {sprintf(b,"%d",Players[i].g);b[0]-='0'-'+';if (b[1]) b[1]-='0'-'+';if (b[2]) b[2]-='0'-'+';}
			if (j==2) {sprintf(b,"%d",Players[i].r);b[0]-='0'-'5';if (b[1]) b[1]-='0'-'5';if (b[2]) b[2]-='0'-'5';}
		//	print small numbers (bricks,gems,recruits)
			BFont_PutStringFont(GfxData[SCREEN],numssmall,10+i*547,115+j*72,b);
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
		case SDL_MOUSEBUTTONDOWN:
			if ((event.button.button==SDL_BUTTON_LEFT) && InRect(event.button,buttonDistanceX,buttonDistanceY,buttonDistanceX+buttonWidth,buttonDistanceY+buttonHeight*buttonNum))
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
	UpdateScreen(rect.x-2,rect.y-2,rect.w+4,rect.h+4);
	
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

int InRect(SDL_MouseButtonEvent e,int x1,int y1,int x2,int y2)
{
	return (e.x>=x1)&&(e.x<=x2)&&(e.y>=y1)&&(e.y<=y2);
}

void LoadSurface(char *filename,SDL_Surface **surface)
{
	*surface=IMG_Load(filename);
	if (!*surface) FatalError("File '%s' is missing or corrupt.",filename);
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
