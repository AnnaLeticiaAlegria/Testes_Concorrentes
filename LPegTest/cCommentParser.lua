-- This example is from the library LPeg's documentation
-- It can be found here: http://lua-users.org/wiki/LpegRecipes
-- Some additions were done by Anna Leticia in order to fully understand the example

lpeg = require 'lpeg'
match = lpeg.match

local BEGIN_COMMENT = lpeg.P("/*")
local END_COMMENT = lpeg.P("*/")
local NOT_BEGIN = (1 - BEGIN_COMMENT)^0 -- any x which is not '/*'
local NOT_END = (1 - END_COMMENT)^0
local FULL_COMMENT_CONTENTS = BEGIN_COMMENT * NOT_END * END_COMMENT -- * to indicate combining patterns in this order

-- Parser to find comments from a string
local searchParser = (NOT_BEGIN * lpeg.C(FULL_COMMENT_CONTENTS))^0 -- NOT_BEGIN followed by capture of FULL_COMMENT_CONTENTS
print(searchParser:match("codigoA/*comentario*/codigoB"))
-- if searchParser was (lpeg.C(NOT_BEGIN) * lpeg.C(FULL_COMMENT_CONTENTS))^0 the result would be codigoA/*comentario*/

-- Parser to find non-comments from a string
local filterParser = (lpeg.C(NOT_BEGIN) * FULL_COMMENT_CONTENTS)^0 * lpeg.C(NOT_BEGIN)
local a, b = filterParser:match("codigoA/*comentario*/codigoB")
print(a)
print(b)

-- Simpler version, although empirically it is slower.... (why?) ... any optimization
-- suggestions are desired as well as optimum integration w/ C++ comments and other
-- syntax elements
local searchParser = (lpeg.C(FULL_COMMENT_CONTENTS) + 1)^0
print(searchParser:match("codigoA/*comentario*/codigoB"))

-- Suggestion by Roberto to make the search faster
-- Works because it loops fast over all non-slashes, then it begins the slower match phase
local searchParser = ((1 - lpeg.P"/")^0 * (lpeg.C(FULL_COMMENT_CONTENTS) + 1))^0
local aux = lpeg.C((1 - lpeg.P"/")^0)
print(searchParser:match("codigoA///*comentario*/codigoB"))
print(aux:match("codigoA///*comentario*/codigoB"))