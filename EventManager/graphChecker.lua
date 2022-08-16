local graphChecker = {}
local visitedNodes = {}

local function fromAssignToString (eventRule, saveSubString) 
  if not eventRule then
    return ""
  end

  if eventRule ["tag"] == "assign" then
    if (eventRule[2]) then
      return fromAssignToString (eventRule[2], true)
    else
      return fromAssignToString (eventRule[1], true)
    end
  else
    if eventRule ["tag"] == "item" then
      return (saveSubString and eventRule[1]) or ""
    else
      return fromAssignToString (eventRule[1], saveSubString) .. fromAssignToString (eventRule[2], saveSubString)
    end
  end
end

local function existsInTable (tableInts, element)
  for _, value in ipairs(tableInts) do
    if element == value then
      return true
    end
  end
  return false
end

-- currentEvents is an array of ints
local function arrayToString (currentEvents)
  table.sort(currentEvents)
  local stringOfEvents = ""
  for _, value in ipairs(currentEvents) do
    stringOfEvents = stringOfEvents .. tostring(value) .. "-"
  end
  return string.sub(stringOfEvents, 1, -2)
end

function graphChecker.checkEdges (graph, currentEvents)
  local sameEventEdges = {}
  local currentEventsAux = {}
  local stringOfEvents = arrayToString (currentEvents)

  -- print("currentEvents: "..stringOfEvents)
  if not visitedNodes[stringOfEvents] then
    visitedNodes[stringOfEvents] = true
    for _, nodeNumber in pairs(currentEvents) do
      -- print("estou vendo o nó " .. tostring(nodeNumber))
      if graph[nodeNumber] then
        for eventName, eventTable in pairs(graph[nodeNumber]) do
          -- print("graph event: "..eventName)
          if (not sameEventEdges[eventName]) then
            sameEventEdges[eventName] = {eventTable}
            currentEventsAux[eventName] = eventTable[1] -- insert nextEvents of this event
          else
            for sameEventName, sameEventTable in pairs (sameEventEdges[eventName]) do
              if not graphChecker.compareEvents (eventTable[2], sameEventTable[2]) then -- events have incompatible thread specification
                -- print("retornando falso")
                return false
              end
            end
            for _, nextNode in ipairs(eventTable[1]) do
              if not existsInTable(currentEventsAux[eventName], nextNode) then
                table.insert(currentEventsAux[eventName], nextNode)
              end
            end
          end
        end
      end
    end

    for _, nextNodesTable in pairs (currentEventsAux) do
      if not graphChecker.checkEdges (graph, nextNodesTable) then
        -- print("retornando falso "..stringOfEvents)
        return false
      end
    end
  end

  -- print("truuue :"..stringOfEvents)
  return true
end


function graphChecker.compareEvents (eventRule1, eventRule2)
  return fromAssignToString(eventRule1, false) == fromAssignToString(eventRule2, false)
end

return graphChecker

-- percorrer o grafo e marcar todos os nós em visitedNodes como não visitados

-- sameEventEdges = {}
-- pra cada nó em currentEvents não visitado:
-- -- marcar nó como visitado
-- -- ver todas as arestas e inserir em sameEventEdges -> [[eventName]]
-- -- -- se existir alguma aresta com aquele nome, checar se possuem o mesmo termo2 da árvore
-- -- -- -- se não possuírem: return false
-- -- -- -- se possuírem: adiciona os dois nextNodes numa mesma entrada de currentEventsAux = [[1], [1,2]]
-- -- -- adiciona nextNode em uma entrada de currentEventAux
-- chama a função pra cada entrada em currentEventAux

-- ["1-2-3-4"]