--
-- Module: eventManager.lua
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

-- local grammarParser = require ("grammarParser")

local graph
local currentEvent
local threadIdTable
local globalGraphNode







--[[
Item <- Id ('[' TheadExp ']' )? | '(' Exp ')'

Iterate <- Item ('*' ?)

Seq <- Iterate (';' Iterate)* ';'?

Exp <- Seq ('|' Seq)*
]]


local m = require "lpeg"


local function syntaxerror (s, i)
  local eols = string.gsub(string.sub(s, 1, i), "[^\n]", "")
  io.stdout:write("syntax error line ", #eols + 1, ": ",
   string.sub(s, i - 30, i - 1), "❌", string.sub(s, i, i + 30), "\n")
  os.exit(false)
end


local function packbin (tag)
  return function (a, b)
           return b and {tag = tag, a, b} or a
         end
end


local function packstar (a, b)
  return b and {tag = "star", a} or a
end


local inlimit = 0

-- Tokens
local S = m.V"S"

local OS = '[' * S
local CS = ']' * S

local OP = '(' * S
local CP = ')' * S

local OrOp = '|' * S
local Star = '*' * S
local Sc = ';' * S

local ID = m.C(m.R("az", "AZ") * m.R("az", "AZ", "09")^0) * S --Captura o nome do ID

local G = m.P{"Prog";

  Prog = S * m.V"Exp" * -m.P(1),

  Exp = (m.V"Seq" * Sc^-1 * (OrOp * m.V"Exp")^-1) / packbin("or"),

  Seq = (m.V"Iterate" * (Sc * m.V"Seq")^-1) / packbin("seq"),

  Iterate = (m.V"Item" * m.C(Star)^-1) / packstar,

  Item = (ID * m.C((OS * m.V"ThreadExp" * CS)^-1)) / packbin("item")
       + OP * m.V"Exp" * CP,

  ThreadExp = (1 - m.P"]")^0, -- Tudo antes do "]"

  S = m.S(" \t\n")^0 *
        m.P(function (_, i)
             -- track input limit
             inlimit = math.max(inlimit, i)
             return i
           end),

}


local function grammarParser (input)
  local p = m.match(G, input)
  if not p then
    syntaxerror(input, inlimit)
  end
  return p
end


-- print(pt.pt(parser(io.read("a"))))




local function pt (x, id)
  id = id or ""
  if type(x) == "string" then return "'" .. tostring(x) .. "'"
  elseif type(x) ~= "table" then return tostring(x)
  else
    local s = id .. "{\n"
    for k,v in pairs(x) do
      s = s .. id .. tostring(k) .. " = " .. pt(v, id .. "  ") .. ";\n"
    end
    s = s .. id .. "}"
    return s
  end
end








function initializeGlobalVariables ()
  graph = {}
  currentEvent = {}
  threadIdTable = {}
  globalGraphNode = 1
end

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

function readStatesFile(path)
  initializeGlobalVariables()

  local file = io.open(path, "r")
  if not file then 
    return 0 
  end

  local fileString = file:read("*all")

  -- tree = grammarParser.grammarParser(fileString)
  tree = grammarParser(fileString)

  file:close()

  if not tree then
    return 0
  end

  createGraph(tree)
  return 1
end

local function existsInTable (searchedTable, element)
  for _, value in ipairs(searchedTable) do
    if (value == element) then
      return 1
    end
  end

  return 0
end

local function insertEdge (entryNode, eventName, exitNode, eventRule)
  if (not graph[entryNode]) then
    graph[entryNode] = {}
  end
  if (not graph[entryNode][eventName]) then
    graph[entryNode][eventName] = {{exitNode}, eventRule}
  else
    if (~existsInTable(graph[entryNode][eventName][1], exitNode)) then
      table.insert(graph[entryNode][eventName][1], exitNode)
    end
  end
end

local function actionTagOr (currentGraphNode, child1, child2)
  local auxTable = {}

  for _,value in ipairs(child1) do table.insert(auxTable,value) end
  
  for _,value in ipairs(child2) do table.insert(auxTable,value) end

  return auxTable
end

local function actionTagSeq (currentGraphNode, child1)
  globalGraphNode = globalGraphNode + 1
  for _,value in ipairs(child1) do 
    insertEdge (value[3], value[1], globalGraphNode, value[2])
  end
end

