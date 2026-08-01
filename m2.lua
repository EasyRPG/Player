---@diagnostic disable: duplicate-doc-alias
local dump = require('dump')

---@alias cindex { name: string, hash: integer }
---@alias liltype { name: string, cindex: cindex }
---@alias func { name: string, arguments: liltype[], return: liltype, overloads: func[]? }
---@alias class { name: string, size: integer, bases: class[], cindex: cindex, methods: func[] }
---@alias namespace { name: string, functions: table<string, func>, classes: table<string, class>, namespaces: table<string, namespace> }

---@param str string
---@param delimiter string
---@return string[]
string.split = function(str, delimiter)
  local result = {}

  if delimiter == "" then
    for i = 1, #str do
      result[i] = str:sub(i, i)
    end
    return result
  end

  local start = 1

  while true do
    local pos = string.find(str, delimiter, start, true) -- plain search
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
  ---@type func|class|namespace
  current = {},
  ---@type 'none'|'func'|'class'|'namespace'
  kind = 'none',
  ---@type string[]
  path = { 'EasyRPGPlayer' },
}

Cursor.qualify = function ()
  return table.concat(Cursor.path, '.')
end

---@param name string
Cursor.advance = function (name)
  Cursor.path[#Cursor.path+1] = name
end

Cursor.stepback = function ()
  table.remove(Cursor.path, #Cursor.path)
end

Cursor.go = function (name)
  Cursor.path = string.split(name, '::')
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

---@param args liltype[]
function Dump.arguments(args)
  local doc1 = ''
  local doc2 = ''
  local list = ''
  for index, value in ipairs(args) do
    if index > 1 then doc1 = doc1 .. ', '; list = list .. ', ' end
    doc1 = doc1 .. 'arg' .. tostring(index) .. ': ' .. value.name
    doc2 = doc2 .. '\n---@param arg' .. tostring(index) .. ' ' .. value.name
    list = list .. 'arg' .. tostring(index)
  end

  return {
    doc1 = doc1,
    doc2 = doc2,
    list = list,
  }
end

---@param fun func
function Dump.func(fun)
  local doc = ''
  local lua = ''
  local name = Cursor.qualify() .. '.' .. fun.name
  if fun.overloads then
    for _, value in ipairs(fun.overloads) do
      doc = doc .. ('---@overload fun(%s):%s\n'):format(Dump.arguments(value.arguments).doc1, value["return"].name)
    end

    lua = ('function %s(...) end\n'):format(name)
  else
    local args =  Dump.arguments(fun.arguments)
    doc = args.doc2 .. '\n---@return ' .. fun["return"].name
    lua = ('function %s(%s) end\n'):format(name, args.list)
  end

  return ('%s\n%s'):format(doc, lua)
end

---@param class class
function Dump.class(class)
  local cname = Cursor.qualify() .. '.' .. Dump.cname(class.cindex.name)
  Cursor.advance(cname)

  local code = '---@class ' .. cname .. '\n' .. cname .. ' = {}\n'

  for _, value in ipairs(class.methods) do
    code = code .. Dump.func(value)
  end

  --Cursor.path = old
  Cursor.stepback()
  return ('%s\n%s = %s\n'):format(code, 'EasyRPGPlayer.' .. string.replace(class.name, '::', '.'), cname)
end

---@param namespace namespace
function Dump.namespace(namespace)
  local all = ('local %s = {}'):format(namespace.name)
  for _, value in pairs(namespace.functions) do all = all .. Dump.func(value) end
  for _, value in pairs(namespace.classes) do all = all .. Dump.class(value) end
  for _, value in pairs(namespace.namespaces) do all = all .. Dump.namespace(value) end
  return all
end

io.open('out.lua', 'w'):write(Dump.namespace(dump))