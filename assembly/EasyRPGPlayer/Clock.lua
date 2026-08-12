_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}

--- from Assembly 'EasyRPGPlayer::Clock'
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.EasyRPGPlayer = _G.EasyRPGPlayer.Assembly.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly.EasyRPGPlayer.Clock = _G.EasyRPGPlayer.Assembly.EasyRPGPlayer.Clock or {}

_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ptr<EasyRPGPlayer.Assembly.StdClock>
EasyRPGPlayer.Assembly['StdClock *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.StdClock>):ptr<ptr<EasyRPGPlayer.Assembly.StdClock>>
EasyRPGPlayer.Assembly['StdClock *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.EasyRPGPlayer.Clock.P8StdClock.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.StdClock>):EasyRPGPlayer.Assembly.StdClock
EasyRPGPlayer.Assembly['StdClock *'].value = function(...)
  return EasyRPGPlayer.Assembly.EasyRPGPlayer.Clock.P8StdClock.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ref<EasyRPGPlayer.Assembly.StdClock>
EasyRPGPlayer.Assembly['StdClock &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.StdClock):ptr<EasyRPGPlayer.Assembly.StdClock>
EasyRPGPlayer.Assembly['StdClock &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.EasyRPGPlayer.Clock['8StdClock'].ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class EasyRPGPlayer.Assembly.StdClock
EasyRPGPlayer.Assembly.StdClock = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.StdClock):ptr<EasyRPGPlayer.Assembly.StdClock>
EasyRPGPlayer.Assembly.StdClock.ptr = function(...)
  return EasyRPGPlayer.Assembly.EasyRPGPlayer.Clock['8StdClock'].ptr(...)
end
---@overload fun():const<ptr<char>>
EasyRPGPlayer.Assembly.StdClock.Name = function(...)
  return EasyRPGPlayer.Assembly.EasyRPGPlayer.Clock['8StdClock'].Name(...)
end
---@overload fun():EasyRPGPlayer.Assembly.std.chrono.time_point<std.chrono.steady_clock,std.chrono.duration<longlong,std.ratio<1,1000000000>>>
EasyRPGPlayer.Assembly.StdClock.now = function(...)
  return EasyRPGPlayer.Assembly.EasyRPGPlayer.Clock['8StdClock'].now(...)
end
