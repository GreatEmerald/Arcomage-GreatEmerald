import std.stdio;
import arco;
import cards;

int main()
{
    InitArcomage(); //GE: We initialise the library first. Perhaps it will auto init later.
    FrontendFunctions.Sound_Play = function(SoundTypes){}; //GE: Init all the frontend functions. Frontends must do that, although I guess it could be in the library and then overridden.
    initGame(); //GE: Start a local player vs bot game. Later on will have more options.
    writeln("Player no. ", Turn, " has these cards in hand:"); //GE: Print all the info about the game.
    foreach(CardInfo CI; Player[Turn].Hand)
    {
        writeln(CI.Name);
        writeln(CI.Description);
        writefln("%s, %s/%s/%s", CI.Colour, CI.BrickCost, CI.GemCost, CI.RecruitCost);
    }
    writeln("The player has the following resources:");
    writefln("%s quarry and %s bricks", Player[Turn].Quarry, Player[Turn].Bricks);
    writefln("%s magic and %s gems", Player[Turn].Magic, Player[Turn].Gems);
    writefln("%s dungeon and %s recruits", Player[Turn].Dungeon, Player[Turn].Recruits);
    writefln("%s tower and %s wall", Player[Turn].Tower, Player[Turn].Wall);
    writeln("We didn't crash!");
    readln();
    return 0;
} 
