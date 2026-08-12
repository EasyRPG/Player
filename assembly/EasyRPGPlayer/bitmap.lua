_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}

--- from Assembly 'EasyRPGPlayer::bitmap'
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.EasyRPGPlayer = _G.EasyRPGPlayer.Assembly.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly.EasyRPGPlayer.bitmap = _G.EasyRPGPlayer.Assembly.EasyRPGPlayer.bitmap or {}

_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ptr<EasyRPGPlayer.Assembly.ImageOut>
EasyRPGPlayer.Assembly['ImageOut *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.ImageOut>):ptr<ptr<EasyRPGPlayer.Assembly.ImageOut>>
EasyRPGPlayer.Assembly['ImageOut *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.EasyRPGPlayer.bitmap.P8ImageOut.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.ImageOut>):EasyRPGPlayer.Assembly.ImageOut
EasyRPGPlayer.Assembly['ImageOut *'].value = function(...)
  return EasyRPGPlayer.Assembly.EasyRPGPlayer.bitmap.P8ImageOut.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ref<EasyRPGPlayer.Assembly.ImageOut>
EasyRPGPlayer.Assembly['ImageOut &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageOut):ptr<EasyRPGPlayer.Assembly.ImageOut>
EasyRPGPlayer.Assembly['ImageOut &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.EasyRPGPlayer.bitmap['8ImageOut'].ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class EasyRPGPlayer.Assembly.ImageOut
EasyRPGPlayer.Assembly.ImageOut = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageOut):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageOut.width = function(...)
  return EasyRPGPlayer.Assembly.EasyRPGPlayer.bitmap['8ImageOut'].width(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageOut):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageOut.height = function(...)
  return EasyRPGPlayer.Assembly.EasyRPGPlayer.bitmap['8ImageOut'].height(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageOut):ptr<EasyRPGPlayer.Assembly.void>
EasyRPGPlayer.Assembly.ImageOut.pixels = function(...)
  return EasyRPGPlayer.Assembly.EasyRPGPlayer.bitmap['8ImageOut'].pixels(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageOut):ptr<EasyRPGPlayer.Assembly.ImageOut>
EasyRPGPlayer.Assembly.ImageOut.ptr = function(...)
  return EasyRPGPlayer.Assembly.EasyRPGPlayer.bitmap['8ImageOut'].ptr(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageOut):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageOut.bpp = function(...)
  return EasyRPGPlayer.Assembly.EasyRPGPlayer.bitmap['8ImageOut'].bpp(...)
end
---@overload fun():EasyRPGPlayer.Assembly.ImageOut
EasyRPGPlayer.Assembly.ImageOut.new = function(...)
  return EasyRPGPlayer.Assembly.EasyRPGPlayer.bitmap['8ImageOut'].new(...)
end
