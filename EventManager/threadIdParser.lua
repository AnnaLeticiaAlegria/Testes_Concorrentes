local pt = require"pt"


local m = require "lpeg"


local function syntaxerror (s, i)
  local eols = string.gsub(string.sub(s, 1, i), "[^\n]", "")
  io.stdout:write("syntax error line ", #eols + 1, ": ",
   string.sub(s, i - 30, i - 1), "❌", string.sub(s, i, i + 30), "\n") -- prints the characters before and after the error
  os.exit(false)
end


local function packbin (tag)
  return function (a, b)
    return {tag = tag, a, b}
  end
end

local inlimit = 0

-- Tokens
local S = m.V"S"

local OS = '[' * S
local CS = ']' * S

local OP = '(' * S
local CP = ')' * S

local PlusOp = '+' * S
local MinusOp = '-' * S
local AssignOp = ">>" * S
local NotOp = '~' * S

local ID = m.C(m.R("az", "AZ") * m.R("az", "AZ", "09")^0) * S

local G = m.P{"ThreadExp";

  ThreadExp = S * OS * m.V"Exp" * CS * -m.P(1),

  Exp = ((m.V"Term")^-1 * AssignOp * m.V"Term") / packbin("assign")
      + m.V"Term",

  Term = (OP^-1 * m.V"Plus" * CP^-1) 
       + (OP^-1 * m.V"Minus" * CP^-1)
       + (OP^-1 * m.V"Not" * CP^-1)
       + (OP^-1 * m.V"Item" * CP^-1),

  Plus = ((m.V"Item")^-1 * PlusOp * m.V"Item") / packbin("plus"),

  Minus = ((m.V"Item")^-1 * MinusOp * m.V"Item") / packbin("minus"),

  Not = (NotOp * m.V"Item") / packbin("not"),

  Item = (OP^-1 * ID * CP^-1) / packbin("item"),



  S = m.S(" \t")^0 *
        m.P(function (_, i)
             -- track input limit
             inlimit = math.max(inlimit, i)
             return i
           end),

}


function threadIdParser (input)
  local p = m.match(G, input)
  if not p then
    syntaxerror(input, inlimit)
  end
  return p
end


-- print(pt.pt(threadIdParser("[ termo2  >> termo3]")))
-- print(pt.pt(threadIdParser("[termo2 + termo3]")))

return {threadIdParser=threadIdParser}