function deepcopy(orig)
  local orig_type = type(orig)
  local copy
  if orig_type == 'table' then
    copy = {}
    for orig_key, orig_value in next, orig, nil do
      copy[deepcopy(orig_key)] = deepcopy(orig_value)
    end
    setmetatable(copy, deepcopy(getmetatable(orig)))
  else -- number, string, boolean, etc
    copy = orig
  end
  return copy
end

local function actionTagStar (currentGraphNode, child1, starCaseChild, hasFatherOr)

  if(hasFatherOr) then

    globalGraphNode = globalGraphNode + 1

    for _, value in ipairs(child1) do
      insertEdge (value[3], value[1], globalGraphNode, value[2])
    end

    if (not starCaseChild) then
      local auxChild = deepcopy(child1)

      for _, starValue in ipairs(child1) do
        for _, childValue2 in ipairs(child1) do
          insertEdge (globalGraphNode, starValue[1], globalGraphNode, starValue[2])
          childValue2[3] = globalGraphNode
        end
      end
      for _, value in ipairs(auxChild) do
        table.insert(child1, value)
      end
      -- for _, value in ipairs
    else
      for _, starValue in ipairs(starCaseChild) do
        for _, childValue2 in ipairs(child1) do
          insertEdge (globalGraphNode, starValue[1], childValue2[3], starValue[2])
        end
      end
    end
  else
    for _, value in ipairs(child1) do
      insertEdge (value[3], value[1], currentGraphNode, value[2])
    end
  end

  return child1
end
  
local function processTree (currentGraphNode, grammarTree, hasFatherOr)
  local child1, child2
  if (grammarTree["tag"] == "item") then
    return {{grammarTree[1], grammarTree[2], currentGraphNode}}
  else
    if(grammarTree["tag"] == "or") then
      child1, _ = processTree (currentGraphNode, grammarTree[1], 1)
      child2, _ = processTree (currentGraphNode, grammarTree[2], 1)

      return actionTagOr (currentGraphNode, child1, child2), child1
    else
      if(grammarTree["tag"] == "star") then
        local starCaseChild
        child1, starCaseChild = processTree (currentGraphNode, grammarTree[1], hasFatherOr)
        actionTagStar (currentGraphNode, child1, starCaseChild, hasFatherOr)

        return child1
      else
        if(grammarTree["tag"] == "seq") then
          child1, _ = processTree (currentGraphNode, grammarTree[1], hasFatherOr)

          actionTagSeq (currentGraphNode, child1)

          child2, _ = processTree (globalGraphNode, grammarTree[2], hasFatherOr)

          return child2, child1
        else
          print("Error creating graph --> Inexistent tag")
        end
      end
    end
  end
end

local function printGraph(graph)

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
end

function createGraph (grammarTree)

  local lastChild, _ = processTree(1, grammarTree, nil)

  if (lastChild) then
    for _,value in ipairs(lastChild) do 
      insertEdge (value[3], value[1], -1, value[2])
    end
  end

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

  local nextNodeTable = {}

  for eventIndex, eventValue in ipairs(currentEvent) do
    if (eventValue >= 0) then -- eventValue = -1 when this event can be the last
      local currentNodeTable = graph[eventValue][eventName]
      if (currentNodeTable and checkThreadId(currentNodeTable[2], threadId)) then
        for _, value in pairs(currentNodeTable[1]) do 
          table.insert(nextNodeTable, value)
        end
      end
    end
  end

  if (next(nextNodeTable)) then
    -- print("Evento realizado: " .. eventName)
    currentEvent = nextNodeTable
    return 1
  else
    return 0
  end
end

function expectedEvent ()
  local auxTable = {}
  local code = 0

  printGraph(graph)

  for _, stateValue in ipairs(currentEvent) do
    if (stateValue >= 0) then
      for eventName, _ in pairs(graph[stateValue]) do
        table.insert(auxTable, eventName)
      end
    else
      table.insert(auxTable, "[This could be the last event]")
      code = 1 -- indicates that the current event could be the last in the script
    end
  end

  if (code == 1 and #auxTable > 1) then -- the current event could lead to another event, so it wasnt necessarily the last in the script
    code = 0
  end 

  return auxTable, #auxTable, code
end

print("\n\n\n\n\nTeste1\n\n\n")
readStatesFile("../ProducerConsumerPassingTheBaton/Tests/test.txt")

printGraph(graph)

print("\n\n\n\n\nTeste2\n\n\n")

readStatesFile("../ProducerConsumerPassingTheBaton/Tests/test2.txt")

printGraph(graph)