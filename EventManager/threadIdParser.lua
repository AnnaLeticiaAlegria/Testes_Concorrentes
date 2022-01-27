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

local function tablepack ()
  return function (capturedTable)
    local term1, op
    local tree = {}
    for key, value in pairs(capturedTable) do
      tree = {}
      if (value == '+' or value == '-' or value == '~') then
        op = value
      else
        if (not op) then
          term1 = value
        else
          table.insert(tree, term1)
          table.insert(tree, value)
          if (op == '+') then
            tree["tag"] = "plus"
          else if (op == '-') then
            tree["tag"] = "minus"
            else
              tree["tag"] = "not"
            end
          end
          op = nil
          term1 = tree
        end
      end
    end

    return term1
  end
end

local inlimit = 0

-- Tokens
local S = m.V"S"

local OS = '[' * S
local CS = ']' * S

local OP = '(' * S
local CP = ')' * S

local PlusOp = m.C('+') * S
local MinusOp = m.C('-') * S
local AssignOp = ">>" * S
local NotOp = m.C('~') * S

local ID = m.C(m.R("az", "AZ") * m.R("az", "AZ", "09")^0) * S

local G = m.P{"ThreadExp";

  ThreadExp = S * OS * m.V"Exp" * CS * -m.P(1),

  Exp = ((m.V"Term")^-1 * AssignOp * m.V"Term") / packbin("assign")
      + m.V"Term",

  Term = m.Ct(m.V"Prefixed" * ((PlusOp + MinusOp) * m.V"Prefixed")^0) / tablepack(),

  Prefixed = (PlusOp + MinusOp + NotOp)^-1 * m.V"Item",

  Item = (OP * m.V"Exp" * CP) + ID/ packbin("item"),



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


-- print(pt.pt(threadIdParser("[termo1 + termo2]")))
print(pt.pt(threadIdParser("[termo1 + (termo2 + termo3)]")))
-- print(pt.pt(threadIdParser("[termo2 + termo3 >> grupo1]")))
-- print(pt.pt(threadIdParser("[ >> grupo2]")))
-- print(pt.pt(threadIdParser("[ >>+ grupo3]")))

return {threadIdParser=threadIdParser}