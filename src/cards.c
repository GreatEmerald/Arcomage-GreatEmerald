#include <stdlib.h>
#include <stdio.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "cards.h"
#include "sound.h"

int req[3][35] = {
	{0,0,0,1,3,4,7,2,5,2,3,2,3,7,8,0,5,4,6,0,8,9,9,11,13,15,16,18,24,7,1,6,10,14,17},
	{0,1,2,2,3,2,5,4,6,2,3,4,3,7,7,6,9,8,7,10,5,13,4,12,14,16,15,17,21,8,0,0,5,11,18},
	{0,0,1,1,3,2,3,4,6,3,5,6,7,8,0,5,6,6,5,8,9,11,9,10,14,11,12,15,17,25,2,2,4,13,18}
};

#define CARDS 102
struct CardInfo CardDB[CARDS]; //GE: Scheduled deprecation. Use D functions to access this instead!
int TidyQ[CARDS];
int *Q;//GE: Queue?
int Qs=0,Qe=0;
int DeckTotal; //GE: The total card number in the deck.
int bInitComplete=0;

lua_State *L;
void ShuffleQ();

int GetCard()//GE: Returns next card in the Q array.
{
	int i;
	i=Q[Qs];
	if (Qs+1 < DeckTotal)
	   Qs++;
	else
	{
      ShuffleQ();
      Qs=0;
  }
	return i;
}

void PutCard(int c)
{
	Qe=(Qe+1)%DeckTotal;
	Q[Qe]=c;
}

int CardFrequencies(int i) //GE: Set special frequencies. Some of them are not known, defaulting to 1.
{
    switch(CardDB[i].ID)
    {
        case (2<<8)+3: return 2; //Moody Goblins
        case (2<<8)+2: return 2; // Faerie
        case (2<<8)+6: return 2;	// Goblin Mob
        case (2<<8)+9: return 2;	// Orc
        case (2<<8)+10: return 2;	// Dwarves
        case (2<<8)+16: return 2;	// Ogre
        case (2<<8)+12: return 2;	// Troll Trainer
        case (1<<8)+1: return 2;	// Quartz
        case (1<<8)+9: return 2;	// Gemstone Flaw
        case (1<<8)+3: return 2;	// Amethyst
        case (1<<8)+4: return 2;	// Spell Weavers
        case (1<<8)+10: return 2;	// Ruby
        case (1<<8)+15: return 2;	// Emerald
        case (1<<8)+19: return 2;	// Sapphire
        case 3: return 2;		// Friendly Terrain
        case 7: return 2;		// Work Overtime
        case 9: return 2;		// Basic Wall
        case 12: return 2;		// Foundations
        case 4: return 2;		// Miners
        case 16: return 2;		// Big Wall
        case 20: return 2;		// Reinforced Wall
        
        
        default: return 1;
    }
}

