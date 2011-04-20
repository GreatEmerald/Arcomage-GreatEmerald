module ArcomageD;
import std.stdio; //GE: Debugging purposes so far.
import std.conv;
import std.string;

/*struct Coords {
    int X, Y, W, H;
} */
struct SDL_Rect {
	short x, y;
	ushort w, h;
};

struct Rect {
	int x, y, w, h;
};

struct PictureInfo {
    string File;
    SDL_Rect Coordinates;
    //int X;
    //int Y;
    //int H;
    //int W;
}

struct CardInfo { //GE: Holds information about a single card.
    int ID;
    int Frequency; //GE: This is the number of cards of this type in the deck. 1=Rare, 2=Uncommon, 3=Common
    string Name;
    string Description;
    int BrickCost; //GE: These three are for rendering purposes, but are used in code as well
    int GemCost;
    int RecruitCost;
    bool Cursed;
    string Colour; //GE: Red, Geen, Blue, Gray/Grey/Black, Brown/White. Rendering purposes, mostly for 0 cost coloured cards
    PictureInfo Picture; //GE: Rendering purposes.
    string Keywords; //GE: Might become an array. These are MArcomage keywords, also used in Lua functions
    string LuaFunction; //GE: This is what we call on playing the card.
};
//CardInfo[] CardPool; //GE: Holds information about all cards in a single Card Pool.
/*struct CardPoolInfo {
    CardPool Pool;
    string Name;
} */
CardInfo[][] CardDB; //GE: Holds information about all the different loaded Card Pools.
string[] PrecachePictures;
//int[] Queue; //GE: This is the list of card IDs in the bank.

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
    
    void D_setID(int Pool, int Card, int ID)
    {
        setBounds(Pool, Card);
        CardDB[Pool][Card].ID = ID;
        //writeln("CardDB.Length is ", CardDB.length, " and that pool has ", CardDB[Pool].length, " cards registered so far.");
    }

    void D_setFrequency(int Pool, int Card, int Frequency)
    {
        setBounds(Pool, Card);
        CardDB[Pool][Card].Frequency = Frequency;
        //writeln("CardDB.Length is ", CardDB.length, " and that pool has ", CardDB[Pool].length, " cards registered so far.");
    }

    void D_setName(int Pool, int Card, const char* Name)
    {
        setBounds(Pool, Card);
        CardDB[Pool][Card].Name = to!string(Name);
        //writeln("Named card: ", CardDB[Pool][Card].Name);
        //writeln("CardDB.Length is ", CardDB.length, " and that pool has ", CardDB[Pool].length, " cards registered so far.");
    }

    void D_setDescription(int Pool, int Card, const char* Description)
    {
        setBounds(Pool, Card);
        CardDB[Pool][Card].Description = to!string(Description);
        //writeln("CardDB.Length is ", CardDB.length, " and that pool has ", CardDB[Pool].length, " cards registered so far.");
    }

    void D_setBrickCost(int Pool, int Card, int BrickCost)
    {
        setBounds(Pool, Card);
        CardDB[Pool][Card].BrickCost = BrickCost;
        //writeln("CardDB.Length is ", CardDB.length, " and that pool has ", CardDB[Pool].length, " cards registered so far.");
    }

    void D_setGemCost(int Pool, int Card, int GemCost)
    {
        setBounds(Pool, Card);
        CardDB[Pool][Card].GemCost = GemCost;
        //writeln("CardDB.Length is ", CardDB.length, " and that pool has ", CardDB[Pool].length, " cards registered so far.");
    }

    void D_setRecruitCost(int Pool, int Card, int RecruitCost)
    {
        setBounds(Pool, Card);
        CardDB[Pool][Card].RecruitCost = RecruitCost;
        //writeln("CardDB.Length is ", CardDB.length, " and that pool has ", CardDB[Pool].length, " cards registered so far.");
    }

    void D_setCursed(int Pool, int Card, int Cursed)
    {
        setBounds(Pool, Card);
        CardDB[Pool][Card].Cursed = cast(bool)Cursed;
	//writeln("Cursed: ", cast(bool)Cursed);
        //writeln("CardDB.Length is ", CardDB.length, " and that pool has ", CardDB[Pool].length, " cards registered so far.");
    }

    void D_setColour(int Pool, int Card, const char* Colour)
    {
        setBounds(Pool, Card);
        CardDB[Pool][Card].Colour = to!string(Colour);
        //writeln("CardDB.Length is ", CardDB.length, " and that pool has ", CardDB[Pool].length, " cards registered so far.");
    }

    void D_setPictureFile(int Pool, int Card, const char* File)
    {
        setBounds(Pool, Card);
        CardDB[Pool][Card].Picture.File = to!string(File);
    }

    void D_setPictureCoords(int Pool, int Card, int X, int Y, int W, int H)
    {
        setBounds(Pool, Card);
        CardDB[Pool][Card].Picture.Coordinates.x = to!short(X);
        CardDB[Pool][Card].Picture.Coordinates.y = to!short(Y);
        CardDB[Pool][Card].Picture.Coordinates.w = to!ushort(W);
        CardDB[Pool][Card].Picture.Coordinates.h = to!ushort(H);
    }

    // GE: GET CODE BEGIN ---------------------------------------

    int D_getFrequency(int Pool, int Card)
    {
        return CardDB[Pool][Card].Frequency;
    }

    int D_getBrickCost(int Pool, int Card)
    {
        return CardDB[Pool][Card].BrickCost;
    }

    int D_getGemCost(int Pool, int Card)
    {
        return CardDB[Pool][Card].GemCost;
    }

    int D_getRecruitCost(int Pool, int Card)
    {
        return CardDB[Pool][Card].RecruitCost;
    }

    int D_getCursed(int Pool, int Card) //GE: No, I'm not telling you to get cursed!
    {
        //writeln("Cursed: ", cast(int)CardDB[Pool][Card].Cursed);
	return cast(int)CardDB[Pool][Card].Cursed;
    }

    immutable(char)* D_getPictureFile(int Pool, int Card)
    {
        return toStringz(CardDB[Pool][Card].Picture.File);
    }

    size_t D_getPictureFileSize(int Pool, int Card)
    {
        //writeln("Getting ur pic size for Pool ", Pool, " Card ", Card);
        return CardDB[Pool][Card].Picture.File.length+1;
    }

    SDL_Rect D_getPictureCoords(int Pool, int Card)
    {
        //writeln("Getting ur pic coords for Pool ", Pool, " Card ", Card);
        //writeln("We have CardDB.pool? ", CardDB[Pool].length);
        //writeln("We have Picture? ", CardDB[Pool][Card].Picture.sizeof);
        //writeln("We have Coordinates? ", CardDB[Pool][Card].Picture.Coordinates.sizeof);
        //writeln("Picture: ", CardDB[Pool][Card].Picture);
        //writeln("One coordinate is: ", CardDB[Pool][Card].Picture.Coordinates.x);
        //writeln("Returning ", CardDB[Pool][Card].Picture.Coordinates.x, ":", CardDB[Pool][Card].Picture.Coordinates.y, "; ", CardDB[Pool][Card].Picture.Coordinates.w, ":", CardDB[Pool][Card].Picture.Coordinates.h);
        return CardDB[Pool][Card].Picture.Coordinates;
    }

    int D_getPictureCoordX(int Pool, int Card)
    {
	writeln("Warning: using a workaround for sharing coordinates for Pool ", Pool, " Card ", Card);
	//writeln("On this D platform, int size is ", int.sizeof);
	//writeln("On this D platform, short size is ", short.sizeof);
	return cast(int).CardDB[Pool][Card].Picture.Coordinates.x;
    }

    int D_getPictureCoordY(int Pool, int Card)
    {
	writeln("Warning: using a workaround for sharing coordinates for Pool ", Pool, " Card ", Card);
	return cast(int).CardDB[Pool][Card].Picture.Coordinates.y;
    }

    int D_getPictureCoordW(int Pool, int Card)
    {
	writeln("Warning: using a workaround for sharing coordinates for Pool ", Pool, " Card ", Card);
	return cast(int).CardDB[Pool][Card].Picture.Coordinates.w;
    }

    int D_getPictureCoordH(int Pool, int Card)
    {
	writeln("Warning: using a workaround for sharing coordinates for Pool ", Pool, " Card ", Card);
	return cast(int).CardDB[Pool][Card].Picture.Coordinates.h;
    }

    /*void D_getPrecachePictures()
    {
        bool bAlreadyListed;
        
        for (int Pool=0; Pool < CardDB.length; Pool++)
        {
            for (int Card=0; Card < CardDB[Pool].length; Card++)
            {
                //GE: See if we have it listed already and filter out empty entries (it shouldn't happen!).
                for (int i=0; i < PrecachePictures.length; i++)
                {
                    if (PrecachePictures[i] == CardDB[Pool][Card].Picture.File || CardDB[Pool][Card].Picture.File == "")
                        bAlreadyListed = True;
                }
                if (!bAlreadyListed)
                {
                    PrecachePictures.length += 1;
                    PrecachePictures[PrecachePictures.length] = CardDB[Pool][Card].Picture.File;
                }
                bAlreadyListed = False;
            }
        }
    } */

    void D_printCardDB()
    {
	  writeln("Warning: Debugging is activated.");
      /*for (int Pool=0; Pool < CardDB.length; Pool++)
        {
            for (int Card=0; Card < CardDB[Pool].length; Card++)
            {
                writeln("CardDB[", Pool, "][", Card, "] = ", CardDB[Pool][Card]);
            }
        }*/
    }
