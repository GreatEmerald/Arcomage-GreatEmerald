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
    int Frequency; //GE: This is the number of cards of this type in the deck.
    char *Name; 
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

#endif
