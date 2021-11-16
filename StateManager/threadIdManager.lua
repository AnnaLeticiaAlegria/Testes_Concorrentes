local lpeg = require 'lpeg'
local match = lpeg.match

local threadIdManager = {}
local threadIdTable = {}

-- returns [term1] [term2] [term3] ----> threadP >> groupP -----> returns threadP, >>, groupP
local function splitTerms (threadExpression)
  local space = lpeg.S(" \t\n")^0
  local assignSignal = space * lpeg.C(lpeg.P(">>+") + lpeg.P(">>-") + lpeg.P(">>")) * space
  local notAssign = (1 - assignSignal)^0

  local expressionParser = (((lpeg.C(notAssign))^-1 * assignSignal)^-1 * lpeg.C(notAssign))

  threadExpression = threadExpression:gsub("%[", "")
  threadExpression = threadExpression:gsub("%]", "")

  local term1, term2, term3 = lpeg.match(expressionParser, threadExpression)

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

  -- Step 2: check if threadId exists in first set

  if (existsInTable(resultedTable, threadId) < 0) then return 0 end
  
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

function threadIdManager.checkThreadId (threadExpression, threadId)
  local splitCase, terms = splitTerms (string.sub(threadExpression,2,string.len(threadExpression) - 1)) -- removing brackets
  local isThreadId

  -- print("Thread expression "..threadExpression)
  
  if (splitCase == "check") then
    isThreadId = checkCase (terms[1], threadId)
  else
    isThreadId = assignCase (terms[1], terms[2], terms[3], threadId)
  end

  if (isThreadId == 1) then
    printTable (threadIdTable)
  end
  return isThreadId
end

return threadIdManager

-- print("\n\n[threadP] --> 123")
-- print(threadIdManager.checkThreadId("[threadP]",123))

-- print("\n\n[threadP] --> 321")
-- print(threadIdManager.checkThreadId("[threadP]",321))

-- print("\n\n[>> groupP] --> 456")
-- print(threadIdManager.checkThreadId("[>> groupP]",456))

-- print("\n\n[threadP >> groupP] --> 123")
-- print(threadIdManager.checkThreadId("[threadP >> groupP]", 123))

-- print("\n\n[ >> threadA] --> 789")
-- print(threadIdManager.checkThreadId("[ >> threadA]", 789))

-- print("\n\n[threadA >>+ groupP] --> 789")
-- print(threadIdManager.checkThreadId("[threadA >>+ groupP]", 789))

-- print("\n\n[threadA >>- groupP] --> 789")
-- print(threadIdManager.checkThreadId("[threadA >>- groupP]", 789))

-- local terms
-- _, terms = splitTerms ("[groupP >>- groupP] ")
-- print(terms[1], terms[2], terms[3])
-- _, terms = splitTerms ("[threadP >>+ groupP]")

-- print(terms[1], terms[2], terms[3])