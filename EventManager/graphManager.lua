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
    if (existsInTable(graph[entryNode][eventName][1], exitNode) == 0) then
      table.insert(graph[entryNode][eventName][1], exitNode)
    end
  end
end

local function actionTagOr (currentGraphNode, children)
  local auxTable = {}
  auxTable["starChildName"] = {}

  -- it doesn't create a new node in the graph, just combine the children into a single table
  for key,child in pairs(children) do 
    for _, value in ipairs(child) do
      table.insert(auxTable,value)
      table.insert(auxTable["starChildName"],value)
    end
  end
  auxTable["starChildDestiny"] = children["child1"][1]["currentGraphNode"]

  return auxTable
end

local function actionTagSeq (currentGraphNode, children)
  globalGraphNode = globalGraphNode + 1
  for _,child in ipairs(children) do 
    insertEdge (child["currentGraphNode"], child["eventName"], globalGraphNode, child["threadIdTree"])
  end
end

local function actionTagPlus (currentGraphNode, children, hasFatherOr)

  if(hasFatherOr) then
    -- if this stars is inside an or, we need to create extra nodes
    globalGraphNode = globalGraphNode + 1

    for _, child in ipairs(children) do
      insertEdge (child["currentGraphNode"], child["eventName"], globalGraphNode, child["threadIdTree"])
    end
    

    if (not children["starChildName"]) then
      -- if this star contains only an item
      local auxChild = deepcopy(children)

      for _, plusValue in pairs(children) do
        for _, childValue in pairs(children) do
          insertEdge (globalGraphNode, plusValue["eventName"], globalGraphNode, plusValue["threadIdTree"]) -- create a loop edge on globalGraphNode
          childValue["currentGraphNode"] = globalGraphNode -- update child1 currentGraphNode
        end
      end
      for _, value in pairs(auxChild) do
        -- do this to return child1 original currentGraphNode and child1 updated currentGraphNode
        table.insert(children, value)
      end
    else
      -- if this star contains other tags that are not items
      local starChildDestinyNode = (children["starGrandChildDestiny"] or children["starChildDestiny"])
      for _,starChild in pairs(children["starChildName"]) do
          insertEdge (globalGraphNode, starChild["eventName"], starChildDestinyNode, starChild["threadIdTree"])
      end
    end
  else
    -- just create a circle in the graph, linking the child's currentNode to star's currentNode
    for _, child in ipairs(children) do
      insertEdge (child["currentGraphNode"], child["eventName"], currentGraphNode, child["threadIdTree"])
    end
  end

  return children
end
  
local function processTree (currentGraphNode, grammarTree, hasFatherOr)
  local children = {}
  if (grammarTree["tag"] == "item") then
    if (eventInEventNameList (grammarTree[1]) == 0) then -- if there is a configuration file, check if this event exists
      os.exit()
    end
    local threadIdTree = threadIdParser.threadIdParser(grammarTree[2]) -- convert ThreadIdString into ThreadIdTree

    local item = {}
    item["eventName"] = grammarTree[1]
    item["threadIdTree"] = threadIdTree
    item["currentGraphNode"] = currentGraphNode
    return {item}
  else
    -- for other tags: grammarTree[1] is the first child and grammarTree[2] is the second one
    if(grammarTree["tag"] == "or") then
      children["child1"] = processTree (currentGraphNode, grammarTree[1], 1)
      children["child2"] = processTree (currentGraphNode, grammarTree[2], 1)

      children = actionTagOr (currentGraphNode, children)

      return children
    else
      if(grammarTree["tag"] == "plus") then
        local plusCaseChild
        children = processTree (currentGraphNode, grammarTree[1], hasFatherOr)

        return actionTagPlus (currentGraphNode, children, hasFatherOr)
      else
        if(grammarTree["tag"] == "seq") then
          local childAux = processTree (currentGraphNode, grammarTree[1], hasFatherOr)

          actionTagSeq (currentGraphNode, childAux) -- create a new node and link it

          children = processTree (globalGraphNode, grammarTree[2], hasFatherOr)

          children["starChildName"] = childAux
          children["starGrandChildDestiny"] = children["starChildDestiny"]
          children["starChildDestiny"] = childAux[1]["currentGraphNode"]

          return children -- child2 needs to be linked by who called this function. If childAux exists, return it
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

  local lastChild = processTree(1, grammarTree, nil)

  if (lastChild) then
    for _,child in ipairs(lastChild) do 
      insertEdge (child["currentGraphNode"], child["eventName"], -1, child["threadIdTree"])
    end
  end

  table.insert(currentEvent, 1)

  return graph, currentEvent
end

return graphManager