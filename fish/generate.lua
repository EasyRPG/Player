local ast = require('fish.ast')
local gen = {}

--[[function gen.rmdata(filename)
  local f = assert(io.open(filename, "r"))
  local text = f:read("*a")
  f:close()

  text = text:gsub(
    "\n?%s*// <fish%-generated>.-// </fish%-generated>\n?",
    "\n"
  )

  f = assert(io.open(filename, "w"))
  f:write(text)
  f:close()
end]]

---@param node ast.node
function gen.genfunc(node)
  
end

return gen