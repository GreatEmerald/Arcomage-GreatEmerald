/**
 * The main class of Arcomage.
 * Initialisation, configuration, input loop, card play event loop and certain
 * other important functions are here.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "common.h"
#include "config.h"
#include "graphics.h"
#include "input.h"
#include "network.h"
#include "minIni.h"
#include "sound.h"
#include "cards.h"

SDL_Event event; ///< Event placeholder.

int turn=0; ///< Number of the player whose turn it is.
int nextturn=0; ///< Number of the player who will go next.
int lastturn=0; ///< Number of the player whose turn ended before.
int bSpecialTurn=0; ///< Used for determining whether or not this is a discarding turn.
int bRefreshNeeded=0; ///< True if we need to refresh the screen. Used in the input loop.
int aiplayer=-1; ///< Used in AI games.
int netplayer=-1; ///< Used in network games.
struct Stats Player[2]; ///< Players. Bugs: Doesn't support more than 2 players.
lua_State *L; ///< Lua support, main state.


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

/**
 * Dumps the contents of the Lua stack.
 *
 * Usually used for debugging. Should never be called in the release version.
 *
 * Authors: GreatEmerald.
 */
void StackDump (lua_State *L)
{
    int a;
    int top = lua_gettop(L);
    for (a = 1; a <= top; a++)  /* repeat for each level */
    {
        int t = lua_type(L, -a);
        printf("%d: ", -a);
        switch (t) {

          case LUA_TSTRING:  /* strings */
            printf("`%s'", lua_tostring(L, -a));
            break;

          case LUA_TBOOLEAN:  /* booleans */
            printf(lua_toboolean(L, -a) ? "true" : "false");
            break;

          case LUA_TNUMBER:  /* numbers */
            printf("%g", lua_tonumber(L, -a));
            break;

          default:  /* other values */
            printf("%s", lua_typename(L, t));
            break;

        }
        printf(", ");
    }
    getchar();
    printf("\n");
}

/**
 * Error handling.
 *
 * Dumps the Lua stack, writes information into the error file and halts the
 * program so the user could see what happened.
 *
 * Authors: GreatEmerald.
 *
 * \param fmt Reason for error.
 */
void error (lua_State *L, const char *fmt, ...)
{
    StackDump(L); //GE: Dump the stack so we'd know what the hell is going on
    va_list argp;
    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);
    lua_close(L);
    getchar();
    exit(EXIT_FAILURE);
}

/**
 * Lua initialisation.
 *
 * Authors: GreatEmerald.
 */
void InitLua()
{
    L = lua_open();
    luaL_openlibs(L);
    if (luaL_loadfile(L,"lua/CardPools.lua"))
      error(L, "Could not access card pool!");
    if (lua_pcall(L, 0, 0, 0))
        error(L, "Protected call failed!");

    lua_register(L, "Damage", L_Damage);
    lua_register(L, "RemoveBricks", L_RemoveBricks);
    lua_register(L, "RemoveGems", L_RemoveGems);
    lua_register(L, "RemoveRecruits", L_RemoveRecruits);
    lua_register(L, "RemoveWall", L_RemoveWall);
    lua_register(L, "RemoveTower", L_RemoveTower);
    lua_register(L, "RemoveQuarry", L_RemoveQuarry);
    lua_register(L, "RemoveDungeon", L_RemoveDungeon);
    lua_register(L, "RemoveMagic", L_RemoveMagic);
    lua_register(L, "AddBricks", L_AddBricks);
    lua_register(L, "AddGems", L_AddGems);
    lua_register(L, "AddRecruits", L_AddRecruits);
    lua_register(L, "AddWall", L_AddWall);
    lua_register(L, "AddTower", L_AddTower);
    lua_register(L, "AddQuarry", L_AddQuarry);
    lua_register(L, "AddMagic", L_AddMagic);
    lua_register(L, "AddDungeon", L_AddDungeon);
    lua_register(L, "SetQuarry", L_SetQuarry);
    lua_register(L, "SetWall", L_SetWall);
    lua_register(L, "SetMagic", L_SetMagic);
    lua_register(L, "GetWall", L_GetWall);
    lua_register(L, "GetQuarry", L_GetQuarry);
    lua_register(L, "GetDungeon", L_GetDungeon);
    lua_register(L, "GetMagic", L_GetMagic);
    lua_register(L, "GetTower", L_GetTower);
    lua_register(L, "GetGems", L_GetGems);
    lua_register(L, "GetBricks", L_GetBricks);
    lua_register(L, "GetRecruits", L_GetRecruits);
}

/// D initialisation.
/// Authors: GreatEmerald.
void InitD()
{
  rt_init();
  D_LinuxInit();
}

