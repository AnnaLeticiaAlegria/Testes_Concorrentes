local lpeg = require 'lpeg'

local threadIdManager = {}
local threadIdTable = {}

local function existsInTableofTables (searchedTable, element)
  if(searchedTable) then
    for key1, value1 in pairs(searchedTable) do
      for key2, value2 in pairs(value1) do
        if (value2 == element) then
          if (type(key2) == "string") then return 0 end
          return key2
        end
      end
    end
  end

  return -1
end

local function existsInTable (searchedTable, element)
  if(searchedTable) then
    for key, value in ipairs(searchedTable) do
      if (value == element) then
        if (type(key) == "string") then return 0 end
        return key
      end
    end
  end

  return -1
end

function printTable (table)
  for key, value in pairs(table) do
    print("\t--------------------")
    print("\t"..key)
    for _, value2 in pairs(value) do
      print("\t\t"..value2)
    end
  end
  print("\t--------------------")
end


function solveTerms (term1, term2, threadId)
  if (term1 and term1[1]) then
    if (existsInTable(term1, threadId) < 0) then
      return 0
    end
  end

  if (term2) then
    local idName = term2[1]
    if (term2["case"] == "minus") then
      local index = existsInTable(threadIdTable[idName], threadId)
      if (index >= 0) then
        table.remove(threadIdTable[idName], index)
      end
    else
      if (term2["case"] ~= "plus") then
        threadIdTable[idName] = {}
      end
      table.insert(threadIdTable[idName], threadId)
    end
  end

  return 1
end

function plusCase (term1, term2)
  local aux = {}
  for key, value in pairs(term1) do
    table.insert(aux, value)
  end

  if (term2) then -- two terms to sum
    for key, value in pairs(term2) do
      table.insert(aux, value)
    end
  else -- case >>+
    aux["case"] = "plus"
  end

  return aux
end


function minusCase (term1, term2)
  local aux = {}
  for key, value in pairs(term1) do
    table.insert(aux, value)
  end

  if (term2) then -- two terms to subtract
    for key, value in pairs(term2) do
      local index = existsInTable(aux, value)
      if (index >= 0) then
        table.remove(aux, index)
      end
    end
  else -- case >>-
    aux["case"] = "minus"
  end

  return aux
end

function notCase (term1)
  local aux = {}

  for key, value in pairs(threadIdTable) do
    for _, value2 in pairs(value) do
      local index = existsInTable(aux, value2)
      if (index < 0) then
        table.insert(aux, value2)
      end
    end
  end

  for key, value in pairs(term1) do
    local index = existsInTable(aux, value)
    if (index >= 0) then
      table.remove(aux, index)
    end
  end
  return aux
end


function processThreadTree (threadTree, isAssign)
  local term1, term2

  if (threadTree["tag"] == "item") then
    if (isAssign) then
      return {threadTree[1]}
    end
    return threadIdTable[threadTree[1]]
  end

  if (threadTree["tag"] == "assign") then
    if (threadTree[2]) then
      term1 = processThreadTree (threadTree[1], nil)
      term2 = processThreadTree (threadTree[2], true)
      return term1, term2
    else
      term1 = processThreadTree (threadTree[1], true)
      return nil, term1
    end
  else
    term1 = processThreadTree (threadTree[1], isAssign)
    if (threadTree[2]) then
      term2 = processThreadTree (threadTree[2], isAssign)
    end

    if (threadTree["tag"] == "plus") then
      return plusCase (term1, term2)
    else
      if (threadTree["tag"] == "minus") then
        return minusCase (term1, term2)
      else
        if (threadTree["tag"] == "not") then
          return notCase (term1)
        else
          print("Error processing threadId --> Inexistent tag")
        end
      end
    end
  end

end


function threadIdManager.checkThreadId (threadIdTree, threadId)

  local isThreadId

  term1, term2 = processThreadTree(threadIdTree, nil)

  isThreadId = solveTerms (term1, term2, threadId)

  -- print("resultado: " .. tostring(isThreadId))

  -- if (isThreadId == 1) then
  --   printTable (threadIdTable)
  -- end
  return isThreadId
end

return threadIdManager