void InitCardDB()
{
    int i;
    
    
    //GE: NEW - use Lua
    
    printf("FIRST: ");
    D_printCardDB();
    StackDump(L);
    lua_getglobal(L, "ArcomageInit"); //GE: Ask Lua to put the ArcomageInit function into the stack.
    //GE: Added a function. STACK: -1: function
    if (!lua_isfunction(L, -1)) //GE: Sanity check
        error(L, "This is not a function.");
    lua_pcall(L, 0, 1, 0); //GE: Call ArcomageInit(). Expect to get one return value, passing no parameters.
    //GE: Got results. STACK: -1: table
    printf("SECOND: ");
    StackDump(L);
    lua_pushnumber(L, 1); //GE: Read the first element from the CardDB table.
    //GE: Added a number. STACK: -1: number, -2: table
    printf("THIRD: ");
    StackDump(L);
    if (!lua_istable(L, -2)) //GE: Sanity check
        error(L, "This is not a table.");
    lua_gettable(L, -2); //GE: Put CardDB[1] onto the stack. It's CardInfo{}. Note that lua_gettable means "get from table", not "get a table" - the fact that we got a table is coincidence.
    //GE: Replaced the key with a table. STACK: -1: table, -2: table
    printf("FOURTH --");
    StackDump(L);
    
    
    
    
    if (!lua_istable(L, -1)) //GE: Sanity check
        error(L, "This is not a table.");
    lua_getfield(L, -1, "ID"); //GE: Put CardInfo.ID onto the stack. It's a number.
    //GE: Received an element. STACK: -1: number, -2: table, -3: table
    StackDump(L);
    if (!lua_isnumber(L, -1)) //GE: Sanity check
        error(L, "This is not a number.");
    D_addID(0,0,(int)lua_tonumber(L, -1));
    CardDB[0].ID = (int)lua_tonumber(L, -1); //GE: Assign the number.
    printf("Snagged ID: %d", CardDB[0].ID);
    lua_pop(L, 1); //GE: Removed one element from the stack, counting from the top.
    //GE: Removed an element. STACK: -1: table, -2: table
    StackDump(L);
    
    if (!lua_istable(L, -1)) //GE: Sanity check
        error(L, "This is not a table.");
    lua_getfield(L, -1, "Frequency"); //GE: Put CardInfo.Frequncy onto the stack. It's a number.
    //GE: Replaced the key with the table. STACK: -1: number, -2: table, -3: table
    if (!lua_isnumber(L, -1)) //GE: Sanity check
        error(L, "This is not a number.");
    CardDB[0].Frequency = (int)lua_tonumber(L, -1); //GE: Assign the number.
    printf("Snagged Frequency: %d", CardDB[0].Frequency);
    getchar();
    lua_pop(L, 1); //GE: Removed one element from the stack, counting from the top.
    //GE: Removed an element. STACK: -1: table, -2: table
    
    if (!lua_istable(L, -1)) //GE: Sanity check
        error(L, "This is not a table.");
    lua_getfield(L, -1, "Name"); //GE: Put CardInfo.Name onto the stack. It's a string.
    //GE: Replaced the key with the table. STACK: -1: string, -2: table, -3: table
    StackDump(L);
    if (!lua_isstring(L, -1)) //GE: Sanity check
        error(L, "This is not a string.");
    printf("Received string: %s\n", lua_tostring(L, -1));
    D_addName(0, 0, lua_tostring(L, -1));
    D_printCardDB();
    //strcpy(CardDB[0].Name, lua_tostring(L, -1)); //GE: Assign the string. It gets garbage'd, so make sure we copy it instead of pointing at it. Also, what kind of logic is destination, source anyway?!
    //printf("Snagged Name: %s", CardDB[0].Name);
    lua_pop(L, 1); //GE: Removed one element from the stack, counting from the top.
    //GE: Removed an element. STACK: -1: table, -2: table
    StackDump(L);
    
    if (!lua_istable(L, -1)) //GE: Sanity check
        error(L, "This is not a table.");
    lua_getfield(L, -1, "Description"); //GE: Put CardInfo.Name onto the stack. It's a string.
    //GE: Replaced the key with the table. STACK: -1: string, -2: table, -3: table
    if (!lua_isstring(L, -1)) //GE: Sanity check
        error(L, "This is not a string.");
    D_addDescription(0, 0, lua_tostring(L, -1));
    D_printCardDB();
    //strcpy(CardDB[0].Description, lua_tostring(L, -1)); //GE: Assign the string. It gets garbage'd, so make sure we copy it instead of pointing at it. Also, what kind of logic is destination, source anyway?!
    lua_pop(L, 1); //GE: Removed one element from the stack, counting from the top.
    //GE: Removed an element. STACK: -1: table, -2: table
    
    lua_pop(L, 1); //GE: Removed one element from the stack, counting from the top.
    //GE: Removed an element. STACK: -1: table
    
    DeckTotal++;
    for (i=1; i<CARDS; i++)
    {
        CardDB[i].ID = TidyQ[i]; //GE: [At this time?], TidyQ is neatly made. Let's use this to our advantage.
        strcpy(CardDB[i].Name, CardName(CardDB[i].ID));
        CardDB[i].Frequency = CardFrequencies(i);
        DeckTotal += CardDB[i].Frequency;
    } 
    
}

void ShuffleQ()
{
  int i,a,b,t;
  /*printf("DeckTotal before: %d\n", DeckTotal);
  for (i=0; i<DeckTotal; i++)
	{
      printf("Before: Q[%d]=%d\n", i, Q[i]);
  } */
  for (i=0;i<65535;i++) //GE: A ludicrous way to randomise the Q array.
	{
		a=rand()%DeckTotal;
		b=rand()%DeckTotal;
		t=Q[a];Q[a]=Q[b];Q[b]=t;
	}
	/*printf("DeckTotal after: %d\n", DeckTotal);
  for (i=0; i<DeckTotal; i++)
	{
      printf("After: Q[%d]=%d\n", i, Q[i]);
  } */
	Sound_Play(SHUFFLE);
}

void InitDeck()
{
	int i, i2=0, i3=0;
	Qs=0;
	Qe=0;
	//struct CardInfo LastEntry;
	//free(Q);
	//printf("Init complete: %d\n", bInitComplete);
  if (bInitComplete)
	{
      ShuffleQ();
      return;
  }
	
  for (i=0;i<CARDS/3;i++) //GE: Creates a neat array of cards.
	{
		TidyQ[i          ]=i+1;
		TidyQ[i+  CARDS/3]=i+1+(1<<8);
		TidyQ[i+2*CARDS/3]=i+1+(2<<8);
/*		printf("%d\n", Q[i]);
		printf("%d\n", Q[i+  CARDS/3]);
		printf("%d\n", Q[i+2*CARDS/3]);*/
	}
	InitCardDB();
	Q = (int*) malloc((sizeof (int)) * DeckTotal); //GE: Make Q as big as we want it to be.
	//printf("DeckTotal: %d\n", DeckTotal);
	for (i=0; i<CARDS; i++) //GE: Fill up Q with correct frequency.
	{
	   for( i2 = 0; i2 < CardDB[i].Frequency; i2++, i3++)
	   {
        Q[i3] = CardDB[i].ID;
        //printf("Q[%d]: %d\n", i3, CardDB[i].ID);
     }   
  }
  bInitComplete = 1;
  ShuffleQ();
}

