import std.stdio;
import std.string;
import arco;
import cards;

int main()
{
    int SelectedCard;
    string Discarding;
    bool CanPlayCard;
    
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
    initGame(); //GE: Start a local player vs bot game. Later on will have more options.
    Player[GetEnemy()].AI = true; //GE: Put a bot in there.
    
    do
    {
        writeln("Player no. ", Turn, " has these cards in hand:"); //GE: Print all the info about the game.
        foreach(int i, CardInfo CI; Player[Turn].Hand)
        {
            writeln(i, ": ", CI.Name, ":");
            writeln("------------");
            writeln(CI.Description);
            writeln("------------");
            writefln("%s, %s/%s/%s", CI.Colour, CI.BrickCost, CI.GemCost, CI.RecruitCost);
            writeln("============");
        }
        writeln("The player has the following resources:");
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
            if (!CanAffordCard(Player[Turn].Hand[SelectedCard]))
            {
                writeln("This card is too expensive!");
                continue;
            }
            CanPlayCard = PlayCard(SelectedCard, stringToBool(Discarding));
            if (!CanPlayCard)
                writeln("The card is cursed!");
        }
        else
            AIPlay();
        if (IsVictorious(Turn) || IsVictorious(GetEnemy()))
        {
            writeln("Player ", Turn, "is victorious!");
            break;
        }
        writeln("Now the turn is ", Turn);
        readln();
    } while (Discarding != "quit");
    writeln("We didn't crash!");
    readln();
    return 0;
} 

bool stringToBool(string Source)
{
    if (Source == "Yes" || Source == "yes" || Source == "True" || Source == "true" || Source == "1")
        return true;
    return false;
}
