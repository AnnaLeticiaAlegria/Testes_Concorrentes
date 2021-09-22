--[[
Item <- Id ('[' TheadExp ']' )? | '(' Exp ')'

Iterate <- Item ('*' ?)

Seq <- Iterate (';' Iterate)* ';'?

Exp <- Seq ('|' Seq)*
]]


local function syntaxerror (s, i)
  local eols = string.gsub(string.sub(s, 1, i), "[^\n]", "")
  io.stdout:write("syntax error line ", #eols + 1, ": ",
   string.sub(s, i - 30, i - 1), "❌", string.sub(s, i, i + 30), "\n")
  os.exit(false)
end


local inlimit = 0


local m = require "lpeg"

-- Tokens
local S = m.V"S" 

local OS = '[' * S -- [ seguido por S
local CS = ']' * S -- ] seguido por S

local OP = '(' * S -- ( seguido por S
local CP = ')' * S -- ) seguido por S

local OrOp = '|' * S -- | seguido por S
local Star = '*' * S -- * seguido por S
local Sc = ';' * S -- ; seguido por S

local ID = m.R("az", "AZ") * m.R("az", "AZ", "09")^0 * S -- ID vai ser qualquer letra seguido de qualquer composição de letras ou números seguido por S

local G = m.P{"Prog";

  Prog = S * m.V"Exp" * -m.P(1), -- Programa começa com S seguido por Exp e pode terminar com uma string vazia

  Exp = m.V"Seq" * (OrOp * m.V"Seq")^0, -- Expressões são sequências seguidos por 0 ou mais "| Seq"

  Seq = m.V"Iterate" * (Sc * m.V"Iterate")^0 * Sc^-1, -- Sequências são iterações seguidos por 0 ou mais "; iterações" seguidos por no máximo 1 ";"

  Iterate = m.V"Item" * Star^-1, -- Iterações são Item seguido por no máximo um "*"

  Item = ID * (OS * m.V"ThreadExp" * CS)^-1 --item é um ID seguido por no máximo um "[exp]" ou não tem ID e é só "(Exp)"
       + OP * m.V"Exp" * CP,

  ThreadExp = m.P"exp",

  S = m.S(" \t\n")^0 *
        m.P(function (_, i)
             -- track input limit
             inlimit = math.max(inlimit, i)
             return i
           end),
 -- S começa com 0 ou mais espaços, tabs ou enters seguidos por
}


parser = function (input)
  local p = m.match(G, input)
  if not p then
    syntaxerror(input, inlimit)
  end
  return p
end


print(parser(io.read("a")))
