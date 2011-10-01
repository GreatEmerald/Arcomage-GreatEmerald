import std.stdio;
import std.string;
import arco;
import cards;

int main()
{
    int SelectedCard;
    string Discarding;
    bool CanPlayCard;
    bool Debug;
    
    InitArcomage(); //GE: We initialise the library first. Perhaps it will auto init later.
    FrontendFunctions.Sound_Play = function(SoundTypes){}; //GE: Init all the frontend functions. Frontends must do that, although I guess it could be in the library and then overridden.
    FrontendFunctions.RedrawScreenFull = function(){};
    FrontendFunctions.PrecacheCard = function(const char*, int){};
    FrontendFunctions.PlayCardAnimation = function(CardInfo CI, int Discarded)
    {
        if (!Discarded)
            writeln("Player ", Turn, " just played ", CI.Name);
        else
            writeln("Player ", Turn, " just discarded ", CI.Name);
    };
    initGame();
    
    writeln("Welcome to CLArcomage, the Command Line Arcomage! This is the release version b1, made entirely in D by GreatEmerald. Have a good game!");
    writeln("Would you like to have control of player no. 1? If not, it will be assigned to an AI.");
    readf(" ");
    Player[Turn].AI = !stringToBool(chomp(readln())); //GE: First player setup.
    writeln("Would you like to have control of player no. 2? If not, it will be assigned to an AI. If both are AIs, you will have a demo match. If both are players, you will have a hotseat game.");
    readf(" ");
    Player[GetEnemy()].AI = !stringToBool(chomp(readln())); //GE: First player setup.
    
    do
    {
        
            
        writeln("Current turn is: Player ", Turn); //GE: Print all the info about the game.
        if (!Player[Turn].AI && !Debug)
        {
            foreach(int i, CardInfo CI; Player[Turn].Hand)
            {
                writeln(i, ": ", CI.Name, ":");
                writeln("------------");
                writeln(CI.Description);
                writeln("------------");
                writefln("%s, %s/%s/%s", CI.Colour, CI.BrickCost, CI.GemCost, CI.RecruitCost);
                writeln("============");
            }
            writeln("Player ", GetEnemy(), " has the following resources:");
            writefln("%s quarry and %s bricks", Player[GetEnemy()].Quarry, Player[GetEnemy()].Bricks);
            writefln("%s magic and %s gems", Player[GetEnemy()].Magic, Player[GetEnemy()].Gems);
            writefln("%s dungeon and %s recruits", Player[GetEnemy()].Dungeon, Player[GetEnemy()].Recruits);
            writefln("%s tower and %s wall", Player[GetEnemy()].Tower, Player[GetEnemy()].Wall);
        }
        writeln("Player ", Turn, " has the following resources:");
        writefln("%s quarry and %s bricks", Player[Turn].Quarry, Player[Turn].Bricks);
        writefln("%s magic and %s gems", Player[Turn].Magic, Player[Turn].Gems);
        writefln("%s dungeon and %s recruits", Player[Turn].Dungeon, Player[Turn].Recruits);
        writefln("%s tower and %s wall", Player[Turn].Tower, Player[Turn].Wall);
        if (!Player[Turn].AI)
        {
            writeln("Select the card to play!");
            readf("%s", &SelectedCard);
            writeln("Would you like to discard this card?");
            readf(" ");//GE: Workaround of the silly D reading process.
            Discarding = chomp(readln());
            if (!stringToBool(Discarding) && !CanAffordCard(Player[Turn].Hand[SelectedCard]))
            {
                writeln("This card is too expensive!");
                readln();
                continue;
            }
            CanPlayCard = PlayCard(SelectedCard, stringToBool(Discarding));
            if (!CanPlayCard)
            {
                writeln("You are prevented from using that card! It's either cursed or you're in a discard round.");
                readln();
            }
        }
        else
            AIPlay();
        if (IsVictorious(Turn) || IsVictorious(GetEnemy()))
        {
            writeln("Player ", GetEnemy(), " is victorious!"); //GE: After PlayCard() the turn is already over
            break;
        }
        writeln("Now the turn is ", Turn);
        if (Discarding == "debug") //GE: Cheat codes yay!
            Debug = true;
        readln();
    } while (Discarding != "quit");
    writeln("The match is over! To start a new one, restart the application.");
    writeln("CLArcomage is copyright (c) GreatEmerald, 2011, under the GPL license.");
    readln();
    return 0;
} 

bool stringToBool(string Source)
{
    if (Source == "Yes" || Source == "yes" || Source == "True" || Source == "true" || Source == "1")
        return true;
    return false;
}
