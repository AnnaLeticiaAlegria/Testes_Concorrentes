--[[
Item <- Id ('[' TheadExp ']' )? | '(' Exp ')'

Iterate <- Item ('*' ?)

Seq <- Iterate (';' Iterate)* ';'?

Exp <- Seq ('|' Seq)*
]]

local pt = require"pt"


local m = require "lpeg"


local function syntaxerror (s, i)
  local eols = string.gsub(string.sub(s, 1, i), "[^\n]", "")
  io.stdout:write("syntax error line ", #eols + 1, ": ",
   string.sub(s, i - 30, i - 1), "❌", string.sub(s, i, i + 30), "\n")
  os.exit(false)
end


local function packbin (tag)
  return function (a, b)
           return b and {tag = tag, a, b} or a
         end
end


local function packstar (a, b)
  return b and {tag = "star", a} or a
end


local inlimit = 0

-- Tokens
local S = m.V"S"

local OS = '[' * S
local CS = ']' * S

local OP = '(' * S
local CP = ')' * S

local OrOp = '|' * S
local Star = '*' * S
local Sc = ';' * S

local ID = m.C(m.R("az", "AZ") * m.R("az", "AZ", "09")^0) * S --Captura o nome do ID

local G = m.P{"Prog";

  Prog = S * m.V"Exp" * -m.P(1),

  Exp = (m.V"Seq" * Sc^-1 * (OrOp * m.V"Exp")^-1) / packbin("or"),

  Seq = (m.V"Iterate" * (Sc * m.V"Seq")^-1) / packbin("seq"),

  Iterate = (m.V"Item" * m.C(Star)^-1) / packstar,

  Item = (ID * m.C((OS * m.V"ThreadExp" * CS)^-1)) / packbin("item")
       + OP * m.V"Exp" * CP,

  ThreadExp = (1 - m.P"]")^0, -- Tudo antes do "]"

  S = m.S(" \t\n")^0 *
        m.P(function (_, i)
             -- track input limit
             inlimit = math.max(inlimit, i)
             return i
           end),

}


local function grammarParser (input)
  local p = m.match(G, input)
  if not p then
    syntaxerror(input, inlimit)
  end
  return p
end


-- print(pt.pt(parser(io.read("a"))))

return {grammarParser=grammarParser}