/// Main initialisation.
/// Authors: GreatEmerald, STiCK.
void Init()
{
    InitLua();
    InitD();

    ReadConfig();

    atexit(SDL_Quit); //GE: What is this for?
    if (soundenabled) Sound_Init();
    Graphics_Init(fullscreen);
}

/// Exit sequence.
/// Authors: GreatEmerald, STiCK.
void Quit()
{
    Graphics_Quit();
    Sound_Quit();
    lua_close(L); //GE: Close Lua
    rt_term(); //GE: Terminate D
}

/**
 * Functionality when playing a card.
 *
 * Plays the animation, handles the turn sequence, distributes resources
 *
 * Bugs: Should be split to different functions for readability.
 *
 * Authors: GreatEmerald, STiCK.
 * \param c Card ID.
 * \param discrd Whether to discard the card. It's used as a boolean.
 */
void PlayCard(int c,int discrd)
{
#define STEPS 10
    int sound;
    double d,x,y;
    int bGiveResources=0, i;

    //GE: You get resources when you use a card and next up is the enemy's turn.

    if (discrd && D_getCursed(0,Player[turn].Hand[c]))
        return;     // Cursed cards like LodeStone can't be discarded

    if (bSpecialTurn && !discrd) //GE: You're trying to play a card during a discard round. Bad.
       return;

    FillRect(8+106*c,342,96,128,0,0,0);
    Blit(SCREEN,BUFFER);
    for (d=0.0;d<=1.0;d+=1.0/STEPS)
    {
        x=(8.0+106.0*c)+d*(272.0-(8.0+106.0*c));
        y=342.0+d*(96.0-342.0);
        Blit(BUFFER,SCREEN);
        DrawCard(Player[turn].Hand[c],(int)x,(int)y,CardTranslucency);
        if (discrd)
            DrawDiscard((int)x,(int)y);
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
        DrawCard(Player[turn].Hand[c],272,96,CardTranslucency);
        DrawDiscard(272,96);
    }
    else
        DrawCard(Player[turn].Hand[c],272,96,255);
    PutCard(Player[turn].Hand[c]);
    if (bGiveResources) //GE: if you didn't put a play again card or you have discarded
    {
        Player[nextturn].b+=Player[nextturn].q; //GE: The enemy gets resources.
        Player[nextturn].g+=Player[nextturn].m;
        Player[nextturn].r+=Player[nextturn].d;
    }
    Player[turn].Hand[c]=GetCard();
    printf("We received a card: %d\n", Player[turn].Hand[c]); //GE: DEBUG
    lastturn=turn;
    turn=nextturn;

    DrawStatus(turn,Player);

    DrawCards(turn);
    UpdateScreen();
}

/**
 * Artificial intelligence support.
 *
 * Bugs: Picks random cards, should be transferred to Lua.
 *
 * Authors: GreatEmerald, STiCK.
 */
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

/**
 * Initialisation before the card game begins.
 *
 * Includes getting the first cards and setting up initial conditions.
 *
 * Authors: GreatEmerald, STiCK.
 */
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

///Remote play.
///Authors: STiCK.
int NetRemPlay(int *t,int *d)
{
    char *s;
    int r;

    r=Recv(&s,0);
    *t=s[0];*d=s[1];

    return r;
}

/// Net Local Play.
/// Authors: STiCK.
void NetLocPlay(int t,int d,int card)
{
    char s[5];
    s[0]=t;s[1]=d;s[2]=turn;
    s[3]=card >> 8;
    s[4]=card & 0xFF;
    Send(s,5,0);
}

/**
 * The main loop in the game.
 *
 * Includes the event loop, victory/loss handling, AI and network support.
 *
 * Authors: STiCK, GreatEmerald.
 */
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
                if ( event.type == SDL_MOUSEMOTION && InRect(event.motion.x, event.motion.y,   8,342,  8+94,468) ) //GE: Support for highlighting cards, to be done: card tooltips.
                {
                    Blit(SCREEN, BUFFER);
                    DrawRectangle(8,342,96,128, 0xFF0000);
                    UpdateScreen();
                    bRefreshNeeded=1;
                }
                else if(bRefreshNeeded)
                {
                    RedrawScreen(turn, Player);
                    bRefreshNeeded=0;
                }

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
            if (aiplayer!=-1 || netplayer!=-1)              // 1 local Player
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
            } else {                                         // 2 local Players
                DialogBox(DLGWINNER,"Winner is\n%s !",Player[Winner(1)].Name);
                Sound_Play(VICTORY);
            }
        }
        WaitForInput();
    }
}

/// Network game support.
/// Authors: STiCK.
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

/**
 * Argument parsing.
 *
 * Deprecated: Use minIni instead.
 *
 * Authors: STiCK.
 */
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

/**
 * Main function and menu.
 *
 * Calls initialisation functions and performs main menu functions.
 *
 * Bugs: Should allow the player to input his name. Alternatively, use minIni.
 *
 * Authors: STiCK.
 */
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
