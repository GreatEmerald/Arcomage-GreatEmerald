#ifndef _CARDS_H_
#define _CARDS_H_ 1

struct Stats {
	int b,g,r,q,m,d;	// bricks,gems,recruits,quarry,magic,dungeons
	int t,w;			// tower,wall
	char *Name;			// player's name
	int Hand[6];		// player's hand
};

struct CardInfo {
    int ID;
    int Frequency; //GE: This is the number of cards of this type in the deck. 1=Rare, 2=Uncommon, 3=Common
    char Name[128];
    char Description[128];
    int BrickCost; //GE: These three are for rendering purposes, but are used in code as well
    int GemCost;
    int RecruitCost;
    char *Colour; //GE: Red, Geen, Blue, Gray/Grey/Black, Brown/White. Rendering purposes, mostly for 0 cost coloured cards
    char *Picture; //GE: Rendering purposes. Might also add coordinates.
    char *Keywords; //GE: Might become an array. These are MArcomage keywords, also used in Lua functions
    char *LuaFunction; //GE: This is what we call on playing the card.
};                                  
//GE: The longest names are with 16 characters right now. ...blasted char arrays :\
//GE: Also funny that it's an array of structs of arrays.
int GetCard();
void PutCard(int c);
//int CardFrequencies(int i);
//void InitCardDB();
void InitDeck();
void SetDeck(int *d);
int Requisite(struct Stats *s,int card);
int Turn(struct Stats *s1,struct Stats *s2,int card,int turn);
char* CardName(int card);

//GE: Functions called in D code.
void D_LinuxInit(); //GE: Special initialisation needed to link the D lib in Linux
int rt_init(); //GE: Initialisation and termination of the D runtime.
int rt_term();

void D_addID(int Pool, int Card, int ID); //GE: These all are responsible for maintaining the Card pools.
void D_addFrequency(int Pool, int Card, int Frequency);
void D_addName(int Pool, int Card, const char* Name);
void D_addDescription(int Pool, int Card, const char* Description);
void D_printCardDB();

#endif
