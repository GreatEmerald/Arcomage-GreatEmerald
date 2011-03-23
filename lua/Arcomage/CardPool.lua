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
  
  for CardID = 0,33 do--101 do --GE: We have 102 cards, IDs from 0-101. NOTE: Tables start at 1, not 0, so add one before adding to CardDB!
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
        CardInfo.Colour = "Red"
     elseif CardID == 1 then
        CardInfo.Name = "Lucky Cache"
        CardInfo.Description = "+2 Bricks\n+2 Gems\nPlay again"
        CardInfo.Colour = "Red"
     elseif CardID == 2 then
        CardInfo.Name = "Friendly Terrain"
        CardInfo.Description = "+1 Wall\nPlay again"
        CardInfo.BrickCost = 1
        CardInfo.Colour = "Red"
     elseif CardID == 3 then
        CardInfo.Name = "Miners"
        CardInfo.Description = "+1 Quarry"
        CardInfo.BrickCost = 3
        CardInfo.Colour = "Red"
     elseif CardID == 4 then
        CardInfo.Name = "Mother Lode"
        CardInfo.Description = "If quarry<enemy\nquarry, +2 quarry\nElse, +1\nquarry"
        CardInfo.BrickCost = 4
        CardInfo.Colour = "Red"
     elseif CardID == 5 then
        CardInfo.Name = "Dwarven Miners"
        CardInfo.Description = "+4 Wall\n+1 Quarry"
        CardInfo.BrickCost = 7
        CardInfo.Colour = "Red"
     elseif CardID == 6 then
        CardInfo.Name = "Work Overtime"
        CardInfo.Description = "+5 Wall\nYou lose 6 gems"
        CardInfo.BrickCost = 2
        CardInfo.Colour = "Red"
     elseif CardID == 7 then
        CardInfo.Name = "Copping the Tech"
        CardInfo.Description = "If quarry<enemy\nquarry, quarry =\nenemy quarry"
        CardInfo.BrickCost = 5
        CardInfo.Colour = "Red"
     elseif CardID == 8 then
        CardInfo.Name = "Basic Wall"
        CardInfo.Description = "+3 Wall"
        CardInfo.BrickCost = 2
        CardInfo.Colour = "Red"
     elseif CardID == 9 then
        CardInfo.Name = "Sturdy Wall"
        CardInfo.Description = "+4 Wall"
        CardInfo.BrickCost = 3
        CardInfo.Colour = "Red"
     elseif CardID == 10 then
        CardInfo.Name = "Innovations"
        CardInfo.Description = "+1 to all players\nquarrys, you gain\n+4 gems"
        CardInfo.BrickCost = 2
        CardInfo.Colour = "Red"
     elseif CardID == 11 then
        CardInfo.Name = "Foundations"
        CardInfo.Description = "If wall = 0, +6\nwall, else\n+3 wall"
        CardInfo.BrickCost = 3
        CardInfo.Colour = "Red"
     elseif CardID == 12 then
        CardInfo.Name = "Tremors"
        CardInfo.Description = "All walls take\n5 damage\nPlay again"
        CardInfo.BrickCost = 7
        CardInfo.Colour = "Red"
     elseif CardID == 13 then
        CardInfo.Name = "Secret Room"
        CardInfo.Description = "+1 Magicl\nPlay again"
        CardInfo.BrickCost = 8
        CardInfo.Colour = "Red"
     elseif CardID == 14 then
        CardInfo.Name = "Earthquake"
        CardInfo.Description = "-1 To all players\nquarrys"
        CardInfo.Colour = "Red"
     elseif CardID == 15 then
        CardInfo.Name = "Big Wall"
        CardInfo.Description = "+6 Wall"
        CardInfo.BrickCost = 5
        CardInfo.Colour = "Red"
     elseif CardID == 16 then
        CardInfo.Name = "Collapse!"
        CardInfo.Description = "-1 Enemy quarry"
        CardInfo.BrickCost = 4
        CardInfo.Colour = "Red"
     elseif CardID == 17 then
        CardInfo.Name = "New Equipment"
        CardInfo.Description = "+2 quarry"
        CardInfo.BrickCost = 6
        CardInfo.Colour = "Red"
     elseif CardID == 18 then
        CardInfo.Name = "Strip Mine"
        CardInfo.Description = "+1 Quarry, +10\nwall. You gain\n5 gems"
        CardInfo.Colour = "Red"
     elseif CardID == 19 then
        CardInfo.Name = "Reinforced wall"
        CardInfo.Description = "+8 Wall"
        CardInfo.BrickCost = 8
        CardInfo.Colour = "Red"
     elseif CardID == 20 then
        CardInfo.Name = "Porticulus"
        CardInfo.Description = "+6 Wall\n+1 Dungeon"
        CardInfo.BrickCost = 9
        CardInfo.Colour = "Red"
     elseif CardID == 21 then
        CardInfo.Name = "Crystal Rocks"
        CardInfo.Description = "+7 Wall\ngain 7 gems"
        CardInfo.BrickCost = 9
        CardInfo.Colour = "Red"
     elseif CardID == 22 then
        CardInfo.Name = "Harmonic Ore"
        CardInfo.Description = "+6 Wall\n+3 Tower"
        CardInfo.BrickCost = 11
        CardInfo.Colour = "Red"
     elseif CardID == 23 then
        CardInfo.Name = "Mondo Wall"
        CardInfo.Description = "+12 Wall"
        CardInfo.BrickCost = 13
        CardInfo.Colour = "Red"
     elseif CardID == 24 then
        CardInfo.Name = "Focused Designs"
        CardInfo.Description = "+8 Wall\n+5 Tower"
        CardInfo.BrickCost = 15
        CardInfo.Colour = "Red"
     elseif CardID == 25 then
        CardInfo.Name = "Great Wall"
        CardInfo.Description = "+15 Wall"
        CardInfo.BrickCost = 16
        CardInfo.Colour = "Red"
     elseif CardID == 26 then
        CardInfo.Name = "Rock Launcher"
        CardInfo.Description = "+6 Wall\n10 Damage\n to enemy"
        CardInfo.BrickCost = 18
        CardInfo.Colour = "Red"
     elseif CardID == 27 then
        CardInfo.Name = "Dragon's Heart"
        CardInfo.Description = "+20 Wall\n+8 Tower"
        CardInfo.BrickCost = 24
        CardInfo.Colour = "Red"
     elseif CardID == 28 then
        CardInfo.Name = "Forced Labor"
        CardInfo.Description = "+9 Wall\nLose 5 recruits"
        CardInfo.BrickCost = 7
        CardInfo.Colour = "Red"
     elseif CardID == 29 then --GE: Start MM8 BP cards
        CardInfo.Name = "Rock Garden"
        CardInfo.Description = "+1 Wall\n+1 Tower\n+2 recruits"
        CardInfo.BrickCost = 1
        CardInfo.Colour = "Red"
     elseif CardID == 30 then
        CardInfo.Name = "Flood Water"
        CardInfo.Description = "Player(s) w/ lowest\nWall are -1 Dung-\neon and 2 dam-\nage to Tower" --GE: YES, Dung Neon and Damnage! >:3
        CardInfo.BrickCost = 6
        CardInfo.Colour = "Red"
     elseif CardID == 31 then
        CardInfo.Name = "Barracks"
        CardInfo.Description = "+6 recruits, +6 Wall\nif dungeon <\nenemy dungeon,\n+1 dungeon"
        CardInfo.BrickCost = 10
        CardInfo.Colour = "Red"
     elseif CardID == 32 then
        CardInfo.Name = "Battlements"
        CardInfo.Description = "+7 Wall\n6 damage to\nenemy"
        CardInfo.BrickCost = 14
        CardInfo.Colour = "Red"
     elseif CardID == 33 then
        CardInfo.Name = "Shift"
        CardInfo.Description = "Switch your Wall\nwith enemy Wall"
        CardInfo.BrickCost = 17
        CardInfo.Colour = "Red"
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