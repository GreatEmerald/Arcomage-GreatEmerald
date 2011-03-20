-- Arcomage/CardPool
-- The Card Pool for the stock Arcomage deck.
-- GreatEmerald, 2011

-- GE: begin algorithm:
-- A Card Pool is a file that provides all of the information about individual
-- cards in a mod. For instance, this file contains all of the information about
-- the names and the frequency of the cards. This information is taken by the C
-- code that uses it to setup the cards for first use.
-- In short: It takes and fills the CardDB array in cards.h.

-- This is how communication works:
-- C has CardPool[] -> CardDB[] -> CardInfo{} -> ID, Frequency, Name
-- This is a single pool, so C/D is handling the CardPool[] part. But we need
-- to setup CardDB[]. All of the entries are unique, so there won't be much of
-- a problem.
-- Now Lua has these things called tables. They are something like a mix of
-- arrays and structs. I believe that I can create a global table that holds
-- the same as CardDB[], then transmit it through the stack. The stack will be
-- humongous, but it will only take a single element! Parsing it should only
-- take a single loop, too, and I can do it in D!

function ArcomageInit()

  local CardInfo = {ID = 0, Frequency = 1, Name = "", Description = "", BrickCost = 0, GemCost = 0, RecruitCost = 0, Colour = "Black", Picture = "", Keywords = "", LuaFunction = ""}
  local CardDB = {} --GE: This is the CardDB, thing that we'll send over to C/D
  local Iteration = 0
  
  --[[local Test = {{x = "a", y = "b"}, {"c", "d"}}
  print("Test is:")
  print(Test)
  print("Test[1] is:")
  print(Test[1])
  print("Test[1].x is:")
  print(Test[1].x) ]]
  
  for CardID = 0,5 do--101 do --GE: We have 102 cards, IDs from 0-101. NOTE: Tables start at 1, not 0, so add one before adding to CardDB!
     Iteration = Iteration + 1
     --CardDB[Iteration] = CardInfo
     CardDB[Iteration] = {}
     CardInfo = {ID = CardID+1, Frequency = 1, Name = "", Description = "", BrickCost = 0, GemCost = 0, RecruitCost = 0, Colour = "Black", Picture = "", Keywords = "", LuaFunction = ""}
     --[[CardInfo.ID = CardID --GE: Make sure we reset CardInfo each loop.
     CardInfo.Frequency = 1
     CardInfo.Name = ""
     CardInfo.Description = ""
     CardInfo.BrickCost = 0
     CardInfo.GemCost = 0
     CardInfo.RecruitCost = 0
     CardInfo.Colour = "Black"
     CardInfo.Picture = ""
     CardInfo.Keywords = ""
     CardInfo.LuaFunction = ""]]
     
     --GE: Frequency is init as 1 because it's the most common in Arcomage, for your mod you might want to use 3
     if CardID == 0 then
        CardInfo.Name = "Brick Shortage"
        CardInfo.Description = "All players\nlose 8 bricks"
     elseif CardID == 1 then
        CardInfo.Name = "Lucky Cache"
        CardInfo.Description = "+2 Bricks\n+2 Gems\nPlay again"
     elseif CardID == 2 then
        CardInfo.Name = "Friendly Terrain"
        CardInfo.Description = "+1 Wall\nPlay again"
        CardInfo.BrickCost = 1
     elseif CardID == 3 then
        CardInfo.Name = "Miners"
        CardInfo.Description = "+1 Quarry"
        CardInfo.BrickCost = 3
     elseif CardID == 4 then
        CardInfo.Name = "Mother Lode"
        CardInfo.Description = "If quarry<enemy\nquarry, +2 quarry\nElse, +1\nquarry"
        CardInfo.BrickCost = 4
     elseif CardID == 5 then
        CardInfo.Name = "Dwarven Miners"
        CardInfo.Description = "+4 Wall\n+1 Quarry"
        CardInfo.BrickCost = 7
     end
     
     CardDB[Iteration].ID = CardInfo.ID
     CardDB[Iteration].Frequency = CardInfo.Frequency
     CardDB[Iteration].Name = CardInfo.Name
     CardDB[Iteration].Description = CardInfo.Description
     CardDB[Iteration].BrickCost = CardInfo.GemCost
     CardDB[Iteration].GemCost = CardInfo.GemCost
     CardDB[Iteration].RecruitCost = CardInfo.RecruitCost
     CardDB[Iteration].Colour = CardInfo.Colour
     CardDB[Iteration].Picture = CardInfo.Picture
     CardDB[Iteration].Keywords = CardInfo.Keywords
     CardDB[Iteration].LuaFunction = CardInfo.LuaFunction
     print(CardDB[1]["Name"])
  end
  
  for i,v in ipairs(CardDB) do print(i,v) end
  
  --[[{ID = 0,
  Frequency = 1,
  Name = "ohai",
  Description = "",
  BrickCost = 0,
  GemCost = 0,
  RecruitCost = 0,
  Picture = "",
  Keywords = "",
  LuaFunction = "",
  Colour = "Black"},]]
  
  -- GE: Start CardDB information generation. Unfortunately, everything is a pointer in Lua, thus making a loop is not going to work.
  --[[CardDB = {
  
  {ID = 0,
  Frequency = 1,
  Name = "Brick Shortage",
  Description = "All players\nlose 8 bricks",
  BrickCost = 0,
  GemCost = 0,
  RecruitCost = 0,
  Picture = "",
  Keywords = "",
  LuaFunction = "",
  Colour = "Red"},
  
  {ID = 1,
  Frequency = 1,
  Name = "Lucky Cache",
  Description = "+2 Bricks\n+2 Gems\nPlay again",
  BrickCost = 0,
  GemCost = 0,
  RecruitCost = 0,
  Picture = "",
  Keywords = "",
  LuaFunction = "",
  Colour = "Red"},
  
  {ID = 2,
  Frequency = 1,
  Name = "Friendly Terrain",
  Description = "+1 Wall\nPlay again",
  BrickCost = 0,
  GemCost = 0,
  RecruitCost = 0,
  Picture = "",
  Keywords = "",
  LuaFunction = "",
  Colour = "Black"},
  
  } ]]
  
  --print(CardDB[1].Name)
  return CardDB
end