void SetDeck(int *d)
{
	int i;
	for (i=0;i<CARDS;i++)
		Q[i]=d[i];
}

void normalize(struct Stats *s)
{
	if (s->q<1) s->q=1;
	if (s->m<1) s->m=1;
	if (s->d<1) s->d=1;
	if (s->q>99) s->q=99;
	if (s->m>99) s->m=99;
	if (s->d>99) s->d=99;

	if (s->b<0) s->b=0;
	if (s->g<0) s->g=0;
	if (s->r<0) s->r=0;
	if (s->b>999) s->b=999;
	if (s->g>999) s->g=999;
	if (s->r>999) s->r=999;

	if (s->t<0) s->t=0;
	if (s->w<0) s->w=0;
	if (s->t>200) s->t=200;
	if (s->w>200) s->w=200;
}

void damage(struct Stats *s,int how)
{
	if (s->w>=how) s->w-=how;
	else {
		s->t-=(how-s->w);
		s->w=0;
	}
}

int Requisite(struct Stats *s,int card)
{
	switch (s->Hand[card]>>8)
	{
		case 0:	if (req[0][(s->Hand[card])&0xFF]>s->b) return 0;break;
		case 1:	if (req[1][(s->Hand[card])&0xFF]>s->g) return 0;break;
		case 2:	if (req[2][(s->Hand[card])&0xFF]>s->r) return 0;
	}
	return 1;
}

void Require(struct Stats *s1, int bricks, int gems, int recruits)
{
	s1->b-=bricks;
	s1->g-=gems;
	s1->r-=recruits;
}

/*
 * GE: Use 'next=-1;' to indicate cards that initiate a "discard turn".
 * Use 'next=turn;' to indicate cards that don't cost a turn.
 */
