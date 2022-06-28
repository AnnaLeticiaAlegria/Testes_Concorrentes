package.path = package.path .. ';../EventManager/?.lua;'

local threadIdParser = require("threadIdParser")

local graphManager = {}

local graph
local currentEvent
local globalGraphNode
local eventsNamesTable



local function deepcopy(orig)
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

local function existsInTable (searchedTable, element)
  for _, value in ipairs(searchedTable) do
    if (value == element) then
      return 1
    end
  end

  return 0
end

local function eventInEventNameList (event)
  if (eventsNamesTable) then
    if (existsInTable (eventsNamesTable, event) == 0) then
      print("Error: event " .. event .. " doesnt exist in configuration file")
      return 0
    end
  end
  
  return 1
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

local function actionTagPlus (currentGraphNode, child1, plusCaseChild, hasFatherOr)

  if(hasFatherOr) then

    globalGraphNode = globalGraphNode + 1

    for _, value in ipairs(child1) do
      insertEdge (value[3], value[1], globalGraphNode, value[2])
    end

    if (not plusCaseChild) then
      local auxChild = deepcopy(child1)

      for _, plusValue in ipairs(child1) do
        for _, childValue2 in ipairs(child1) do
          insertEdge (globalGraphNode, plusValue[1], globalGraphNode, plusValue[2])
          childValue2[3] = globalGraphNode
        end
      end
      for _, value in ipairs(auxChild) do
        table.insert(child1, value)
      end
    else
      for _, plusValue in ipairs(plusCaseChild) do
        for _, childValue2 in ipairs(child1) do
          insertEdge (globalGraphNode, plusValue[1], childValue2[3], plusValue[2])
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
    if (eventInEventNameList (grammarTree[1]) == 0) then
      os.exit()
    end
    local threadIdTree = threadIdParser.threadIdParser(grammarTree[2])

    return {{grammarTree[1], threadIdTree, currentGraphNode}}
  else
    if(grammarTree["tag"] == "or") then
      child1, _ = processTree (currentGraphNode, grammarTree[1], 1)
      child2, _ = processTree (currentGraphNode, grammarTree[2], 1)

      local childs = actionTagOr (currentGraphNode, child1, child2)
      return childs, childs
    else
      if(grammarTree["tag"] == "plus") then
        local plusCaseChild
        child1, plusCaseChild = processTree (currentGraphNode, grammarTree[1], hasFatherOr)
        actionTagPlus (currentGraphNode, child1, plusCaseChild, hasFatherOr)

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

function graphManager.printGraph(graph)

  print(graph)
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

function graphManager.createGraph (grammarTree, namesTable)
  graph = {}
  currentEvent = {}
  globalGraphNode = 1
  eventsNamesTable = namesTable

  local lastChild, _ = processTree(1, grammarTree, nil)

  if (lastChild) then
    for _,value in ipairs(lastChild) do 
      insertEdge (value[3], value[1], -1, value[2])
    end
  end

  table.insert(currentEvent, 1)

  return graph, currentEvent
end

return graphManager