#include <stdlib.h>
#include "cards.h"
#include "sound.h"

int req[3][35] = {
	{0,0,0,1,3,4,7,2,5,2,3,2,3,7,8,0,5,4,6,0,8,9,9,11,13,15,16,18,24,7,1,6,10,14,17},
	{0,1,2,2,3,2,5,4,6,2,3,4,3,7,7,6,9,8,7,10,5,13,4,12,14,16,15,17,21,8,0,0,5,11,18},
	{0,0,1,1,3,2,3,4,6,3,5,6,7,8,0,5,6,6,5,8,9,11,9,10,14,11,12,15,17,25,2,2,4,13,18}
};

#define CARDS 102
int Q[CARDS];
int Qs=0,Qe=0;

int GetCard()
{
	int i;
	i=Q[Qs];
	Qs=(Qs+1)%CARDS;
	return i;
}

void PutCard(int c)
{
	Qe=(Qe+1)%CARDS;
	Q[Qe]=c;
}

void InitDeck()
{
	int i,a,b;
	int t;
	Qs=0;
	Qe=0;
	for (i=0;i<CARDS/3;i++)
	{
		Q[i          ]=i+1;
		Q[i+  CARDS/3]=i+1+(1<<8);
		Q[i+2*CARDS/3]=i+1+(2<<8);
	}
	for (i=0;i<65535;i++)
	{
		a=rand()%CARDS;
		b=rand()%CARDS;
		t=Q[a];Q[a]=Q[b];Q[b]=t;				
	}
}

void SetDeck(int *d)
{
	int i;
	for (i=0;i<102;i++)
		Q[i]=d[i];
}