int Deck(struct Stats *s1,struct Stats *s2,int card,int turn)
{
	int next=!turn;
	int x;

	switch (card)
	{
		case 1:		// Brick Shortage
			s1->b-=8;
			s2->b-=8;
			Sound_Play(RESS_DOWN);
			break;
		case 2:		// Lucky Cache
			s1->b+=2;
			s1->g+=2;
			Sound_Play(RESS_UP);
			next=turn;
			break;
		case 3:		// Friendly Terrain
			Require(s1, 1, 0, 0);
			s1->w++;
			Sound_Play(WALL_UP);
			next=turn;
			break;
		case 4:		// Miners
			Require(s1, 3, 0, 0);
			s1->q++;
			Sound_Play(RESB_UP);
			break;
		case 5:		// Mother Lode
			Require(s1, 4, 0, 0);
			if (s1->q<s2->q) s1->q++;
			s1->q++;
			Sound_Play(RESB_UP);
			break;
		case 6:		// Dwarven Miners
			Require(s1, 7, 0, 0);
			s1->w+=4;
			s1->q++;
			Sound_Play(WALL_UP);
			Sound_Play(RESB_UP);
			break;
		case 7:		// Work Overtime
			Require(s1, 2, 0, 0);
			s1->w+=5;
			s1->g-=6;
			Sound_Play(WALL_UP);
			Sound_Play(RESS_DOWN);
			break;
		case 8:		// Copping the Tech
			Require(s1, 5, 0, 0);
			if (s1->q<s2->q)
			{
				s1->q=s2->q;
				Sound_Play(RESB_UP);
			}
			break;
		case 9:		// Basic Wall
			Require(s1, 2, 0, 0);
			s1->w+=3;
			Sound_Play(WALL_UP);
			break;
		case 10:		// Sturdy Wall
			Require(s1, 3, 0, 0);
			s1->w+=4;
			Sound_Play(WALL_UP);
			break;
		case 11:		// Innovations
			Require(s1, 2, 0, 0);
			s1->q++;
			s2->q++;
			s1->g+=4;
			Sound_Play(RESB_UP);
			Sound_Play(RESS_UP);
			break;
		case 12:		// Foundations
			Require(s1, 3, 0, 0);
			if (!s1->w)
				s1->w+=6;
			else
				s1->w+=3;
			Sound_Play(WALL_UP);
			break;
		case 13:		// Tremors
			Require(s1, 7, 0, 0);
			s1->w-=5;
			s2->w-=5;
			Sound_Play(DAMAGE);
			next=turn;
			break;
		case 14:		// Secret Room
			Require(s1, 8, 0, 0);
			s1->m++;
			Sound_Play(RESB_UP);
			next=turn;
			break;
		case 15:		// Earthquake
			s1->q--;
			s2->q--;
			Sound_Play(RESB_DOWN);
			break;
		case 16:		// Big Wall
			Require(s1, 5, 0, 0);
			s1->w+=6;
			Sound_Play(WALL_UP);
			break;
		case 17:		// Collapse
			Require(s1, 4, 0, 0);
			s2->q--;
			Sound_Play(RESB_DOWN);
			break;
		case 18:		// New Equipment
			Require(s1, 6, 0, 0);
			s1->q+=2;
			Sound_Play(RESB_UP);
			break;
		case 19:		// Strip Mine
			s1->q--;
			s1->w+=10;
			s1->g+=5;
			Sound_Play(WALL_UP);
			Sound_Play(RESB_DOWN);
			Sound_Play(RESS_UP);
			break;
		case 20:		// Reinforced Wall
			Require(s1, 8, 0, 0);
			s1->w+=8;
			Sound_Play(WALL_UP);
			break;
		case 21:		// Porticulus
			Require(s1, 9, 0, 0);
			s1->w+=5;
			s1->d++;
			Sound_Play(WALL_UP);
			Sound_Play(RESB_UP);
			break;
		case 22:		// Crystal Rocks
			Require(s1, 9, 0, 0);
			s1->w+=7;
			s1->g+=7;
			Sound_Play(WALL_UP);
			Sound_Play(RESS_UP);
			break;
		case 23:		// Harmonic Ore
			Require(s1, 11, 0, 0);
			s1->w+=6;
			s1->t+=3;
			Sound_Play(TOWER_UP);
			Sound_Play(WALL_UP);
			break;
		case 24:		// MondoWall
			Require(s1, 13, 0, 0);
			s1->w+=12;
			Sound_Play(WALL_UP);
			break;
		case 25:		// Focused Designs
			Require(s1, 15, 0, 0);
			s1->w+=8;
			s1->t+=5;
			Sound_Play(TOWER_UP);
			Sound_Play(WALL_UP);
			break;
		case 26:		// Great Wall
			Require(s1, 16, 0, 0);
			s1->w+=15;
			Sound_Play(WALL_UP);
			break;
		case 27:		// Rock Launcher
			Require(s1, 18, 0, 0);
			s1->w+=6;
			Sound_Play(DAMAGE);
			Sound_Play(WALL_UP);
			damage(s2,10);
			break;
		case 28:		// Dragon's Heart
			Require(s1, 1, 24, 0);
			s1->w+=20;
			s1->t+=8;
			Sound_Play(TOWER_UP);
			Sound_Play(WALL_UP);
			break;
		case 29:		// Forced Labor
			Require(s1, 7, 0, 0);
			s1->w+=9;
			s1->r-=5;
			Sound_Play(WALL_UP);
			Sound_Play(RESS_DOWN);
			break;
		case 30:		// Rock Garden
			Require(s1, 1, 0, 0);
			s1->w++;
			s1->t++;
			s1->r+=2;
			Sound_Play(TOWER_UP);
			Sound_Play(WALL_UP);
			Sound_Play(RESS_UP);
			break;
		case 31:		// Flood Water
			Require(s1, 6, 0, 0);
			if (s1->w<s2->w)
			{
				s1->d--;
				s1->t-=2;
			} else {	//GE: Originally it's beneficial
				s2->d--;
				s2->t-=2;
			}
			Sound_Play(DAMAGE);
			Sound_Play(RESB_DOWN);
			break;
		case 32:		// Barracks
			Require(s1, 10, 0, 0);
			s1->r+=6;
			s1->w+=6;
			Sound_Play(WALL_UP);
			Sound_Play(RESS_UP);
			if (s1->d<s2->d)
			{
				s1->d++;
				Sound_Play(RESB_UP);
			}
			break;
		case 33:		// Battlements
			Require(s1, 14, 0, 0);
			s1->w+=7;
			Sound_Play(DAMAGE);
			Sound_Play(WALL_UP);
			damage(s2,6);
			break;
		case 34:		// Shift
			Require(s1, 17, 0, 0);
			if (s1->w!=s2->w)
			{
				Sound_Play(DAMAGE);
				Sound_Play(WALL_UP);
			}
			x=s1->w;
			s1->w=s2->w;
			s2->w=x;
			break;
		case (1<<8)+1:	// Quartz
			Require(s1, 0, 1, 0);
			s1->t++;
			Sound_Play(TOWER_UP);
			next=turn;
			break;
		case (1<<8)+2:	// Smoky Quartz
			Require(s1, 0, 2, 0);
			Sound_Play(DAMAGE);
			s2->t--;
			next=turn;
			break;
		case (1<<8)+3:	// Amethyst
			Require(s1, 0, 2, 0);
			s1->t+=3;
			Sound_Play(TOWER_UP);
			break;
		case (1<<8)+4:	// Spell Weavers
			Require(s1, 0, 3, 0);
			s1->m++;
			Sound_Play(RESB_UP);
			break;
		case (1<<8)+5:	// Prism
			Require(s1, 0, 2, 0);
			next=-1;
			break;
		case (1<<8)+6:	// Lodestone
			Require(s1, 0, 5, 0);
			s1->t+=3;
			Sound_Play(TOWER_UP);
			break;
		case (1<<8)+7:	// Solar Flare
			Require(s1, 0, 4, 0);
			s1->t+=2;
			s2->t-=2;
			Sound_Play(TOWER_UP);
			Sound_Play(DAMAGE);
			break;
		case (1<<8)+8:	// Crystal Matrix
			Require(s1, 0, 6, 0);
			s1->m++;
			s1->t+=3;
			s2->t++;
			Sound_Play(TOWER_UP);
			Sound_Play(RESB_UP);
			break;
		case (1<<8)+9:	// Gemstone Flaw
			Require(s1, 0, 2, 0);
			s2->t-=3;
			Sound_Play(DAMAGE);
			break;
		case (1<<8)+10:	// Ruby
			Require(s1, 0, 3, 0);
			s1->t+=5;
			Sound_Play(TOWER_UP);
			break;
		case (1<<8)+11:	// Gem Spear
			Require(s1, 0, 4, 0);
			s2->t-=5;
			Sound_Play(DAMAGE);
			break;
		case (1<<8)+12:	// Power Burn
			Require(s1, 0, 3, 0);
			s1->t-=5;
			s1->m+=2;
			Sound_Play(RESB_UP);
			Sound_Play(DAMAGE);
			break;
		case (1<<8)+13:	// Harmonic Vibe
			Require(s1, 0, 7, 0);
			s1->m++;
			s1->t+=3;
			s1->w+=3;
			Sound_Play(TOWER_UP);
			Sound_Play(WALL_UP);
			Sound_Play(RESB_UP);
			break;
		case (1<<8)+14:	// Parity
			Require(s1, 0, 7, 0);
			if (s2->m>s1->m)
			{
				s1->m=s2->m;
				Sound_Play(RESB_UP);
			}
			else if (s2->m<s1->m)
			{
				s2->m=s1->m;
				Sound_Play(RESB_UP);
			}
			break;
		case (1<<8)+15:	// Emerald
			Require(s1, 0, 6, 0);
			s1->t+=8;
			Sound_Play(TOWER_UP);
			break;
		case (1<<8)+16:	// Pearl of Wisdom
			Require(s1, 0, 9, 0);
			s1->t+=5;
			s1->m++;
			Sound_Play(TOWER_UP);
			Sound_Play(RESB_UP);
			break;
		case (1<<8)+17:	// Shatterer
			Require(s1, 0, 8, 0);
			s1->m--;
			s2->t-=9;
			Sound_Play(DAMAGE);
			Sound_Play(RESB_DOWN);
			break;
		case (1<<8)+18:	// Crumblestone
			Require(s1, 0, 7, 0);
			s1->t+=5;
			s2->b-=6;
			Sound_Play(TOWER_UP);
			Sound_Play(RESS_DOWN);
			break;
		case (1<<8)+19:	// Sapphire
			Require(s1, 0, 10, 0);
			s1->t+=11;
			Sound_Play(TOWER_UP);
			break;
		case (1<<8)+20:	// Discord
			Require(s1, 0, 5, 0);
			s1->t-=7;
			s2->t-=7;
			s1->m--;
			s2->m--;
			Sound_Play(DAMAGE);
			Sound_Play(RESB_DOWN);
			break;
		case (1<<8)+21:	// Fire Ruby
			Require(s1, 0, 13, 0);
			s1->t+=6;
			s2->t-=4;
			Sound_Play(TOWER_UP);
			Sound_Play(DAMAGE);
			break;
		case (1<<8)+22:	// Quarry's Help
			Require(s1, 0, 4, 0);
			s1->t+=7;
			s1->b-=10;
			Sound_Play(TOWER_UP);
			Sound_Play(RESS_DOWN);
			break;
		case (1<<8)+23:	// Crystal Shield
			Require(s1, 0, 12, 0);
			s1->t+=8;
			s1->w+=3;
			Sound_Play(TOWER_UP);
			Sound_Play(WALL_UP);
			break;
		case (1<<8)+24:	// Empathy Gem
			Require(s1, 0, 14, 0);
			s1->t+=8;
			s1->d++;
			Sound_Play(TOWER_UP);
			Sound_Play(RESB_UP);
			break;
		case (1<<8)+25:	// Diamond
			Require(s1, 0, 16, 0);
			s1->t+=15;
			Sound_Play(TOWER_UP);
			break;
		case (1<<8)+26:	// sanctuary
			Require(s1, 0, 15, 0);
			s1->t+=10;
			s1->w+=5;
			s1->r+=5;
			Sound_Play(TOWER_UP);
			Sound_Play(RESS_UP);
			Sound_Play(WALL_UP);
			break;
		case (1<<8)+27:	// Lava Jewel
			Require(s1, 0, 17, 0);
			s1->t+=12;
			damage(s2,6);
			Sound_Play(TOWER_UP);
			Sound_Play(DAMAGE);
			break;
		case (1<<8)+28:	// Dragon's Eye
			Require(s1, 0, 21, 0);
			s1->t+=20;
			Sound_Play(TOWER_UP);
			break;
		case (1<<8)+29:	// Crystallize
			Require(s1, 0, 8, 0);
			s1->t+=11;
			s1->w-=6;
			Sound_Play(TOWER_UP);
			Sound_Play(DAMAGE);
			break;
		case (1<<8)+30:	// Bag of Baubles
			if (s1->t<s2->t) s1->t++;
			s1->t++;
			Sound_Play(TOWER_UP);
			break;
		case (1<<8)+31:	// Rainbow
			s1->t++;
			s2->t++;
			s1->g+=3;
			Sound_Play(TOWER_UP);
			Sound_Play(RESS_UP);
			break;
		case (1<<8)+32:	// Apprentice
			Require(s1, 0, 5, 0);
			s1->t+=4;
			s1->r-=3;
			s2->t-=2;
			Sound_Play(TOWER_UP);
			Sound_Play(RESS_DOWN);
			Sound_Play(DAMAGE);
			break;
		case (1<<8)+33:	// Lightning Shard
			Require(s1, 0, 11, 0);
			if (s1->t>s2->w)
				s2->t-=8;
			else
				damage(s2,8);
			Sound_Play(DAMAGE);
			break;
		case (1<<8)+34:	// Phase Jewel
			Require(s1, 0, 18, 0);
			s1->t+=13;
			s1->r+=6;
			s1->b+=6;
			Sound_Play(TOWER_UP);
			Sound_Play(RESS_UP);
			break;
		case (2<<8)+1:	// Mad Cow Disease
			s1->r-=6;
			s2->r-=6;
			Sound_Play(RESS_DOWN);
			break;
		case (2<<8)+2:	// Faerie
			Require(s1, 0, 0, 1);
			Sound_Play(DAMAGE);
			damage(s2,2);
			next=turn;
			break;
		case (2<<8)+3:	// Moody Goblins
			Require(s1, 0, 0, 1);
			Sound_Play(DAMAGE);
			Sound_Play(RESS_DOWN);
			damage(s2,4);
			s1->g-=3;
			break;
		case (2<<8)+4:	// Minotaur
			Require(s1, 0, 0, 3);
			s1->d++;
			Sound_Play(RESB_UP);
			break;
		case (2<<8)+5:	// Elven Scout
			Require(s1, 0, 0, 2);
			next=-1;
			break;
		case (2<<8)+6:	// Goblin Mob
			Require(s1, 0, 0, 3);
			Sound_Play(DAMAGE);
			damage(s2,6);
			damage(s1,3);
			break;
		case (2<<8)+7:	// Goblin Archers
			Require(s1, 0, 0, 4);
			s2->t-=3;
			Sound_Play(DAMAGE);
			damage(s1,1);
			break;
		case (2<<8)+8:	// Shadow Faerie
			Require(s1, 0, 0, 6);
			Sound_Play(DAMAGE);
			s2->t-=2;
			next=turn;
			break;
		case (2<<8)+9:	// Orc
			Require(s1, 0, 0, 3);
			Sound_Play(DAMAGE);
			damage(s2,5);
			break;
		case (2<<8)+10:	// Dwarves
			Require(s1, 0, 0, 5);
			Sound_Play(DAMAGE);
			Sound_Play(WALL_UP);
			damage(s2,4);
			s1->w+=3;
			break;
		case (2<<8)+11:	// Little Snakes
			Require(s1, 0, 0, 6);
			Sound_Play(DAMAGE);
			s2->t-=4;
			break;
		case (2<<8)+12:	// Troll Trainer
			Require(s1, 0, 0, 7);
			s1->d+=2;
			Sound_Play(RESB_UP);
			break;
		case (2<<8)+13:	// Tower Gremlin
			Require(s1, 0, 0, 8);
			s1->w+=4;
			s1->t+=2;
			Sound_Play(TOWER_UP);
			Sound_Play(DAMAGE);
			Sound_Play(WALL_UP);
			damage(s2,2);
			break;
		case (2<<8)+14:	// Full Moon
			s1->d++;
			s2->d++;
			s1->r+=3;
			Sound_Play(RESB_UP);
			Sound_Play(RESS_UP);
			break;
		case (2<<8)+15:	// Slasher
			Require(s1, 0, 0, 5);
			Sound_Play(DAMAGE);
			damage(s2,6);
			break;
		case (2<<8)+16:	// Ogre
			Require(s1, 0, 0, 6);
			Sound_Play(DAMAGE);
			damage(s2,7);
			break;
		case (2<<8)+17:	// Rabid Sheep
			Require(s1, 0, 0, 6);
			Sound_Play(DAMAGE);
			Sound_Play(RESS_DOWN);
			damage(s2,6);
			s2->r-=3;
			break;
		case (2<<8)+18:	// Imp
			Require(s1, 0, 0, 5);
			Sound_Play(DAMAGE);
			Sound_Play(RESS_DOWN);
			damage(s2,6);
			s1->b-=5;
			s2->b-=5;
			s1->g-=5;
			s2->g-=5;
			s1->r-=5;
			s2->r-=5;
			break;
		case (2<<8)+19:	// Spizzer
			Require(s1, 0, 0, 8);
			Sound_Play(DAMAGE);
			if (!s2->w)
				damage(s2,10);
			else
				damage(s2,6);
			break;
		case (2<<8)+20:	// Werewolf
			Require(s1, 0, 0, 9);
			Sound_Play(DAMAGE);
			damage(s2,9);
			break;
		case (2<<8)+21:	// Corrosion Cloud
			Require(s1, 0, 0, 11);
			Sound_Play(DAMAGE);
			if (s2->w)
				damage(s2,10);
			else
				damage(s2,7);
			break;
		case (2<<8)+22:	// Unicorn
			Require(s1, 0, 0, 9);
			Sound_Play(DAMAGE);
			if (s1->m>s2->m)
				damage(s2,12);
			else
				damage(s2,8);
		case (2<<8)+23:	// Elven Archers
			Require(s1, 0, 0, 10);
			Sound_Play(DAMAGE);
			if (s1->w>s2->w)
				s2->t-=6;
			else
				damage(s2,6);
			break;
		case (2<<8)+24:	// Succubus
			Require(s1, 0, 0, 14);
			Sound_Play(DAMAGE);
			Sound_Play(RESS_DOWN);
			s2->t-=5;
			s2->r-=8;
			break;
		case (2<<8)+25:	// Rock Stompers
			Require(s1, 0, 0, 11);
			Sound_Play(DAMAGE);
			Sound_Play(RESB_DOWN);
			damage(s2,8);
			s2->q--;
			break;
		case (2<<8)+26:	// Thief
			Require(s1, 0, 0, 12);
			Sound_Play(RESS_UP);
			Sound_Play(RESS_DOWN);
			if (s2->g>=10)
			{
				s2->g-=10;
				s1->g+=5;
			} else {
				s1->g+=(s2->g+1)/2;
				s2->g=0;
			}
			if (s2->b>=5)
			{
				s2->b-=5;
				s1->b+=3;
			} else {
				s1->b+=(s2->b+1)/2;
				s2->b=0;
			}
			break;
		case (2<<8)+27:	// Stone Giant
			Require(s1, 0, 0, 15);
			Sound_Play(DAMAGE);
			Sound_Play(WALL_UP);
			damage(s2,10);
			s1->w+=4;
			break;
		case (2<<8)+28:	// Vampire
			Require(s1, 0, 0, 17);
			Sound_Play(DAMAGE);
			Sound_Play(RESB_DOWN);
			damage(s2,10);
			s2->r-=5;
			s2->d--;
			break;
		case (2<<8)+29:	// Dragon
			Require(s1, 0, 0, 25);
			Sound_Play(DAMAGE);
			Sound_Play(RESB_DOWN);
			damage(s2,20);
			s2->g-=10;
			s2->d--;
			break;
		case (2<<8)+30:	// Spearman
			Require(s1, 0, 0, 2);
			Sound_Play(DAMAGE);
			if (s1->w>s2->w)
				damage(s2,3);
			else
				damage(s2,2);
			break;
		case (2<<8)+31:	// Gnome
			Require(s1, 0, 0, 2);
			Sound_Play(DAMAGE);
			Sound_Play(RESS_UP);
			damage(s2,3);
			s1->g++;
			break;
		case (2<<8)+32:	// Berserker
			Require(s1, 0, 0, 4);
			Sound_Play(DAMAGE);
			damage(s2,8);
			s1->t-=3;
			break;
		case (2<<8)+33:	// Warlord
			Require(s1, 0, 0, 13);
			Sound_Play(DAMAGE);
			Sound_Play(RESS_DOWN);
			damage(s2,13);
			s1->g-=3;
			break;
		case (2<<8)+34:	// Pegasus Lancer
			Require(s1, 0, 0, 18);
			Sound_Play(DAMAGE);
			s2->t-=12;
			break;
	}

	return next;
}

