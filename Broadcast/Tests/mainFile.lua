package.path = package.path .. ';../ProducerConsumerPassingTheBaton/Tests/?.lua;'

local fooFile = require("file1") 

function boo ()
  local variable = fooFile.foo()
  print(variable)
end