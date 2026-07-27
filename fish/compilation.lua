local json = require('fish.json')
local compilation = {}

local compilers = {
  "clang++",
  "clang",
  "g++",
  "gcc",
  "c++",
  "cc",
  "cl.exe",
  "cl",
  "zig c++",
  "zig cc",
  "icx",
  "icpx",
}

local function find_compiler(cmd)
  for _, compiler in ipairs(compilers) do
    local s, e = cmd:find(compiler, 1, true)
    if s then
      return compiler, s, e
    end
  end
end

---@param file string
---@param cmd string
---@return string
function compilation.makecmd(bin, file, cmd)
  local c, s, e = find_compiler(cmd)
  local args = cmd:sub(e + 1)

  local sf, se = args:find(file)
  if sf then
    args = args:sub(1, sf - 1) .. args:sub(se + 1)
  end

  return ([[
    %s -Xclang  -fsyntax-only -Xclang -ast-dump=json %s %s
  ]]):format(bin, file, args)
end

---@param file string
---@param cmd string
---@param clang string?
---@return table
function compilation.compile(file, cmd, clang)
  local f = io.popen(compilation.makecmd(clang or 'clang++', file, cmd))
  if not f then return {} end

  local a, b, c = json.decode(f:read("*a"))
  f:close()
  if type(a) ~= "table" then a = { a } end
  return a or {}
end

function compilation.sysincs(args)
  local includes = {}

  local i = 1
  while i <= #args do
    local arg = args[i]
    if arg then
      if arg == "-isystem" or arg == "-idirafter" or arg == "-F" then
        if args[i + 1] then
          table.insert(includes, args[i + 1])
          i = i + 1
        end

      elseif arg:match("^%-isystem") then
        table.insert(
          includes,
          arg:gsub("^%-isystem", "")
        )

      elseif arg:match("^%-idirafter") then
        table.insert(
          includes,
          arg:gsub("^%-idirafter", "")
        )

      elseif arg:match("^%-F") then
        table.insert(
          includes,
          arg:gsub("^%-F", "")
        )
      end
    end

    i = i + 1
  end

  return includes
end

return compilation