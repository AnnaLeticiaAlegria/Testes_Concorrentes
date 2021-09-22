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

local grammarParser = require "grammarParser"

local graph = {}
local currentEvent = {}
local threadIdTable = {}
local globalGraphNode = 1

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

  local fileString = file:read("*all")

  tree = grammarParser.grammarParser(fileString)

  file:close()

  if not tree then
    return 0
  end

  createGraph(tree)
  return 1
end

local function insertEdge (entryNode, eventName, exitNode, eventRule)
  if (not graph[entryNode]) then
    graph[entryNode] = {}
  end
  if (not graph[entryNode][eventName]) then
    graph[entryNode][eventName] = {{exitNode}, eventRule}
  else
    table.insert(graph[entryNode][eventName][1], exitNode)
  end
end

local function processTree (currentGraphNode, grammarTree)
  local child1, child2
  if (grammarTree["tag"] == "item") then
    return {{grammarTree[1], grammarTree[2], currentGraphNode}}
  else
    if(grammarTree["tag"] == "or") then
      local auxTable = {}
      child1 = processTree (currentGraphNode, grammarTree[1])
      for _,value in ipairs(child1) do table.insert(auxTable,value) end

      child2 = processTree (currentGraphNode, grammarTree[2])
      for _,value in ipairs(child2) do table.insert(auxTable,value) end

      return auxTable
    else
      if(grammarTree["tag"] == "star") then
        child1 = processTree (currentGraphNode, grammarTree[1])
        for _, value in ipairs(child1) do
          insertEdge (value[3], value[1], currentGraphNode, value[2])
        end
      else
        if(grammarTree["tag"] == "seq") then
          child1 = processTree (currentGraphNode, grammarTree[1])
          globalGraphNode = globalGraphNode + 1
          for _,value in ipairs(child1) do 
            insertEdge (value[3], value[1], globalGraphNode, value[2])
          end

          child2 = processTree (globalGraphNode, grammarTree[2])
          return child2
        else
          print("Error creating graph --> Inexistent tag")
        end
      end
    end
  end
end

function createGraph (grammarTree)

  local lastChild = processTree(1, grammarTree)

  if (lastChild) then
    for _,value in ipairs(lastChild) do 
      insertEdge (value[3], value[1], globalGraphNode, value[2])
    end
  end

  for k,value in ipairs(graph) do
    print("Node "..tostring(k))
    for k2,value2 in pairs(value) do
      print("--- "..k2)
      for _,value3 in pairs(value2) do
        for _,value4 in ipairs(value3) do
          print("------ "..tostring(value4))
        end
      end
    end
  end


  -- graph[1]["WriterWantsToStart"] = {{2} , "t1"}
  -- graph[1]["ReaderWantsToStart"] = {{5}, "t2"}
  -- graph[2]["WriterStarts"] = {{3,4}, "t1"}
  -- graph[3]["WriterWrites"] = {{4}, "t1"}
  -- graph[4]["WriterEnds"] = {{1}, "t1"}
  -- graph[5]["ReaderStarts"] = {{6,7}, "t2"}
  -- graph[6]["ReaderReads"] = {{7}, "t2"}
  -- graph[7]["ReaderEnds"] = {{1}, "t2"}

  table.insert(currentEvent, 1)
end

function checkThreadId (threadVariable, threadId)
  if (threadIdTable[threadVariable]) then
    if (threadId == threadIdTable[threadVariable]) then return 1 else return 0 end
  else
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
    print("Evento realizado: " .. eventName)
    currentEvent = nextNodeTable
    return 1
  else
    return 0
  end
end

readStates("../GraphTests/RW_statesFile2.txt")

print("Chamada 1 -> "..tostring(checkEvent("WriterWantsToStart", 123)))
print("Chamada 2 -> "..tostring(checkEvent("WriterStarts", 123)))
print("Chamada 3 -> "..tostring(checkEvent("WriterWrites", 123)))
print("Chamada 4 -> "..tostring(checkEvent("WriterEnds", 123)))
print("Chamada 5 -> "..tostring(checkEvent("ReaderWantsToStart", 456)))
print("Chamada 6 -> "..tostring(checkEvent("ReaderStarts", 456)))
print("Chamada 7 -> "..tostring(checkEvent("ReaderReads", 456)))
print("Chamada 8 -> "..tostring(checkEvent("ReaderEnds", 456)))
print("Testando loops")

for i = 1, 4 do
  print("Chamada 1 loop i=" ..tostring(i).." -> "..tostring(checkEvent("WriterWantsToStart", 123)))
  print("Chamada 2 loop i=" ..tostring(i).." -> "..tostring(checkEvent("WriterStarts", 123)))
  print("Chamada 3 loop i=" ..tostring(i).." -> "..tostring(checkEvent("WriterWrites", 123)))
  print("Chamada 4 loop i=" ..tostring(i).." -> "..tostring(checkEvent("WriterEnds", 123)))
  print("Chamada 3 -> "..tostring(checkEvent("WriterWrites", 123)))
end
for i = 1, 4 do
  print("Chamada 1 loop i=" ..tostring(i).." -> "..tostring(checkEvent("ReaderWantsToStart", 456)))
  print("Chamada 2 loop i=" ..tostring(i).." -> "..tostring(checkEvent("ReaderStarts", 456)))
  print("Chamada 3 loop i=" ..tostring(i).." -> "..tostring(checkEvent("ReaderReads", 456)))
  print("Chamada 4 loop i=" ..tostring(i).." -> "..tostring(checkEvent("ReaderEnds", 456)))
end