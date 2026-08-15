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

---@overload fun(a:integer, b:string):boolean
---@overload fun(c: string, e: string, o: table):string
function name()

end

local easyrpg = require('assembly.appdomain')

local node = EasyRPGPlayer.Assembly.leasy.meta2.node;

function leasy.User.ready()
  --local sprite = node.Sprite2D.new('/Users/wys/Documents/leasy/icon.png', false)
  local n = node.Node.new():makeShared();
  node.addChildToMain(n)
end
