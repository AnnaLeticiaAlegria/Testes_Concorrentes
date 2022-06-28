--
-- Module: luaMain.lua
-- Author: Anna Leticia Alegria
-- Last Modified at: 24/11/2021

----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
-- Description: This module contains the Lua functions. It is a function called by module eventManager.c that
-- reads the statesFile and uses LPeg library to treat each condition. It returns the allocated and filled array
-- stateNameArray, stateIdArray and their length (which are equal).
----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
--
package.path = package.path .. ';../EventManager/?.lua;'

local grammarParser = require ("grammarParser")
local graphManager = require ("graphManager")
local threadIdManager = require("threadIdManager")

local graph
local currentEvent
local eventsNamesTable
local ignoredEventsNames

local configurationOn = 0

----------------------------------------------------------------------------------------------------------------------
-- Function: readEventFile
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

function readConfigFile (configFilePath)
  ignoredEventsNames = {}
  eventsNamesTable = {}

  configurationOn = 1

  local file = io.open(configFilePath, "r")
  if not file then 
    return 0
  end

  for line in file:lines() do
    if (string.find(line, "//")) then
      local aux = string.gsub(line, "//","")
      table.insert(ignoredEventsNames, aux)
    else
      table.insert(eventsNamesTable, line)
    end
  end

  return 1
end

function readEventsFile(path)

  local file = io.open(path, "r")
  if not file then 
    return 0 
  end

  local fileString = file:read("*all")

  tree = grammarParser.grammarParser(fileString)

  file:close()

  if not tree then
    return 0
  end

  graph, currentEvent = graphManager.createGraph(tree, eventsNamesTable)

  return 1
end

function existsInTable (searchedTable, eventName)
  if (configurationOn == 0) then
    return 1
  end

  for _, value in ipairs(searchedTable) do
    if (eventName == value) then
      return 1
    end
  end

  return 0
end


function checkEvent (eventName, threadId)

  -- print(eventName, threadId)
  local nextNodeTable = {}

  if (configurationOn == 1 and existsInTable(ignoredEventsNames, eventName) == 1) then
    print(eventName .. " --> Ignored")
    return 1
  end

  for eventIndex, eventValue in ipairs(currentEvent) do
    if (eventValue >= 0) then -- eventValue = -1 when this event can be the last
      local currentNodeTable = graph[eventValue][eventName]
      if (existsInTable(eventsNamesTable, eventName) == 1) then
        -- print("evento: ".. eventName .. " threadId: ".. threadId)
        if (currentNodeTable and threadIdManager.checkThreadId(currentNodeTable[2], threadId) == 1) then
          for _, value in pairs(currentNodeTable[1]) do 
            table.insert(nextNodeTable, value)
          end
        end
      else
        print("Error during execution of event " .. eventName .. " --> Doesn't exist")
        os.exit()
      end
    end
  end

  if (next(nextNodeTable)) then
    print(eventName .. " --> Executed")
    currentEvent = nextNodeTable
    return 1
  else
    return 0
  end
end

function expectedEvent ()
  local auxTable = {}
  local code = 0

  graphManager.printGraph(graph)

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

-- print("\n\n\n\n\nTeste1\n\n\n")

-- readEventsFile("../Broadcast/EventsFiles/eventOrderFile_1.txt")

-- -- graphManager.printGraph(graph)

-- print(checkEvent ("ProducerWantsToStart", 1))
-- print(checkEvent ("ProducerWantsToStart", 2))
-- print(checkEvent ("ProducerStarts", 2))
-- print(checkEvent ("ProducerStarts", 1))
-- print(checkEvent ("ProducerWantsToStart", 2))
-- print(checkEvent ("ProducerStarts", 2))
-- print(checkEvent ("ConsumerWantsToStart", 4))
-- print(checkEvent ("ProducerWantsToStart", 3))
-- print(checkEvent ("ProducerStarts", 3))
-- print(checkEvent ("ProducerEnds", 3))
-- print(checkEvent ("ProducerEnds", 2))
-- print(checkEvent ("ProducerEnds", 1))
-- print(checkEvent ("ProducerEnds", 3))

-- readEventsFile("../GraphTests/threadIdTest1.txt")

-- checkEvent ("Event1", 1)
-- checkEvent ("Event2", 1)
-- checkEvent ("Event3", 1)
-- checkEvent ("Event4", 2)
-- checkEvent ("Event5", 2)
-- checkEvent ("Event6", 3)
-- checkEvent ("Event7", 1)
-- checkEvent ("Event8", 2)
-- checkEvent ("Event9", 1)
-- checkEvent ("Event9", 2)

-- readEventsFile("../EventManager/test.txt")

-- graphManager.printGraph(graph)

-- readEventsFile("../ProducerConsumer/EventsFiles/eventOrder5.txt")

-- graphManager.printGraph(graph)

-- readEventsFile("../EventManager/test_threadId.txt")

-- checkEvent ("WorkerArrives", 1)
-- checkEvent ("CoordinatorPosted", 3)
-- checkEvent ("WorkerWaits", 1)
-- checkEvent ("WorkerArrives", 1)
-- checkEvent ("WorkerWaits", 1)

-- readEventsFile("../EventManager/test_or_plus.txt")
-- readEventsFile("../ReaderWriter/EventsFiles/infinite.txt")

readEventsFile("../EventManager/test_seq_plus.txt")
graphManager.printGraph(graph)