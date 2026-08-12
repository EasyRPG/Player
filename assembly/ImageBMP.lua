_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}

--- from Assembly 'ImageBMP'
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.ImageBMP = _G.EasyRPGPlayer.Assembly.ImageBMP or {}

_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.ImageBMP = _G.EasyRPGPlayer.Assembly.ImageBMP or {}


---@class ref<EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader>
EasyRPGPlayer.Assembly.ImageBMP['BitmapHeader &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):ptr<EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader>
EasyRPGPlayer.Assembly.ImageBMP['BitmapHeader &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.ImageBMP.N8ImageBMP12BitmapHeaderE.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.ImageBMP = _G.EasyRPGPlayer.Assembly.ImageBMP or {}


---@class ptr<EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader>
EasyRPGPlayer.Assembly.ImageBMP['BitmapHeader *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader>):ptr<ptr<EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader>>
EasyRPGPlayer.Assembly.ImageBMP['BitmapHeader *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.ImageBMP.PN8ImageBMP12BitmapHeaderE.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader>):EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader
EasyRPGPlayer.Assembly.ImageBMP['BitmapHeader *'].value = function(...)
  return EasyRPGPlayer.Assembly.ImageBMP.PN8ImageBMP12BitmapHeaderE.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.ImageBMP = _G.EasyRPGPlayer.Assembly.ImageBMP or {}


---@class EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):ptr<EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader>
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.ptr = function(...)
  return EasyRPGPlayer.Assembly.ImageBMP.N8ImageBMP12BitmapHeaderE.ptr(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.num_colors = function(...)
  return EasyRPGPlayer.Assembly.ImageBMP.N8ImageBMP12BitmapHeaderE.num_colors(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.size = function(...)
  return EasyRPGPlayer.Assembly.ImageBMP.N8ImageBMP12BitmapHeaderE.size(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.w = function(...)
  return EasyRPGPlayer.Assembly.ImageBMP.N8ImageBMP12BitmapHeaderE.w(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.depth = function(...)
  return EasyRPGPlayer.Assembly.ImageBMP.N8ImageBMP12BitmapHeaderE.depth(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.h = function(...)
  return EasyRPGPlayer.Assembly.ImageBMP.N8ImageBMP12BitmapHeaderE.h(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.planes = function(...)
  return EasyRPGPlayer.Assembly.ImageBMP.N8ImageBMP12BitmapHeaderE.planes(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.compression = function(...)
  return EasyRPGPlayer.Assembly.ImageBMP.N8ImageBMP12BitmapHeaderE.compression(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.palette_size = function(...)
  return EasyRPGPlayer.Assembly.ImageBMP.N8ImageBMP12BitmapHeaderE.palette_size(...)
end
---@overload fun():EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.new = function(...)
  return EasyRPGPlayer.Assembly.ImageBMP.N8ImageBMP12BitmapHeaderE.new(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.PKh, arg1: EasyRPGPlayer.Assembly.j, arg2: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.std.tuple<bool,ImageOut>
function EasyRPGPlayer.Assembly.ImageBMP.read(...) end
