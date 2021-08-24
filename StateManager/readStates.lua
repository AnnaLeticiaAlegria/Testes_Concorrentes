--
-- Module: readStates.lua
-- Author: Anna Leticia Alegria
-- Last Modified at: 17/06/2021

----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
-- Description: This module contains the Lua function readStates. It is a function called by module stateManager.c that
-- reads the statesFile and uses LPeg library to treat each condition. It returns the allocated and filled array
-- stateNameArray, stateIdArray and their length (which are equal).
----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
--
local lpeg = require 'lpeg'
local match = lpeg.match

local graph = {}
local currentEvent = {}
local threadIdTable = {}

----------------------------------------------------------------------------------------------------------------------
-- Function: readStates
-- Parameters: 
--   -> path: the path of the statesFile.txt file as a string
-- Returns: 
--   -> stateIdArray: a table with each key being the sequential number in range (1,#stateNameArray) and value being
-- the state's id. This table is ordered accordingly to the statesFile. Each id corresponds to the name at the same
-- position in stateNameArray
--   -> stateNameArray: a table with each key being the sequential number in range (1,#stateNameArray) and value being
-- the state's name. This table is ordered accordingly to the statesFile
--   -> #stateNameArray: the length of stateNameArray
--
-- Description: This function opens the file at the given path and puts each event at a sequential position of the array.
-- It also uses LPeg to treat the state's id. To understand what each id's means, see documentation.
--
-- Input assertions: 
--   -> The file at the given path must be written follow the documentation rules
--
-- Output assertions:
--   -> If the file cannot be opened, this function returns nil
--   -> In case of success, this function returns the values in the following order: stateIdArray, stateNameArray, 
-- #stateNameArray
----------------------------------------------------------------------------------------------------------------------

function readStates(path)
  local file = io.open(path, "r")
  if not file then 
    return nil 
  end

  -- Iniatilize stateNameArray and stateIdArray as empty tables
  local stateNameArray = {}
  local stateIdArray = {}

  -- For each line of the file do
  for line in file:lines() do
    local space = lpeg.P(' ') -- Detects whitespaces
    local notSpace = (1 - space)^0 -- Detects everything that comes before a whitespace 
    local beforeSpace = lpeg.C(notSpace) * space -- Captures everything that comes before a whitespace
    local afterSpace = notSpace * space * lpeg.C(notSpace) -- Captures everything that comes after a whitespace

    local stateName = beforeSpace:match(line) -- Aplies beforeSpace on the string line
    table.insert (stateNameArray, stateName) -- Insert all characters before whitespace at the next position of stateNameArray

    local stateIdAux = afterSpace:match(line) -- Aplies afterSpace on the string line
    local stateId
    local anyId = lpeg.P("*") -- Detects the character '*'
    local notId = lpeg.P("!") -- Detects the character '!'

    if anyId:match(stateIdAux) then -- If stateIdAux contains a '*'
      stateId = 0 -- It means it can be any thread id
    else
      if notId:match(stateIdAux) then -- If stateIdAux contains a '!'
        local pattern = notId * lpeg.C((1 - notId)^0) -- Captures the number that follows the character '!'
        stateId = tonumber(pattern:match(stateIdAux)) * (-1) -- Convert this number to a negative integer
      else
        stateId = tonumber(stateIdAux) -- Convert stateIdAux to a positive integer (Since it contains only number)
      end
    end
    table.insert (stateIdArray, stateId)
  end
  file:close()

  createGraph ()
  return stateIdArray, stateNameArray, #stateNameArray
end

function createGraph ()
  graph = {}
  for i = 1,6 do
    graph[i] = {}
  end

  graph[1]["ThreadStarts"] = {{2} , "t1"}
  graph[2]["ReadCount"] = {{3}, "t1"}
  graph[3]["ThreadStarts"] = {{4}, "t2"}
  graph[4]["ReadCount"] = {{5}, "t2"}
  graph[5]["UpdateCount"] = {{6}, "t1"}
  graph[6]["UpdateCount"] = {{}, "t2"}

  table.insert(currentEvent, 1)
end

function checkThreadId (threadVariable, threadId)
  if (threadIdTable[threadVariable]) then
    print("threadId existe " .. tostring(threadVariable) .. " " .. tostring(threadIdTable[threadVariable]))
    if (threadId == threadIdTable[threadVariable]) then return 1 else return 0 end
  else
    print("threadId nova " .. tostring(threadVariable) .. " " .. tostring(threadIdTable[threadVariable]))
    threadIdTable[threadVariable] = threadId
    return 1
  end
end

function checkEvent (eventName, threadId)
  print("Processando evento " .. eventName)

  local nextNodeTable = {}

  for eventIndex, eventValue in ipairs(currentEvent) do
    local currentNodeTable = graph[eventValue][eventName]
    if (currentNodeTable and checkThreadId(currentNodeTable[2], threadId)) then
      for _, value in ipairs(currentNodeTable[1]) do 
        table.insert(nextNodeTable, value)
      end
    end
  end

  if (next(nextNodeTable)) then
    currentEvent = nextNodeTable
    return 1
  else
    return 0
  end
end

createGraph()
print("Chamada 1 -> "..tostring(checkEvent("ThreadStarts", 123)))
print("Chamada 2 -> "..tostring(checkEvent("ThreadStarts", 456)))
print("Chamada 3 -> "..tostring(checkEvent("ReadCount", 123)))
print("Chamada 4 -> "..tostring(checkEvent("ThreadStarts", 456)))
print("Chamada 5 -> "..tostring(checkEvent("ReadCount", 456)))
print("Chamada 6 -> "..tostring(checkEvent("UpdateCount", 123)))
print("Chamada 7 -> "..tostring(checkEvent("UpdateCount", 456)))