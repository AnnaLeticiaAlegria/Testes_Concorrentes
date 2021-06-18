local lpeg = require 'lpeg'
local match = lpeg.match


function readStates(path)
  local file = io.open(path, "r")
  if not file then 
    return nil 
  end

  local stateNameArray = {}
  local stateIdArray = {}
  for line in file:lines() do
    local space = lpeg.P(' ')
    local notSpace = (1 - space)^0
    local beforeSpace = lpeg.C(notSpace) * space
    local afterSpace = notSpace * space * lpeg.C(notSpace)

    local stateName = beforeSpace:match(line)
    table.insert (stateNameArray, stateName) --insert all characters before space

    local stateIdAux = afterSpace:match(line)
    local stateId
    local anyId = lpeg.P("*")
    local notId = lpeg.P("!")

    if anyId:match(stateIdAux) then
      stateId = 0
    else
      if notId:match(stateIdAux) then
        local pattern = notId * lpeg.C((1 - notId)^0)
        stateId = tonumber(pattern:match(stateIdAux)) * (-1)
      else
        stateId = tonumber(stateIdAux)
      end
    end
    table.insert (stateIdArray, stateId)
  end
  file:close()
  return stateIdArray, stateNameArray, #stateNameArray
end