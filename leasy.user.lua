---@diagnostic disable: lowercase-global
local function dump(value)
  local seen = {}

  local function write(v, indent, path)
    indent = indent or ""
    path = path or "root"

    if type(v) ~= "table" then
      io.write(indent .. tostring(v) .. "\n")
      return
    end

    if seen[v] then
      io.write(indent .. "<ref " .. seen[v] .. ">\n")
      return
    end

    seen[v] = path

    io.write(indent .. "{\n")

    for k, val in pairs(v) do
      io.write(indent .. "  [" .. tostring(k) .. "] = ")

      if type(val) == "table" then
        write(val, indent .. "  ", path .. "." .. tostring(k))
      else
        io.write(tostring(val) .. "\n")
      end
    end

    io.write(indent .. "}\n")
  end

  write(value)
end

function leasy.User.ready()
  dump(_G)
  --local o = leasy.meta2.node.Node2D.new()
  --print('o:', o, type(o))
  --print(o:pos())
end
