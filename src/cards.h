#ifndef _CARDS_H_
#define _CARDS_H_ 1

struct Stats {
	int b,g,r,q,m,d;	// bricks,gems,recruits,quarry,magic,dungeons
	int t,w;			// tower,wall
	char *Name;			// player's name
	int Hand[6];		// player's hand
};

int GetCard();
void PutCard(int c);
void InitDeck();
void SetDeck(int *d);
int Requisite(struct Stats *s,int card);
int Turn(struct Stats *s1,struct Stats *s2,int card,int turn,int *sound);
char* CardName(int card);

#endif