void normalize(struct Stats *s)
{
	if (s->q<1) s->q=0;
	if (s->m<1) s->m=0;
	if (s->d<1) s->d=0;
	if (s->q>9) s->q=9;
	if (s->m>9) s->m=9;
	if (s->d>9) s->d=9;

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

int Turn(struct Stats *s1,struct Stats *s2,int card,int turn)
{
	int x;
	int next=!turn;
	switch (s1->Hand[card]>>8)
	{
		case 0:s1->b-=req[0][s1->Hand[card]&0xFF];break;
		case 1:s1->g-=req[1][s1->Hand[card]&0xFF];break;
		case 2:s1->r-=req[2][s1->Hand[card]&0xFF];break;
	}
	switch (card)
	{
		case 1:		// Brick Shortage
			s1->b-=8;
			s2->b-=8;
			SOUND_PLAY(RESS_DOWN);
			break;
		case 2:		// Lucky Cache
			s1->b+=2;
			s1->g+=2;
			SOUND_PLAY(RESS_UP);
			next=turn;
			break;
		case 3:		// Friendly Terrain
			s1->w++;
			SOUND_PLAY(WALL_UP);
			next=turn;
			break;
		case 4:		// Miners
			s1->q++;
			SOUND_PLAY(RESB_UP);
			break;
		case 5:		// Mother Lode
			if (s1->q<s2->q) s1->q++;
			s1->q++;
			SOUND_PLAY(RESB_UP);
			break;
		case 6:		// Dwarven Miners
			s1->w+=4;
			s1->q++;
			SOUND_PLAY(WALL_UP);
			SOUND_PLAY(RESB_UP);
			break;
		case 7:		// Work Overtime
			s1->w+=5;
			s1->g-=6;
			SOUND_PLAY(WALL_UP);
			SOUND_PLAY(RESS_DOWN);
			break;
		case 8:		// Copping the Tech
			if (s1->q<s2->q)
			{
				s1->q=s2->q;
				SOUND_PLAY(RESB_UP);
			}
			break;
		case 9:		// Basic Wall
			s1->w+=3;
			SOUND_PLAY(WALL_UP);
			break;
		case 10:		// Sturdy Wall
			s1->w+=4;
			SOUND_PLAY(WALL_UP);
			break;
		case 11:		// Innovations
			s1->q++;
			s2->q++;
			s1->g+=4;
			SOUND_PLAY(RESB_UP);
			SOUND_PLAY(RESS_UP);
			break;
		case 12:		// Foundations
			if (!s1->w)
				s1->w+=6;
			else
				s1->w+=3;
			SOUND_PLAY(WALL_UP);
			break;
		case 13:		// Tremors
			s1->w-=5;
			s2->w-=5;
			SOUND_PLAY(DAMAGE);
			next=turn;
			break;
		case 14:		// Secret Room
			s1->m++;
			SOUND_PLAY(RESB_UP);
			next=turn;
			break;
		case 15:		// Earthquake
			s1->q--;
			s2->q--;
			SOUND_PLAY(RESB_DOWN);
			break;
		case 16:		// Big Wall
			s1->w+=6;
			SOUND_PLAY(WALL_UP);
			break;
		case 17:		// Collapse
			s2->q--;
			SOUND_PLAY(RESB_DOWN);
			break;
		case 18:		// New Equipment
			s1->q+=2;
			SOUND_PLAY(RESB_UP);
			break;
		case 19:		// Strip Mine
			s1->q--;
			s1->w+=10;
			s1->g+=5;
			SOUND_PLAY(WALL_UP);
			SOUND_PLAY(RESB_DOWN);
			SOUND_PLAY(RESS_UP);
			break;
		case 20:		// Reinforced Wall
			s1->w+=8;
			SOUND_PLAY(WALL_UP);
			break;
		case 21:		// Porticulus
			s1->w+=5;
			s1->d++;
			SOUND_PLAY(WALL_UP);
			SOUND_PLAY(RESB_UP);
			break;
		case 22:		// Crystal Rocks
			s1->w+=7;
			s1->g+=7;
			SOUND_PLAY(WALL_UP);
			SOUND_PLAY(RESS_UP);
			break;
		case 23:		// Harmonic Ore
			s1->w+=6;
			s1->t+=3;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(WALL_UP);
			break;
		case 24:		// MondoWall
			s1->w+=12;
			SOUND_PLAY(WALL_UP);
			break;
		case 25:		// Focused Designs
			s1->w+=8;
			s1->t+=5;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(WALL_UP);
			break;
		case 26:		// Great Wall
			s1->w+=15;
			SOUND_PLAY(WALL_UP);
			break;
		case 27:		// Rock Launcher
			s1->w+=6;
			SOUND_PLAY(DAMAGE);
			SOUND_PLAY(WALL_UP);
			damage(s2,10);
			break;
		case 28:		// Dragon's Heart
			s1->w+=20;
			s1->t+=8;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(WALL_UP);
			break;
		case 29:		// Forced Labor
			s1->w+=9;
			s1->r-=5;
			SOUND_PLAY(WALL_UP);
			SOUND_PLAY(RESS_DOWN);
			break;
		case 30:		// Rock Garden
			s1->w++;
			s1->t++;
			s1->r+=2;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(WALL_UP);
			SOUND_PLAY(RESS_UP);
			break;
		case 31:		// Flood Water
			if (s1->w>s2->w)
			{
				s2->d--;
				s2->t-=2;
			} else {
				s1->d--;
				s1->t-=2;
			}
			SOUND_PLAY(DAMAGE);
			SOUND_PLAY(RESB_DOWN);
			break;
		case 32:		// Barracks
			s1->r+=6;
			s1->w+=6;
			SOUND_PLAY(WALL_UP);
			SOUND_PLAY(RESS_UP);
			if (s1->d<s2->d)
			{
				s1->d++;
				SOUND_PLAY(RESB_UP);
			}
			break;
		case 33:		// Battlements
			s1->w+=7;
			SOUND_PLAY(DAMAGE);
			SOUND_PLAY(WALL_UP);
			damage(s2,6);
			break;
		case 34:		// Shift
			if (s1->w!=s2->w)
			{
				SOUND_PLAY(DAMAGE);
				SOUND_PLAY(WALL_UP);
			}
			x=s1->w;
			s1->w=s2->w;
			s2->w=x;
			break;
		case (1<<8)+1:	// Quartz
			s1->t++;
			SOUND_PLAY(TOWER_UP);
			next=turn;
			break;
		case (1<<8)+2:	// Smoky Quartz
			SOUND_PLAY(DAMAGE);
			damage(s2,1);
			next=turn;
			break;
		case (1<<8)+3:	// Amethyst
			s1->t+=3;
			SOUND_PLAY(TOWER_UP);
			break;
		case (1<<8)+4:	// Spell Weavers
			s1->m++;
			SOUND_PLAY(RESB_UP);
			break;
		case (1<<8)+5:	// Prism
			// TODO
			next=turn;
			break;
		case (1<<8)+6:	// Lodestone
			s1->t+=3;
			SOUND_PLAY(TOWER_UP);
			break;
		case (1<<8)+7:	// Solar Flare
			s1->t+=2;
			s2->t-=2;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(DAMAGE);
			break;
		case (1<<8)+8:	// Crystal Matrix
			s1->m++;
			s1->t+=3;
			s2->t++;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(RESB_UP);
			break;
		case (1<<8)+9:	// Gemstone Flaw
			s2->t-=3;
			SOUND_PLAY(DAMAGE);
			break;
		case (1<<8)+10:	// Ruby
			s1->t+=5;
			SOUND_PLAY(TOWER_UP);
			break;
		case (1<<8)+11:	// Gem Spear
			s2->t-=5;
			SOUND_PLAY(DAMAGE);
			break;
		case (1<<8)+12:	// Power Burn
			s1->t-=5;
			s1->m+=2;
			SOUND_PLAY(RESB_UP);
			SOUND_PLAY(DAMAGE);
			break;
		case (1<<8)+13:	// Harmonic Vibe
			s1->m++;
			s1->t+=3;
			s1->w+=3;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(WALL_UP);
			SOUND_PLAY(RESB_UP);
			break;
		case (1<<8)+14:	// Parity
			if (s2->m>s1->m)
			{
				s1->m=s2->m;
				SOUND_PLAY(RESB_UP);
			}
			else if (s2->m<s1->m)
			{
				s2->m=s1->m;
				SOUND_PLAY(RESB_UP);
			}
			break;
		case (1<<8)+15:	// Emerald
			s1->t+=8;
			SOUND_PLAY(TOWER_UP);
			break;
		case (1<<8)+16:	// Pearl of Wisdom
			s1->t+=5;
			s1->m++;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(RESB_UP);
			break;
		case (1<<8)+17:	// Shatterer
			s1->m--;
			s2->t-=9;
			SOUND_PLAY(DAMAGE);
			SOUND_PLAY(RESB_DOWN);
			break;
		case (1<<8)+18:	// Crumblestone
			s1->t+=5;
			s2->b-=6;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(RESS_DOWN);
			break;
		case (1<<8)+19:	// Sapphire
			s1->t+=11;
			SOUND_PLAY(TOWER_UP);
			break;
		case (1<<8)+20:	// Discord
			s1->t-=7;
			s2->t-=7;
			s1->m--;
			s2->m--;
			SOUND_PLAY(DAMAGE);
			SOUND_PLAY(RESB_DOWN);
			break;
		case (1<<8)+21:	// Fire Ruby
			s1->t+=6;
			s2->t-=4;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(DAMAGE);
			break;
		case (1<<8)+22:	// Quarry's Help
			s1->t+=7;
			s1->b-=10;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(RESS_DOWN);
			break;
		case (1<<8)+23:	// Crystal Shield
			s1->t+=8;
			s1->w+=3;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(WALL_UP);
			break;
		case (1<<8)+24:	// Empathy Gem
			s1->t+=8;
			s1->d++;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(RESB_UP);
			break;
		case (1<<8)+25:	// Diamond
			s1->t+=15;
			SOUND_PLAY(TOWER_UP);
			break;
		case (1<<8)+26:	// sanctuary
			s1->t+=10;
			s1->w+=5;
			s1->r+=5;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(RESS_UP);
			SOUND_PLAY(WALL_UP);
			break;
		case (1<<8)+27:	// Lava Jewel
			s1->t+=12;
			damage(s2,6);
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(DAMAGE);
			break;
		case (1<<8)+28:	// Dragon's Eye
			s1->t+=20;
			SOUND_PLAY(TOWER_UP);
			break;
		case (1<<8)+29:	// Crystallize
			s1->t+=11;
			s1->w-=6;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(DAMAGE);
			break;
		case (1<<8)+30:	// Bag of Baubles
			if (s1->t<s2->t) s1->t++;
			s1->t++;
			SOUND_PLAY(TOWER_UP);
			break;
		case (1<<8)+31:	// Rainbow
			s1->t++;
			s2->t++;
			s1->g+=3;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(RESS_UP);
			break;
		case (1<<8)+32:	// Apprentice
			s1->t+=4;
			s1->r-=3;
			s2->t-=2;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(RESS_DOWN);
			SOUND_PLAY(DAMAGE);
			break;
		case (1<<8)+33:	// Lightning Shard
			if (s1->t>s2->w)
				damage(s2,8);
			else
				damage(s2,4);
			SOUND_PLAY(DAMAGE);
			break;
		case (1<<8)+34:	// Phase Jewel
			s1->t+=13;
			s1->r+=6;
			s1->b+=6;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(RESS_UP);
			break;
		case (2<<8)+1:	// Mad Cow Disease
			s1->r-=6;
			s2->r-=6;
			SOUND_PLAY(RESS_DOWN);
			break;
		case (2<<8)+2:	// Faerie
			SOUND_PLAY(DAMAGE);
			damage(s2,2);
			next=turn;
			break;
		case (2<<8)+3:	// Moody Goblins
			SOUND_PLAY(DAMAGE);
			damage(s2,4);
			s1->g-=3;
			break;
		case (2<<8)+4:	// Minotaur
			s1->d++;
			SOUND_PLAY(RESB_UP);
			break;
		case (2<<8)+5:	// Elven Scout
			// TODO
			next=turn;
			break;
		case (2<<8)+6:	// Goblin Mob
			SOUND_PLAY(DAMAGE);
			damage(s2,6);
			damage(s1,3);
			break;
		case (2<<8)+7:	// Goblin Archers
			s2->t-=3;
			SOUND_PLAY(DAMAGE);
			damage(s1,1);
			break;
		case (2<<8)+8:	// Shadow Faerie
			SOUND_PLAY(DAMAGE);
			damage(s2,2);
			next=turn;
			break;
		case (2<<8)+9:	// Orc
			SOUND_PLAY(DAMAGE);
			damage(s2,5);
			break;
		case (2<<8)+10:	// Dwarves
			SOUND_PLAY(DAMAGE);
			SOUND_PLAY(WALL_UP);
			damage(s2,4);
			s1->w+=3;
			break;
		case (2<<8)+11:	// Little Snakes
			SOUND_PLAY(DAMAGE);
			s2->t-=4;
			break;
		case (2<<8)+12:	// Troll Trainer
			s1->d+=2;
			SOUND_PLAY(RESB_UP);
			break;
		case (2<<8)+13:	// Tower Gremlin
			s1->w+=4;
			s1->t+=2;
			SOUND_PLAY(TOWER_UP);
			SOUND_PLAY(DAMAGE);
			SOUND_PLAY(WALL_UP);
			damage(s2,2);
			break;
		case (2<<8)+14:	// Full Moon
			s1->d++;
			s2->d++;
			s1->r+=3;
			SOUND_PLAY(RESB_UP);
			SOUND_PLAY(RESS_UP);
			break;
		case (2<<8)+15:	// Slasher
			SOUND_PLAY(DAMAGE);
			damage(s2,6);
			break;
		case (2<<8)+16:	// Ogre
			SOUND_PLAY(DAMAGE);
			damage(s2,7);
			break;
		case (2<<8)+17:	// Rabid Sheep
			SOUND_PLAY(DAMAGE);
			SOUND_PLAY(RESS_DOWN);
			damage(s2,6);
			s2->r-=3;
			break;
		case (2<<8)+18:	// Imp
			SOUND_PLAY(DAMAGE);
			SOUND_PLAY(RESS_DOWN);
			damage(s2,6);
			s1->b-=5;
			s2->b-=5;
			s1->g-=5;
			s2->g-=5;
			s1->r-=5;
			s2->r-=5;
			break;
		case (2<<8)+19:	// Spizzer
			SOUND_PLAY(DAMAGE);
			if (!s2->w)
				damage(s2,10);
			else
				damage(s2,6);
			break;
		case (2<<8)+20:	// Werewolf
			SOUND_PLAY(DAMAGE);
			damage(s2,9);
			break;
		case (2<<8)+21:	// Corrosion Cloud
			SOUND_PLAY(DAMAGE);
			if (s2->w)
				damage(s2,10);
			else
				damage(s2,7);
			break;
		case (2<<8)+22:	// Unicorn
			SOUND_PLAY(DAMAGE);
			if (s1->m>s2->m)
				damage(s2,12);
			else
				damage(s2,8);
		case (2<<8)+23:	// Elven Archers
			SOUND_PLAY(DAMAGE);
			if (s1->w>s2->w)
				s2->t-=6;
			else
				damage(s2,6);
			break;
		case (2<<8)+24:	// Succubus
			SOUND_PLAY(DAMAGE);
			SOUND_PLAY(RESS_DOWN);
			s2->t-=5;
			s2->r-=8;
			break;
		case (2<<8)+25:	// Rock Stompers
			SOUND_PLAY(DAMAGE);
			SOUND_PLAY(RESB_DOWN);
			damage(s2,8);
			s2->q--;
			break;
		case (2<<8)+26:	// Thief
			SOUND_PLAY(RESS_UP);
			SOUND_PLAY(RESS_DOWN);
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
			SOUND_PLAY(DAMAGE);
			SOUND_PLAY(WALL_UP);
			damage(s2,10);
			s1->w+=4;
			break;
		case (2<<8)+28:	// Vampire
			SOUND_PLAY(DAMAGE);
			SOUND_PLAY(RESB_DOWN);
			damage(s2,10);
			s2->r-=5;
			s2->d--;
			break;
		case (2<<8)+29:	// Dragon
			SOUND_PLAY(DAMAGE);
			SOUND_PLAY(RESB_DOWN);
			damage(s2,20);
			s2->g-=10;
			s2->d--;
			break;
		case (2<<8)+30:	// Spearman
			SOUND_PLAY(DAMAGE);
			if (s1->w>s2->w)
				damage(s2,3);
			else
				damage(s2,2);
			break;
		case (2<<8)+31:	// Gnome
			SOUND_PLAY(DAMAGE);
			SOUND_PLAY(RESS_UP);
			damage(s2,3);
			s1->g++;
			break;
		case (2<<8)+32:	// Berserker
			SOUND_PLAY(DAMAGE);
			damage(s2,8);
			s1->t-=3;
			break;
		case (2<<8)+33:	// Warlord
			SOUND_PLAY(DAMAGE);
			SOUND_PLAY(RESS_DOWN);
			damage(s2,13);
			s1->g-=3;
			break;
		case (2<<8)+34:	// Pegasus Lancer
			SOUND_PLAY(DAMAGE);
			s2->t-=12;
			break;
	}
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