int Turn(struct Stats *s1,struct Stats *s2,int card,int turn)
{
	int next=!turn;

/*	switch (s1->Hand[card]>>8)
	{
		case 0:s1->b-=req[0][s1->Hand[card]&0xFF];break;
		case 1:s1->g-=req[1][s1->Hand[card]&0xFF];break;
		case 2:s1->r-=req[2][s1->Hand[card]&0xFF];break;
	}*/
	next=Deck(s1, s2, card, turn);

	normalize(s1);
	normalize(s2);
	return next;
}

char* CardName(int card)
{
	switch (card)
	{
		case 1:			return "Brick Shortage";
		case 2:			return "Lucky Cache";
		case 3:			return "Friendly Terrain";
		case 4:			return "Miners";
		case 5:			return "Mother Lode";
		case 6:			return "Dwarven Miners";
		case 7:			return "Work Overtime";
		case 8:			return "Copping the Tech";
		case 9:			return "Basic Wall";
		case 10:		return "Sturdy Wall";
		case 11:		return "Innovations";
		case 12:		return "Foundations";
		case 13:		return "Tremors";
		case 14:		return "Secret Room";
		case 15:		return "Earthquake";
		case 16:		return "Big Wall";
		case 17:		return "Collapse";
		case 18:		return "New Equipment";
		case 19:		return "Strip Mine";
		case 20:		return "Reinforced Wall";
		case 21:		return "Porticulus";
		case 22:		return "Crystal Rocks";
		case 23:		return "Harmonic Ore";
		case 24:		return "MondoWall";
		case 25:		return "Focused Designs";
		case 26:		return "Great Wall";
		case 27:		return "Rock Launcher";
		case 28:		return "Dragon's Heart";
		case 29:		return "Forced Labor";
		case 30:		return "Rock Garden";
		case 31:		return "Flood Water";
		case 32:		return "Barracks";
		case 33:		return "Battlements";
		case 34:		return "Shift";
		case (1<<8)+1:	return "Quartz";
		case (1<<8)+2:	return "Smoky Quartz";
		case (1<<8)+3:	return "Amethys";
		case (1<<8)+4:	return "Spell Weavers";
		case (1<<8)+5:	return "Prism";
		case (1<<8)+6:	return "Lodestone";
		case (1<<8)+7:	return "Solar Flare";
		case (1<<8)+8:	return "Crystal Matrix";
		case (1<<8)+9:	return "Gemstone Flaw";
		case (1<<8)+10:	return "Ruby";
		case (1<<8)+11:	return "Gem Spear";
		case (1<<8)+12:	return "Power Burn";
		case (1<<8)+13:	return "Harmonic Vibe";
		case (1<<8)+14:	return "Parity";
		case (1<<8)+15:	return "Emerald";
		case (1<<8)+16:	return "Pearl of Wisdom";
		case (1<<8)+17:	return "Shatterer";
		case (1<<8)+18:	return "Crumblestone";
		case (1<<8)+19:	return "Sapphire";
		case (1<<8)+20:	return "Discord";
		case (1<<8)+21:	return "Fire Ruby";
		case (1<<8)+22:	return "Quarry's Help";
		case (1<<8)+23:	return "Crystal Shield";
		case (1<<8)+24:	return "Empathy Gem";
		case (1<<8)+25:	return "Diamond";
		case (1<<8)+26:	return "Sanctuary";
		case (1<<8)+27:	return "Lava Jewel";
		case (1<<8)+28:	return "Dragon's Eye";
		case (1<<8)+29:	return "Crystalize";
		case (1<<8)+30:	return "Bag of Baubles";
		case (1<<8)+31:	return "Rainbow";
		case (1<<8)+32:	return "Apprentice";
		case (1<<8)+33:	return "Lightning Shard";
		case (1<<8)+34:	return "Phase Jewel";
		case (2<<8)+1:	return "Mad Cow Disease";
		case (2<<8)+2:	return "Faerie";
		case (2<<8)+3:	return "Moody Goblins";
		case (2<<8)+4:	return "Minotaur";
		case (2<<8)+5:	return "Elven Scout";
		case (2<<8)+6:	return "Goblin Mob";
		case (2<<8)+7:	return "Goblin Archers";
		case (2<<8)+8:	return "Shadow Faerie";
		case (2<<8)+9:	return "Orc";
		case (2<<8)+10:	return "Dwarves";
		case (2<<8)+11:	return "Little Snakes";
		case (2<<8)+12:	return "Troll Trainer";
		case (2<<8)+13:	return "Tower Gremlin";
		case (2<<8)+14:	return "Full Moon";
		case (2<<8)+15:	return "Slasher";
		case (2<<8)+16:	return "Ogre";
		case (2<<8)+17:	return "Rabid Sheep";
		case (2<<8)+18:	return "Imp";
		case (2<<8)+19:	return "Spizzer";
		case (2<<8)+20:	return "Werewolf";
		case (2<<8)+21:	return "Corrosion Cloud";
		case (2<<8)+22:	return "Unicorn";
		case (2<<8)+23:	return "Elven Archers";
		case (2<<8)+24:	return "Succubus";
		case (2<<8)+25:	return "Rock Stompers";
		case (2<<8)+26:	return "Thief";
		case (2<<8)+27:	return "Stone Giant";
		case (2<<8)+28:	return "Vampire";
		case (2<<8)+29:	return "Dragon";
		case (2<<8)+30:	return "Spearman";
		case (2<<8)+31:	return "Gnome";
		case (2<<8)+32:	return "Berserker";
		case (2<<8)+33:	return "Warlord";
		case (2<<8)+34:	return "Pegasus Lancer";
		default:		return "";
	}
}
