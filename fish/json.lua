local settings = require('fish.settings')
local backend = {
  decode = function (e, ...) return {} end,
  encode = function (e, ...) return '' end,
}

if settings.cjson then
  print('settings: using cjson as json backend!')
  backend = require('cjson')
else
  backend = require('fish.dkjson')
  print('settings: using dkjson as json backend!')
end

local json = {}

---@param e any
---@param ... unknown
---@return table
function json.decode(e, ...)
  local r = backend.decode(e, ...)
  if type(r) ~= "table" then r = {r} end
  return r
end

---@param e any
---@param ... unknown
---@return string
function json.encode(e, ...)
  local r = backend.encode(e, ...)
  return tostring(r)
end

return json