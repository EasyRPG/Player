local uv = require('luv')
local lfs = require("lfs")

local fs = {}

---@param path string
---@param ext string
---@param res? string[]
---@param opt? {rec: boolean?, skip: table<string, boolean>?}
---@return string[]
function fs.find(path, ext, res, opt)
  res = res or {}
  opt = opt or {}

  local rec = opt.rec ~= false
  local skip = opt.skip or {}

  for e in lfs.dir(path) do
    if e ~= "." and e ~= ".." then
      local p = path .. "/" .. e
      local a = lfs.attributes(p)

      if a.mode == "directory" then
        if rec and not skip[e] then
          fs.find(p, ext, res, opt)
        end
      elseif a.mode == "file" and e:sub(-#ext) == ext then
        table.insert(res, p)
      end
    end
  end

  return res
end

---@param path string
---@return number?
function fs.time(path)
  local a = lfs.attributes(path)
  return a and a.modification
end

---@param path string
---@return number?
function fs.size(path)
  local a = lfs.attributes(path)
  return a and a.size
end

function fs.isabs(path)
  return path:match("^/") or path:match("^%a:[/\\]")
end

function fs.realpath(path)
  return uv.fs_realpath(path)
end

---@param file string
---@param dir string
---@return boolean
function fs.childof(file, dir)
  local function normalize(path)
    path = path:gsub("\\", "/")
    path = path:gsub("/+$", "")
    return path
  end

  file = fs.realpath(normalize(file))
  dir = fs.realpath(normalize(dir))

  if file == dir then return true end

  return file:sub(1, #dir + 1) == dir .. "/"
end

return fs