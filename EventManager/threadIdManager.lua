local lpeg = require 'lpeg'

local threadIdManager = {}
local threadIdTable = {}

-- returns [term1] [term2] [term3] ----> threadP >> groupP -----> returns threadP, >>, groupP
local function splitTerms (threadExpression)
  local space = lpeg.S(" \t\n")^0
  local assignSignal = space * lpeg.C(lpeg.P(">>+") + lpeg.P(">>-") + lpeg.P(">>")) * space
  local notAssign = (1 - assignSignal)^0

  local firstTerm = lpeg.C(notAssign)
  local lastTerm = lpeg.C(lpeg.R("az", "AZ","(-","09")^0) * space * lpeg.P("]")

  local expressionParser = (lpeg.P("[") * space * ((firstTerm)^-1 * assignSignal)^-1 * lastTerm)

  local term1, term2, term3 = lpeg.match(expressionParser, threadExpression)

  -- print(term1, term2, term3)

  local splitCase = "assign"
  if (not term2) then
    splitCase = "check"
  end

  return splitCase, {term1, term2, term3}
end

local function existsInTable (searchedTable, element)
  if(searchedTable) then
    for key, value in pairs(searchedTable) do
      if (value == element) then
        if (type(key) == "string") then return 0 end
        return key
      end
    end
  end

  return -1
end


local function solveOperators(threadTerm1, threadId)
  -- resolver as operacoes
  local resultedTable = {}
  local returnedTable = {}

  -------- funções do lpeg para separar os conjuntos
  if (lpeg.match(lpeg.P("~"), threadTerm1)) then
    threadTerm1 = threadTerm1:gsub("~", "")
  end

  resultedTable = threadIdTable[threadTerm1] -- mudar para o resultado dos conjuntos

  if (resultedTable) then
    for _, value in pairs(resultedTable) do
      table.insert(returnedTable, value)
    end
  end
  
  return returnedTable
end

local function checkCase (threadTerm, threadId)

  resultedTable = solveOperators(threadTerm, threadId)

  if (existsInTable(threadIdTable[threadTerm], threadId) >= 0) then return 1 end

  return 0
end

local function assignCase (threadTerm1, threadTerm2, threadTerm3, threadId)

  local resultedTable = {}

  -- Step 1: prepare first set (solve possible operators)

  if (threadTerm1 == "") then
    table.insert(resultedTable, threadId)
  else
    resultedTable = solveOperators(threadTerm1, threadId)
  end

  -- Step 2: check if threadId exists in first set or if doesn't exist
  if (lpeg.match(lpeg.P("~"), threadTerm1)) then
    if (existsInTable(resultedTable, threadId) >= 0) then return 0 end
  else
    if (existsInTable(resultedTable, threadId) < 0) then return 0 end
  end
  
  -- Step 3: if threadId exists in first set, adds it on second set

  if (not (threadIdTable[threadTerm3])) then
    threadIdTable[threadTerm3] = {}
  end

  if (threadTerm2 == ">>-") then
    local index = existsInTable(threadIdTable[threadTerm3], threadId)
    if (index >= 0) then
      table.remove(threadIdTable[threadTerm3], index)
    end
  else
    if(threadTerm2 == ">>") then threadIdTable[threadTerm3] = {} end
    table.insert(threadIdTable[threadTerm3], threadId)
  end

  return 1
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

function plusCase (term1, term2)
  if (term2) then -- two terms to sum
    for _, value in ipairs(term2) do
      table.insert(term1, value)
    end
  else -- case >>+
    term1["case"] = "plus"
  end

  return term1
end


function minusCase (term1, term2)
  local aux = term1
  if (term2) then -- two terms to subtract
    for _, value in ipairs(term2) do
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


function processThreadTree (threadTree)
  local term1, term2

  if (threadTree["tag"] == "item") then
    return {threadTree[1]}
  end

  term1 = processThreadTree (threadTree[1])
  if (threadTree[2]) then
    term2 = processThreadTree (threadTree[2])
  end

  if (threadTree["tag"] == "assign") then
    if (term2) then
      return term1, term2
    else
      return nil, term1
    end
  else
    if (threadTree["tag"] == "plus") then
      return plusCase (term1, term2)
    else
      if (threadTree["tag"] == "minus") then
        return minusCase (term1, term2)
      else
        if (threadTree["tag"] == "not") then
          term1["not"] = true
          return term1
        else
          print("Error processing threadId --> Inexistent tag")
        end
      end
    end
  end

end


function threadIdManager.checkThreadId (threadIdTree, threadId)
  print(threadIdTree)

  local isThreadId = 1

  term1, term2 = processThreadTree(threadIdTree)
  
  if(term1) then
    for key, value in pairs(term1) do
      print(key)
      print(value)
    end
  else
    print(term1)
  end

  if(term2) then
    for key, value in pairs(term2) do
      print(key)
      print(value)
    end
  else
    print(term2)
  end

  -- if (isThreadId == 1) then
  --   printTable (threadIdTable)
  -- end
  return isThreadId
end

return threadIdManager