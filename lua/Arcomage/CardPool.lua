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


CardDB = {} --GE: This is the CardDB, thing that we'll send over to C/D

for CardID = 0,101 do --GE: We have 102 cards, IDs from 0-101. NOTE: Tables start at 1, not 0, so add one before adding to CardDB!
   CardInfo = {ID = 0, Frequency = 1, Name = "", Description = "", BrickCost = 0, GemCost = 0, RecruitCost = 0, Picture = "", Keywords = "", LuaFunction = ""} --GE: Make sure we reset CardInfo each loop.
   --GE: I will ASSUMING DIRECT CONTROL-- Err, I mean, I will assume that CardDB[CardID+1]=CardInfo means copying and not pointing at
   --GE: Also, Frequency is init as 1 because it's the most common in Arcomage, for your mod you might want to use 3
   if CardID == 0 then do
      CardInfo.Name = "Brick Shortage"
      CardInfo.Description = "All players\nlose 8 bricks"
   elseif CardID == 1 then do
      CardInfo.Name = "Lucky Cache"
      CardInfo.Description = "+2 Bricks\n+2 Gems\nPlay again"
   elseif CardID == 2 then do
      CardInfo.Name = "Friendly Terrain"
      CardInfo.Description = "+1 Wall\nPlay again"
      CardInfo.BrickCost = 1
   end
end