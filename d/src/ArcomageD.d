module ArcomageD;
import std.stdio; //GE: Debugging purposes so far.
import std.conv;

struct CardInfo { //GE: Holds information about a single card.
    int ID;
    int Frequency; //GE: This is the number of cards of this type in the deck. 1=Rare, 2=Uncommon, 3=Common
    string Name;
    string Description;
    int BrickCost; //GE: These three are for rendering purposes, but are used in code as well
    int GemCost;
    int RecruitCost;
    string Colour; //GE: Red, Geen, Blue, Gray/Grey/Black, Brown/White. Rendering purposes, mostly for 0 cost coloured cards
    string Picture; //GE: Rendering purposes. Might also add coordinates.
    string Keywords; //GE: Might become an array. These are MArcomage keywords, also used in Lua functions
    string LuaFunction; //GE: This is what we call on playing the card.
};
//CardInfo[] CardPool; //GE: Holds information about all cards in a single Card Pool.
/*struct CardPoolInfo {
    CardPool Pool;
    string Name;
} */
CardInfo[][] CardDB; //GE: Holds information about all the different loaded Card Pools.

version(linux) //GE: Linux needs an entry point.
{
    int main()
    {
        return 0;
    }
}

//GE: Make sure the array we have is big enough to use.
void setBounds(int Pool, int Card)
{
    if (Pool >= CardDB.length)
        CardDB.length = Pool+1;
    if (Card >= CardDB[Pool].length)
        CardDB[Pool].length = Card+1;
}


//GE: Declare initialisation and termination of the D runtime.
extern (C) bool  rt_init( void delegate( Exception ) dg = null );
extern (C) bool  rt_term( void delegate( Exception ) dg = null );

/*
 * GE: C code sends us the information it's gathering from Lua. We are here to
 * build a CardDB. C is aware of the pool it's accessing and of which card it's
 * looking at, so it sends us the information.
 * Might need to make a struct which would hold the name of the pool.   
 */

extern(C):
    void D_LinuxInit() //GE: Special Linux initialisation.
    {
        version(linux)
            main();
    }
    
    void D_addID(int Pool, int Card, int ID)
    {
        setBounds(Pool, Card);
        CardDB[Pool][Card].ID = ID;
        writeln("CardDB.Length is ", CardDB.length, " and that pool has ", CardDB[Pool].length, " cards registered so far.");
    }

    void D_addFrequency(int Pool, int Card, int Frequency)
    {
        setBounds(Pool, Card);
        CardDB[Pool][Card].Frequency = Frequency;
        writeln("CardDB.Length is ", CardDB.length, " and that pool has ", CardDB[Pool].length, " cards registered so far.");
    }

    void D_addName(int Pool, int Card, const char* Name)
    {
        setBounds(Pool, Card);
        CardDB[Pool][Card].Name = to!string(Name);
        writeln("CardDB.Length is ", CardDB.length, " and that pool has ", CardDB[Pool].length, " cards registered so far.");
    }

    void D_addDescription(int Pool, int Card, const char* Description)
    {
        setBounds(Pool, Card);
        CardDB[Pool][Card].Description = to!string(Description);
        writeln("CardDB.Length is ", CardDB.length, " and that pool has ", CardDB[Pool].length, " cards registered so far.");
    }

    void D_printCardDB()
    {
        for (int Pool=0; Pool < CardDB.length; Pool++)
        {
            for (int Card=0; Card < CardDB[Pool].length; Card++)
            {
                writeln("CardDB[", Pool, "][", Card, "] = ", CardDB[Pool][Card]);
            }
        }
    }
