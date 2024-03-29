local function pt (x, id)
  id = id or ""
  if type(x) == "string" then return "'" .. tostring(x) .. "'"
  elseif type(x) ~= "table" then return tostring(x)
  else
    local s = id .. "{\n"
    for k,v in pairs(x) do
      s = s .. id .. tostring(k) .. " = " .. pt(v, id .. "  ") .. ";\n"
    end
    s = s .. id .. "}"
    return s
  end
end

return {pt=pt}
