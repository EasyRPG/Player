---@diagnostic disable: duplicate-doc-alias
local dump = require("dump")

---@alias cindex { name: string, hash: integer }
---@alias liltype { name: string, cindex: cindex }
---@alias func { name: string, arguments: liltype[], return: liltype, overloads: func[]? }
---@alias class { name: string, size: integer, bases: class[], cindex: cindex, methods: func[] }
---@alias namespace { name: string, functions: table<string, func>, classes: table<string, class>, namespaces: table<string, namespace> }

---@param str string
---@param delimiter string
---@return string[]
function string.split(str, delimiter)
  local result = {}

  if delimiter == "" then
    for i = 1, #str do
      result[i] = str:sub(i, i)
    end
    return result
  end

  local start = 1

  while true do
    local pos = string.find(str, delimiter, start, true)
    if not pos then
      table.insert(result, string.sub(str, start))
      break
    end

    table.insert(result, string.sub(str, start, pos - 1))
    start = pos + #delimiter
  end

  return result
end

---@param str string
---@param old string
---@param new string
---@return string
function string.replace(str, old, new)
  old = old:gsub("([%(%)%.%%%+%-%*%?%[%]%^%$])", "%%%1")
  return (str:gsub(old, new))
end

local Cursor = {
  ---@type string[]
  path = { "EasyRPGPlayer" },
}

function Cursor.qualify()
  return table.concat(Cursor.path, ".")
end

---@param name string
function Cursor.advance(name)
  Cursor.path[#Cursor.path + 1] = name
end

function Cursor.stepback()
  table.remove(Cursor.path)
end

local Dump = {}

---@param name string
---@return string
function Dump.cname(name)
  if name:match("^%d") then
    return "_" .. name
  end
  return name
end

---@param fullname string
---@return string
function Dump.basename(fullname)
  local parts = string.split(fullname, "::")
  return Dump.cname(parts[#parts])
end

---@param args liltype[]
function Dump.arguments(args)
  local doc1 = ""
  local doc2 = ""
  local list = ""

  for index, value in ipairs(args) do
    if index > 1 then
      doc1 = doc1 .. ", "
      list = list .. ", "
    end

    doc1 = doc1 .. ("arg%d: %s"):format(index, 'EasyRPGPlayer.' .. value.name)
    doc2 = doc2 .. ("\n---@param arg%d %s"):format(index, 'EasyRPGPlayer.' .. value.name)
    list = list .. ("arg%d"):format(index)
  end

  return {
    doc1 = doc1,
    doc2 = doc2,
    list = list,
  }
end

---@param fun func
function Dump.func(fun)
  local name = Cursor.qualify() .. "." .. fun.name

  if fun.overloads then
    local doc = ""

    for _, overload in ipairs(fun.overloads) do
      doc = doc
        .. ("---@overload fun(%s):%s\n"):format(
          Dump.arguments(overload.arguments).doc1,
          'EasyRPGPlayer.' .. overload["return"].name
        )
    end

    return ("%s\nfunction %s(...) end\n"):format(doc, name)
  end

  local args = Dump.arguments(fun.arguments)

  return ("%s\n---@return %s\nfunction %s(%s) end\n"):format(
    args.doc2,
    'EasyRPGPLayer.' .. fun["return"].name,
    name,
    args.list
  )
end

---@param class class
function Dump.class(class)
  local short = Dump.basename(class.name)

  Cursor.advance(short)

  local fqcn = Cursor.qualify()

  local code = ("---@class %s\n%s = {}\n"):format(fqcn, fqcn)

  for _, method in ipairs(class.methods) do
    code = code .. Dump.func(method)
  end

  Cursor.stepback()

  return ("%s\nEasyRPGPlayer.%s = %s\n---@alias %s %s"):format(
    code,
    string.replace(class.name, "::", "."),
    fqcn,
    fqcn,
    class.cindex.name
  )


end

---@param namespace namespace
function Dump.namespace(namespace)
  local all = ("---@diagnostic disable: missing-return\n%s = {}\n\n"):format(namespace.name)

  if namespace.name ~= "EasyRPGPlayer" then
    all = all .. ("%s = {}\n"):format(namespace.name)
    Cursor.advance(namespace.name)
  end

  for _, value in pairs(namespace.functions) do
    all = all .. Dump.func(value)
  end

  for _, value in pairs(namespace.classes) do
    all = all .. Dump.class(value)
  end

  for _, value in pairs(namespace.namespaces) do
    all = all .. Dump.namespace(value)
  end

  if namespace.name ~= "EasyRPGPlayer" then
    Cursor.stepback()
  end

  return all
end

io.open("out.lua", "w"):write(Dump.namespace(dump))