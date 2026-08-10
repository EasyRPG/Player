_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}

--- from Assembly 'EasyRPGPlayer'
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ptr<EasyRPGPlayer.Assembly.StdClock>
EasyRPGPlayer.Assembly['StdClock *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.StdClock>):ptr<ptr<EasyRPGPlayer.Assembly.StdClock>>
EasyRPGPlayer.Assembly['StdClock *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.P8StdClock.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.StdClock>):EasyRPGPlayer.Assembly.StdClock
EasyRPGPlayer.Assembly['StdClock *'].value = function(...)
  return EasyRPGPlayer.Assembly.P8StdClock.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ref<EasyRPGPlayer.Assembly.StdClock>
EasyRPGPlayer.Assembly['StdClock &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.StdClock):ptr<EasyRPGPlayer.Assembly.StdClock>
EasyRPGPlayer.Assembly['StdClock &'].ptr = function(...)
  return EasyRPGPlayer.Assembly['8StdClock'].ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class EasyRPGPlayer.Assembly.StdClock
EasyRPGPlayer.Assembly.StdClock = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.StdClock):ptr<EasyRPGPlayer.Assembly.StdClock>
EasyRPGPlayer.Assembly.StdClock.ptr = function(...)
  return EasyRPGPlayer.Assembly['8StdClock'].ptr(...)
end
---@overload fun():const<ptr<char>>
EasyRPGPlayer.Assembly.StdClock.Name = function(...)
  return EasyRPGPlayer.Assembly['8StdClock'].Name(...)
end
---@overload fun():EasyRPGPlayer.Assembly.std.chrono.time_point<std.chrono.steady_clock,std.chrono.duration<longlong,std.ratio<1,1000000000>>>
EasyRPGPlayer.Assembly.StdClock.now = function(...)
  return EasyRPGPlayer.Assembly['8StdClock'].now(...)
end
--- from Assembly 'leasy::std'
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.std = _G.EasyRPGPlayer.Assembly.std or {}


---@class ref<EasyRPGPlayer.Assembly.std.string_view>
EasyRPGPlayer.Assembly.std['string_view &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.std.string_view):ptr<EasyRPGPlayer.Assembly.std.string_view>
EasyRPGPlayer.Assembly.std['string_view &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.NSt3__117basic_string_viewIcNS_11char_traitsIcEEEE.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.std = _G.EasyRPGPlayer.Assembly.std or {}


---@class ptr<EasyRPGPlayer.Assembly.std.string>
EasyRPGPlayer.Assembly.std['string *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.std.string>):ptr<ptr<EasyRPGPlayer.Assembly.std.string>>
EasyRPGPlayer.Assembly.std['string *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.PNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.std.string>):EasyRPGPlayer.Assembly.std.string
EasyRPGPlayer.Assembly.std['string *'].value = function(...)
  return EasyRPGPlayer.Assembly.PNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.std = _G.EasyRPGPlayer.Assembly.std or {}


---@class EasyRPGPlayer.Assembly.std.string
EasyRPGPlayer.Assembly.std.string = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.std.string):ptr<EasyRPGPlayer.Assembly.std.string>
EasyRPGPlayer.Assembly.std.string.ptr = function(...)
  return EasyRPGPlayer.Assembly.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.std = _G.EasyRPGPlayer.Assembly.std or {}


---@class ref<EasyRPGPlayer.Assembly.std.string>
EasyRPGPlayer.Assembly.std['string &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.std.string):ptr<EasyRPGPlayer.Assembly.std.string>
EasyRPGPlayer.Assembly.std['string &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ptr<EasyRPGPlayer.Assembly.unsignedchar>
EasyRPGPlayer.Assembly['unsigned char *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.unsignedchar>):ptr<ptr<EasyRPGPlayer.Assembly.unsignedchar>>
EasyRPGPlayer.Assembly['unsigned char *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.Ph.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.unsignedchar>):EasyRPGPlayer.Assembly.unsignedchar
EasyRPGPlayer.Assembly['unsigned char *'].value = function(...)
  return EasyRPGPlayer.Assembly.Ph.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ptr<EasyRPGPlayer.Assembly.unsignedint>
EasyRPGPlayer.Assembly['unsigned int *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.unsignedint>):ptr<ptr<EasyRPGPlayer.Assembly.unsignedint>>
EasyRPGPlayer.Assembly['unsigned int *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.Pj.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.unsignedint>):EasyRPGPlayer.Assembly.unsignedint
EasyRPGPlayer.Assembly['unsigned int *'].value = function(...)
  return EasyRPGPlayer.Assembly.Pj.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class EasyRPGPlayer.Assembly.unsignedshort
EasyRPGPlayer.Assembly['unsigned short'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.unsignedshort
---@overload fun(arg0: EasyRPGPlayer.Assembly.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.Assembly.unsignedshort
---@overload fun():EasyRPGPlayer.Assembly.unsignedshort
EasyRPGPlayer.Assembly['unsigned short'].new = function(...)
  return EasyRPGPlayer.Assembly.t.new(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned short'].is_even = function(...)
  return EasyRPGPlayer.Assembly.t.is_even(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort, arg1: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly['unsigned short'].add = function(...)
  return EasyRPGPlayer.Assembly.t.add(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort, arg1: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned short'].lt = function(...)
  return EasyRPGPlayer.Assembly.t.lt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort, arg1: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly['unsigned short'].mul = function(...)
  return EasyRPGPlayer.Assembly.t.mul(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort, arg1: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned short'].ne = function(...)
  return EasyRPGPlayer.Assembly.t.ne(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort, arg1: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned short'].gt = function(...)
  return EasyRPGPlayer.Assembly.t.gt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort, arg1: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned short'].ge = function(...)
  return EasyRPGPlayer.Assembly.t.ge(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort, arg1: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly['unsigned short'].sub = function(...)
  return EasyRPGPlayer.Assembly.t.sub(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort):ptr<EasyRPGPlayer.Assembly.unsignedshort>
EasyRPGPlayer.Assembly['unsigned short'].ptr = function(...)
  return EasyRPGPlayer.Assembly.t.ptr(...)
end
---@overload fun():EasyRPGPlayer.Assembly.unsignedshort
EasyRPGPlayer.Assembly['unsigned short'].max = function(...)
  return EasyRPGPlayer.Assembly.t.max(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort, arg1: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly['unsigned short'].div = function(...)
  return EasyRPGPlayer.Assembly.t.div(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned short'].is_signed = function(...)
  return EasyRPGPlayer.Assembly.t.is_signed(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort, arg1: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned short'].eq = function(...)
  return EasyRPGPlayer.Assembly.t.eq(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort, arg1: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned short'].le = function(...)
  return EasyRPGPlayer.Assembly.t.le(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned short'].is_zero = function(...)
  return EasyRPGPlayer.Assembly.t.is_zero(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned short'].is_negative = function(...)
  return EasyRPGPlayer.Assembly.t.is_negative(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned short'].is_odd = function(...)
  return EasyRPGPlayer.Assembly.t.is_odd(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned short'].is_positive = function(...)
  return EasyRPGPlayer.Assembly.t.is_positive(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.unsignedshort
EasyRPGPlayer.Assembly['unsigned short'].abs = function(...)
  return EasyRPGPlayer.Assembly.t.abs(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort, arg1: EasyRPGPlayer.Assembly.unsignedshort):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly['unsigned short'].mod = function(...)
  return EasyRPGPlayer.Assembly.t.mod(...)
end
---@overload fun():EasyRPGPlayer.Assembly.unsignedshort
EasyRPGPlayer.Assembly['unsigned short'].min = function(...)
  return EasyRPGPlayer.Assembly.t.min(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ptr<EasyRPGPlayer.Assembly.bool>
EasyRPGPlayer.Assembly['bool *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.bool>):ptr<ptr<EasyRPGPlayer.Assembly.bool>>
EasyRPGPlayer.Assembly['bool *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.Pb.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.bool>):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['bool *'].value = function(...)
  return EasyRPGPlayer.Assembly.Pb.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ref<EasyRPGPlayer.Assembly.double>
EasyRPGPlayer.Assembly['double &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.double):ptr<EasyRPGPlayer.Assembly.double>
EasyRPGPlayer.Assembly['double &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.d.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ptr<EasyRPGPlayer.Assembly.longlong>
EasyRPGPlayer.Assembly['long long *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.longlong>):ptr<ptr<EasyRPGPlayer.Assembly.longlong>>
EasyRPGPlayer.Assembly['long long *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.Px.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.longlong>):EasyRPGPlayer.Assembly.longlong
EasyRPGPlayer.Assembly['long long *'].value = function(...)
  return EasyRPGPlayer.Assembly.Px.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class EasyRPGPlayer.Assembly.unsignedchar
EasyRPGPlayer.Assembly['unsigned char'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.unsignedchar
---@overload fun(arg0: EasyRPGPlayer.Assembly.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.Assembly.unsignedchar
---@overload fun():EasyRPGPlayer.Assembly.unsignedchar
EasyRPGPlayer.Assembly['unsigned char'].new = function(...)
  return EasyRPGPlayer.Assembly.h.new(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned char'].is_even = function(...)
  return EasyRPGPlayer.Assembly.h.is_even(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar, arg1: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly['unsigned char'].add = function(...)
  return EasyRPGPlayer.Assembly.h.add(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar, arg1: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned char'].lt = function(...)
  return EasyRPGPlayer.Assembly.h.lt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar, arg1: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly['unsigned char'].mul = function(...)
  return EasyRPGPlayer.Assembly.h.mul(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar, arg1: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned char'].ne = function(...)
  return EasyRPGPlayer.Assembly.h.ne(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar, arg1: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned char'].gt = function(...)
  return EasyRPGPlayer.Assembly.h.gt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar, arg1: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned char'].ge = function(...)
  return EasyRPGPlayer.Assembly.h.ge(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar, arg1: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly['unsigned char'].sub = function(...)
  return EasyRPGPlayer.Assembly.h.sub(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar):ptr<EasyRPGPlayer.Assembly.unsignedchar>
EasyRPGPlayer.Assembly['unsigned char'].ptr = function(...)
  return EasyRPGPlayer.Assembly.h.ptr(...)
end
---@overload fun():EasyRPGPlayer.Assembly.unsignedchar
EasyRPGPlayer.Assembly['unsigned char'].max = function(...)
  return EasyRPGPlayer.Assembly.h.max(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar, arg1: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly['unsigned char'].div = function(...)
  return EasyRPGPlayer.Assembly.h.div(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned char'].is_signed = function(...)
  return EasyRPGPlayer.Assembly.h.is_signed(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar, arg1: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned char'].eq = function(...)
  return EasyRPGPlayer.Assembly.h.eq(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar, arg1: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned char'].le = function(...)
  return EasyRPGPlayer.Assembly.h.le(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned char'].is_zero = function(...)
  return EasyRPGPlayer.Assembly.h.is_zero(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned char'].is_negative = function(...)
  return EasyRPGPlayer.Assembly.h.is_negative(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned char'].is_odd = function(...)
  return EasyRPGPlayer.Assembly.h.is_odd(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned char'].is_positive = function(...)
  return EasyRPGPlayer.Assembly.h.is_positive(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.unsignedchar
EasyRPGPlayer.Assembly['unsigned char'].abs = function(...)
  return EasyRPGPlayer.Assembly.h.abs(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar, arg1: EasyRPGPlayer.Assembly.unsignedchar):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly['unsigned char'].mod = function(...)
  return EasyRPGPlayer.Assembly.h.mod(...)
end
---@overload fun():EasyRPGPlayer.Assembly.unsignedchar
EasyRPGPlayer.Assembly['unsigned char'].min = function(...)
  return EasyRPGPlayer.Assembly.h.min(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class EasyRPGPlayer.Assembly.longlong
EasyRPGPlayer.Assembly['long long'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.longlong
---@overload fun(arg0: EasyRPGPlayer.Assembly.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.Assembly.longlong
---@overload fun():EasyRPGPlayer.Assembly.longlong
EasyRPGPlayer.Assembly['long long'].new = function(...)
  return EasyRPGPlayer.Assembly.x.new(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long long'].is_even = function(...)
  return EasyRPGPlayer.Assembly.x.is_even(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong, arg1: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.longlong
EasyRPGPlayer.Assembly['long long'].add = function(...)
  return EasyRPGPlayer.Assembly.x.add(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong, arg1: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long long'].lt = function(...)
  return EasyRPGPlayer.Assembly.x.lt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong, arg1: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.longlong
EasyRPGPlayer.Assembly['long long'].mul = function(...)
  return EasyRPGPlayer.Assembly.x.mul(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong, arg1: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long long'].ne = function(...)
  return EasyRPGPlayer.Assembly.x.ne(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong, arg1: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long long'].gt = function(...)
  return EasyRPGPlayer.Assembly.x.gt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong, arg1: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long long'].ge = function(...)
  return EasyRPGPlayer.Assembly.x.ge(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong, arg1: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.longlong
EasyRPGPlayer.Assembly['long long'].sub = function(...)
  return EasyRPGPlayer.Assembly.x.sub(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong):ptr<EasyRPGPlayer.Assembly.longlong>
EasyRPGPlayer.Assembly['long long'].ptr = function(...)
  return EasyRPGPlayer.Assembly.x.ptr(...)
end
---@overload fun():EasyRPGPlayer.Assembly.longlong
EasyRPGPlayer.Assembly['long long'].max = function(...)
  return EasyRPGPlayer.Assembly.x.max(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong, arg1: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.longlong
EasyRPGPlayer.Assembly['long long'].div = function(...)
  return EasyRPGPlayer.Assembly.x.div(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long long'].is_signed = function(...)
  return EasyRPGPlayer.Assembly.x.is_signed(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong, arg1: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long long'].eq = function(...)
  return EasyRPGPlayer.Assembly.x.eq(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong, arg1: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long long'].le = function(...)
  return EasyRPGPlayer.Assembly.x.le(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long long'].is_zero = function(...)
  return EasyRPGPlayer.Assembly.x.is_zero(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long long'].is_negative = function(...)
  return EasyRPGPlayer.Assembly.x.is_negative(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long long'].is_odd = function(...)
  return EasyRPGPlayer.Assembly.x.is_odd(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long long'].is_positive = function(...)
  return EasyRPGPlayer.Assembly.x.is_positive(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.longlong
EasyRPGPlayer.Assembly['long long'].abs = function(...)
  return EasyRPGPlayer.Assembly.x.abs(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong, arg1: EasyRPGPlayer.Assembly.longlong):EasyRPGPlayer.Assembly.longlong
EasyRPGPlayer.Assembly['long long'].mod = function(...)
  return EasyRPGPlayer.Assembly.x.mod(...)
end
---@overload fun():EasyRPGPlayer.Assembly.longlong
EasyRPGPlayer.Assembly['long long'].min = function(...)
  return EasyRPGPlayer.Assembly.x.min(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ptr<EasyRPGPlayer.Assembly.unsignedshort>
EasyRPGPlayer.Assembly['unsigned short *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.unsignedshort>):ptr<ptr<EasyRPGPlayer.Assembly.unsignedshort>>
EasyRPGPlayer.Assembly['unsigned short *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.Pt.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.unsignedshort>):EasyRPGPlayer.Assembly.unsignedshort
EasyRPGPlayer.Assembly['unsigned short *'].value = function(...)
  return EasyRPGPlayer.Assembly.Pt.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class EasyRPGPlayer.Assembly.unsignedlonglong
EasyRPGPlayer.Assembly['unsigned long long'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.unsignedlonglong
---@overload fun(arg0: EasyRPGPlayer.Assembly.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.Assembly.unsignedlonglong
---@overload fun():EasyRPGPlayer.Assembly.unsignedlonglong
EasyRPGPlayer.Assembly['unsigned long long'].new = function(...)
  return EasyRPGPlayer.Assembly.y.new(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned long long'].is_even = function(...)
  return EasyRPGPlayer.Assembly.y.is_even(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong, arg1: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.unsignedlonglong
EasyRPGPlayer.Assembly['unsigned long long'].add = function(...)
  return EasyRPGPlayer.Assembly.y.add(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong, arg1: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned long long'].lt = function(...)
  return EasyRPGPlayer.Assembly.y.lt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong, arg1: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.unsignedlonglong
EasyRPGPlayer.Assembly['unsigned long long'].mul = function(...)
  return EasyRPGPlayer.Assembly.y.mul(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong, arg1: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned long long'].ne = function(...)
  return EasyRPGPlayer.Assembly.y.ne(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong, arg1: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned long long'].gt = function(...)
  return EasyRPGPlayer.Assembly.y.gt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong, arg1: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned long long'].ge = function(...)
  return EasyRPGPlayer.Assembly.y.ge(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong, arg1: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.unsignedlonglong
EasyRPGPlayer.Assembly['unsigned long long'].sub = function(...)
  return EasyRPGPlayer.Assembly.y.sub(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong):ptr<EasyRPGPlayer.Assembly.unsignedlonglong>
EasyRPGPlayer.Assembly['unsigned long long'].ptr = function(...)
  return EasyRPGPlayer.Assembly.y.ptr(...)
end
---@overload fun():EasyRPGPlayer.Assembly.unsignedlonglong
EasyRPGPlayer.Assembly['unsigned long long'].max = function(...)
  return EasyRPGPlayer.Assembly.y.max(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong, arg1: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.unsignedlonglong
EasyRPGPlayer.Assembly['unsigned long long'].div = function(...)
  return EasyRPGPlayer.Assembly.y.div(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned long long'].is_signed = function(...)
  return EasyRPGPlayer.Assembly.y.is_signed(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong, arg1: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned long long'].eq = function(...)
  return EasyRPGPlayer.Assembly.y.eq(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong, arg1: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned long long'].le = function(...)
  return EasyRPGPlayer.Assembly.y.le(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned long long'].is_zero = function(...)
  return EasyRPGPlayer.Assembly.y.is_zero(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned long long'].is_negative = function(...)
  return EasyRPGPlayer.Assembly.y.is_negative(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned long long'].is_odd = function(...)
  return EasyRPGPlayer.Assembly.y.is_odd(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned long long'].is_positive = function(...)
  return EasyRPGPlayer.Assembly.y.is_positive(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.unsignedlonglong
EasyRPGPlayer.Assembly['unsigned long long'].abs = function(...)
  return EasyRPGPlayer.Assembly.y.abs(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong, arg1: EasyRPGPlayer.Assembly.unsignedlonglong):EasyRPGPlayer.Assembly.unsignedlonglong
EasyRPGPlayer.Assembly['unsigned long long'].mod = function(...)
  return EasyRPGPlayer.Assembly.y.mod(...)
end
---@overload fun():EasyRPGPlayer.Assembly.unsignedlonglong
EasyRPGPlayer.Assembly['unsigned long long'].min = function(...)
  return EasyRPGPlayer.Assembly.y.min(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ref<EasyRPGPlayer.Assembly.short>
EasyRPGPlayer.Assembly['short &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.short):ptr<EasyRPGPlayer.Assembly.short>
EasyRPGPlayer.Assembly['short &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.s.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ref<EasyRPGPlayer.Assembly.unsignedshort>
EasyRPGPlayer.Assembly['unsigned short &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedshort):ptr<EasyRPGPlayer.Assembly.unsignedshort>
EasyRPGPlayer.Assembly['unsigned short &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.t.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class EasyRPGPlayer.Assembly.unsignedint
EasyRPGPlayer.Assembly['unsigned int'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.unsignedint
---@overload fun(arg0: EasyRPGPlayer.Assembly.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.Assembly.unsignedint
---@overload fun():EasyRPGPlayer.Assembly.unsignedint
EasyRPGPlayer.Assembly['unsigned int'].new = function(...)
  return EasyRPGPlayer.Assembly.j.new(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned int'].is_even = function(...)
  return EasyRPGPlayer.Assembly.j.is_even(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint, arg1: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.unsignedint
EasyRPGPlayer.Assembly['unsigned int'].add = function(...)
  return EasyRPGPlayer.Assembly.j.add(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint, arg1: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned int'].lt = function(...)
  return EasyRPGPlayer.Assembly.j.lt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint, arg1: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.unsignedint
EasyRPGPlayer.Assembly['unsigned int'].mul = function(...)
  return EasyRPGPlayer.Assembly.j.mul(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint, arg1: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned int'].ne = function(...)
  return EasyRPGPlayer.Assembly.j.ne(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint, arg1: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned int'].gt = function(...)
  return EasyRPGPlayer.Assembly.j.gt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint, arg1: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned int'].ge = function(...)
  return EasyRPGPlayer.Assembly.j.ge(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint, arg1: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.unsignedint
EasyRPGPlayer.Assembly['unsigned int'].sub = function(...)
  return EasyRPGPlayer.Assembly.j.sub(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint):ptr<EasyRPGPlayer.Assembly.unsignedint>
EasyRPGPlayer.Assembly['unsigned int'].ptr = function(...)
  return EasyRPGPlayer.Assembly.j.ptr(...)
end
---@overload fun():EasyRPGPlayer.Assembly.unsignedint
EasyRPGPlayer.Assembly['unsigned int'].max = function(...)
  return EasyRPGPlayer.Assembly.j.max(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint, arg1: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.unsignedint
EasyRPGPlayer.Assembly['unsigned int'].div = function(...)
  return EasyRPGPlayer.Assembly.j.div(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned int'].is_signed = function(...)
  return EasyRPGPlayer.Assembly.j.is_signed(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint, arg1: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned int'].eq = function(...)
  return EasyRPGPlayer.Assembly.j.eq(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint, arg1: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned int'].le = function(...)
  return EasyRPGPlayer.Assembly.j.le(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned int'].is_zero = function(...)
  return EasyRPGPlayer.Assembly.j.is_zero(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned int'].is_negative = function(...)
  return EasyRPGPlayer.Assembly.j.is_negative(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned int'].is_odd = function(...)
  return EasyRPGPlayer.Assembly.j.is_odd(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['unsigned int'].is_positive = function(...)
  return EasyRPGPlayer.Assembly.j.is_positive(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.unsignedint
EasyRPGPlayer.Assembly['unsigned int'].abs = function(...)
  return EasyRPGPlayer.Assembly.j.abs(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint, arg1: EasyRPGPlayer.Assembly.unsignedint):EasyRPGPlayer.Assembly.unsignedint
EasyRPGPlayer.Assembly['unsigned int'].mod = function(...)
  return EasyRPGPlayer.Assembly.j.mod(...)
end
---@overload fun():EasyRPGPlayer.Assembly.unsignedint
EasyRPGPlayer.Assembly['unsigned int'].min = function(...)
  return EasyRPGPlayer.Assembly.j.min(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ref<EasyRPGPlayer.Assembly.unsignedchar>
EasyRPGPlayer.Assembly['unsigned char &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedchar):ptr<EasyRPGPlayer.Assembly.unsignedchar>
EasyRPGPlayer.Assembly['unsigned char &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.h.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ref<EasyRPGPlayer.Assembly.unsignedlonglong>
EasyRPGPlayer.Assembly['unsigned long long &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedlonglong):ptr<EasyRPGPlayer.Assembly.unsignedlonglong>
EasyRPGPlayer.Assembly['unsigned long long &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.y.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.longdouble
---@overload fun(arg0: EasyRPGPlayer.Assembly.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.Assembly.longdouble
---@overload fun():EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double'].new = function(...)
  return EasyRPGPlayer.Assembly.e.new(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble, arg1: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double'].add = function(...)
  return EasyRPGPlayer.Assembly.e.add(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble, arg1: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long double'].lt = function(...)
  return EasyRPGPlayer.Assembly.e.lt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble, arg1: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long double'].ne = function(...)
  return EasyRPGPlayer.Assembly.e.ne(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble, arg1: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long double'].gt = function(...)
  return EasyRPGPlayer.Assembly.e.gt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble, arg1: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long double'].ge = function(...)
  return EasyRPGPlayer.Assembly.e.ge(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble, arg1: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double'].sub = function(...)
  return EasyRPGPlayer.Assembly.e.sub(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):ptr<EasyRPGPlayer.Assembly.longdouble>
EasyRPGPlayer.Assembly['long double'].ptr = function(...)
  return EasyRPGPlayer.Assembly.e.ptr(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double'].sin = function(...)
  return EasyRPGPlayer.Assembly.e.sin(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble, arg1: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double'].div = function(...)
  return EasyRPGPlayer.Assembly.e.div(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double'].sqrt = function(...)
  return EasyRPGPlayer.Assembly.e.sqrt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long double'].is_signed = function(...)
  return EasyRPGPlayer.Assembly.e.is_signed(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double'].ceil = function(...)
  return EasyRPGPlayer.Assembly.e.ceil(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble, arg1: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long double'].le = function(...)
  return EasyRPGPlayer.Assembly.e.le(...)
end
---@overload fun():EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double'].epsilon = function(...)
  return EasyRPGPlayer.Assembly.e.epsilon(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double'].cos = function(...)
  return EasyRPGPlayer.Assembly.e.cos(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long double'].is_zero = function(...)
  return EasyRPGPlayer.Assembly.e.is_zero(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble, arg1: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long double'].eq = function(...)
  return EasyRPGPlayer.Assembly.e.eq(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long double'].is_negative = function(...)
  return EasyRPGPlayer.Assembly.e.is_negative(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long double'].is_positive = function(...)
  return EasyRPGPlayer.Assembly.e.is_positive(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble, arg1: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double'].mul = function(...)
  return EasyRPGPlayer.Assembly.e.mul(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double'].abs = function(...)
  return EasyRPGPlayer.Assembly.e.abs(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double'].floor = function(...)
  return EasyRPGPlayer.Assembly.e.floor(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double'].round = function(...)
  return EasyRPGPlayer.Assembly.e.round(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double'].tan = function(...)
  return EasyRPGPlayer.Assembly.e.tan(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long double'].is_nan = function(...)
  return EasyRPGPlayer.Assembly.e.is_nan(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long double'].is_inf = function(...)
  return EasyRPGPlayer.Assembly.e.is_inf(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['long double'].is_finite = function(...)
  return EasyRPGPlayer.Assembly.e.is_finite(...)
end
---@overload fun():EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double'].min = function(...)
  return EasyRPGPlayer.Assembly.e.min(...)
end
---@overload fun():EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double'].max = function(...)
  return EasyRPGPlayer.Assembly.e.max(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.std = _G.EasyRPGPlayer.Assembly.std or {}


---@class EasyRPGPlayer.Assembly.std.string_view
EasyRPGPlayer.Assembly.std.string_view = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.std.string_view):ptr<EasyRPGPlayer.Assembly.std.string_view>
EasyRPGPlayer.Assembly.std.string_view.ptr = function(...)
  return EasyRPGPlayer.Assembly.NSt3__117basic_string_viewIcNS_11char_traitsIcEEEE.ptr(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.std.string_view):EasyRPGPlayer.Assembly.std.string
EasyRPGPlayer.Assembly.std.string_view.get = function(...)
  return EasyRPGPlayer.Assembly.NSt3__117basic_string_viewIcNS_11char_traitsIcEEEE.get(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class EasyRPGPlayer.Assembly.signedchar
EasyRPGPlayer.Assembly['signed char'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.signedchar
---@overload fun(arg0: EasyRPGPlayer.Assembly.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.Assembly.signedchar
---@overload fun():EasyRPGPlayer.Assembly.signedchar
EasyRPGPlayer.Assembly['signed char'].new = function(...)
  return EasyRPGPlayer.Assembly.a.new(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['signed char'].is_even = function(...)
  return EasyRPGPlayer.Assembly.a.is_even(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar, arg1: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly['signed char'].add = function(...)
  return EasyRPGPlayer.Assembly.a.add(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar, arg1: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['signed char'].lt = function(...)
  return EasyRPGPlayer.Assembly.a.lt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar, arg1: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly['signed char'].mul = function(...)
  return EasyRPGPlayer.Assembly.a.mul(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar, arg1: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['signed char'].ne = function(...)
  return EasyRPGPlayer.Assembly.a.ne(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar, arg1: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['signed char'].gt = function(...)
  return EasyRPGPlayer.Assembly.a.gt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar, arg1: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['signed char'].ge = function(...)
  return EasyRPGPlayer.Assembly.a.ge(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar, arg1: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly['signed char'].sub = function(...)
  return EasyRPGPlayer.Assembly.a.sub(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar):ptr<EasyRPGPlayer.Assembly.signedchar>
EasyRPGPlayer.Assembly['signed char'].ptr = function(...)
  return EasyRPGPlayer.Assembly.a.ptr(...)
end
---@overload fun():EasyRPGPlayer.Assembly.signedchar
EasyRPGPlayer.Assembly['signed char'].max = function(...)
  return EasyRPGPlayer.Assembly.a.max(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar, arg1: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly['signed char'].div = function(...)
  return EasyRPGPlayer.Assembly.a.div(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['signed char'].is_signed = function(...)
  return EasyRPGPlayer.Assembly.a.is_signed(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar, arg1: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['signed char'].eq = function(...)
  return EasyRPGPlayer.Assembly.a.eq(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar, arg1: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['signed char'].le = function(...)
  return EasyRPGPlayer.Assembly.a.le(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['signed char'].is_zero = function(...)
  return EasyRPGPlayer.Assembly.a.is_zero(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['signed char'].is_negative = function(...)
  return EasyRPGPlayer.Assembly.a.is_negative(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['signed char'].is_odd = function(...)
  return EasyRPGPlayer.Assembly.a.is_odd(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly['signed char'].is_positive = function(...)
  return EasyRPGPlayer.Assembly.a.is_positive(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly['signed char'].abs = function(...)
  return EasyRPGPlayer.Assembly.a.abs(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar, arg1: EasyRPGPlayer.Assembly.signedchar):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly['signed char'].mod = function(...)
  return EasyRPGPlayer.Assembly.a.mod(...)
end
---@overload fun():EasyRPGPlayer.Assembly.signedchar
EasyRPGPlayer.Assembly['signed char'].min = function(...)
  return EasyRPGPlayer.Assembly.a.min(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ref<EasyRPGPlayer.Assembly.unsignedint>
EasyRPGPlayer.Assembly['unsigned int &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.unsignedint):ptr<EasyRPGPlayer.Assembly.unsignedint>
EasyRPGPlayer.Assembly['unsigned int &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.j.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ptr<EasyRPGPlayer.Assembly.unsignedlonglong>
EasyRPGPlayer.Assembly['unsigned long long *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.unsignedlonglong>):ptr<ptr<EasyRPGPlayer.Assembly.unsignedlonglong>>
EasyRPGPlayer.Assembly['unsigned long long *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.Py.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.unsignedlonglong>):EasyRPGPlayer.Assembly.unsignedlonglong
EasyRPGPlayer.Assembly['unsigned long long *'].value = function(...)
  return EasyRPGPlayer.Assembly.Py.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ptr<EasyRPGPlayer.Assembly.signedchar>
EasyRPGPlayer.Assembly['signed char *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.signedchar>):ptr<ptr<EasyRPGPlayer.Assembly.signedchar>>
EasyRPGPlayer.Assembly['signed char *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.Pa.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.signedchar>):EasyRPGPlayer.Assembly.signedchar
EasyRPGPlayer.Assembly['signed char *'].value = function(...)
  return EasyRPGPlayer.Assembly.Pa.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ref<EasyRPGPlayer.Assembly.signedchar>
EasyRPGPlayer.Assembly['signed char &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.signedchar):ptr<EasyRPGPlayer.Assembly.signedchar>
EasyRPGPlayer.Assembly['signed char &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.a.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.int = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.int
---@overload fun(arg0: EasyRPGPlayer.Assembly.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.Assembly.int
---@overload fun():EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.int.new = function(...)
  return EasyRPGPlayer.Assembly.i.new(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.int.is_even = function(...)
  return EasyRPGPlayer.Assembly.i.is_even(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int, arg1: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.int.add = function(...)
  return EasyRPGPlayer.Assembly.i.add(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int, arg1: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.int.lt = function(...)
  return EasyRPGPlayer.Assembly.i.lt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int, arg1: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.int.mul = function(...)
  return EasyRPGPlayer.Assembly.i.mul(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int, arg1: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.int.ne = function(...)
  return EasyRPGPlayer.Assembly.i.ne(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int, arg1: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.int.gt = function(...)
  return EasyRPGPlayer.Assembly.i.gt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int, arg1: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.int.ge = function(...)
  return EasyRPGPlayer.Assembly.i.ge(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int, arg1: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.int.sub = function(...)
  return EasyRPGPlayer.Assembly.i.sub(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int):ptr<EasyRPGPlayer.Assembly.int>
EasyRPGPlayer.Assembly.int.ptr = function(...)
  return EasyRPGPlayer.Assembly.i.ptr(...)
end
---@overload fun():EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.int.max = function(...)
  return EasyRPGPlayer.Assembly.i.max(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int, arg1: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.int.div = function(...)
  return EasyRPGPlayer.Assembly.i.div(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.int.is_signed = function(...)
  return EasyRPGPlayer.Assembly.i.is_signed(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int, arg1: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.int.eq = function(...)
  return EasyRPGPlayer.Assembly.i.eq(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int, arg1: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.int.le = function(...)
  return EasyRPGPlayer.Assembly.i.le(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.int.is_zero = function(...)
  return EasyRPGPlayer.Assembly.i.is_zero(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.int.is_negative = function(...)
  return EasyRPGPlayer.Assembly.i.is_negative(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.int.is_odd = function(...)
  return EasyRPGPlayer.Assembly.i.is_odd(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.int.is_positive = function(...)
  return EasyRPGPlayer.Assembly.i.is_positive(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.int.abs = function(...)
  return EasyRPGPlayer.Assembly.i.abs(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.int, arg1: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.int.mod = function(...)
  return EasyRPGPlayer.Assembly.i.mod(...)
end
---@overload fun():EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.int.min = function(...)
  return EasyRPGPlayer.Assembly.i.min(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ref<EasyRPGPlayer.Assembly.int>
EasyRPGPlayer.Assembly['int &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.int):ptr<EasyRPGPlayer.Assembly.int>
EasyRPGPlayer.Assembly['int &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.i.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class EasyRPGPlayer.Assembly.short
EasyRPGPlayer.Assembly.short = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.short
---@overload fun(arg0: EasyRPGPlayer.Assembly.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.Assembly.short
---@overload fun():EasyRPGPlayer.Assembly.short
EasyRPGPlayer.Assembly.short.new = function(...)
  return EasyRPGPlayer.Assembly.s.new(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.short.is_even = function(...)
  return EasyRPGPlayer.Assembly.s.is_even(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short, arg1: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.short.add = function(...)
  return EasyRPGPlayer.Assembly.s.add(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short, arg1: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.short.lt = function(...)
  return EasyRPGPlayer.Assembly.s.lt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short, arg1: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.short.mul = function(...)
  return EasyRPGPlayer.Assembly.s.mul(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short, arg1: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.short.ne = function(...)
  return EasyRPGPlayer.Assembly.s.ne(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short, arg1: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.short.gt = function(...)
  return EasyRPGPlayer.Assembly.s.gt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short, arg1: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.short.ge = function(...)
  return EasyRPGPlayer.Assembly.s.ge(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short, arg1: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.short.sub = function(...)
  return EasyRPGPlayer.Assembly.s.sub(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short):ptr<EasyRPGPlayer.Assembly.short>
EasyRPGPlayer.Assembly.short.ptr = function(...)
  return EasyRPGPlayer.Assembly.s.ptr(...)
end
---@overload fun():EasyRPGPlayer.Assembly.short
EasyRPGPlayer.Assembly.short.max = function(...)
  return EasyRPGPlayer.Assembly.s.max(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short, arg1: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.short.div = function(...)
  return EasyRPGPlayer.Assembly.s.div(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.short.is_signed = function(...)
  return EasyRPGPlayer.Assembly.s.is_signed(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short, arg1: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.short.eq = function(...)
  return EasyRPGPlayer.Assembly.s.eq(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short, arg1: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.short.le = function(...)
  return EasyRPGPlayer.Assembly.s.le(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.short.is_zero = function(...)
  return EasyRPGPlayer.Assembly.s.is_zero(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.short.is_negative = function(...)
  return EasyRPGPlayer.Assembly.s.is_negative(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.short.is_odd = function(...)
  return EasyRPGPlayer.Assembly.s.is_odd(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.short.is_positive = function(...)
  return EasyRPGPlayer.Assembly.s.is_positive(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.short.abs = function(...)
  return EasyRPGPlayer.Assembly.s.abs(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.short, arg1: EasyRPGPlayer.Assembly.short):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.short.mod = function(...)
  return EasyRPGPlayer.Assembly.s.mod(...)
end
---@overload fun():EasyRPGPlayer.Assembly.short
EasyRPGPlayer.Assembly.short.min = function(...)
  return EasyRPGPlayer.Assembly.s.min(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ptr<EasyRPGPlayer.Assembly.short>
EasyRPGPlayer.Assembly['short *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.short>):ptr<ptr<EasyRPGPlayer.Assembly.short>>
EasyRPGPlayer.Assembly['short *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.Ps.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.short>):EasyRPGPlayer.Assembly.short
EasyRPGPlayer.Assembly['short *'].value = function(...)
  return EasyRPGPlayer.Assembly.Ps.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ptr<EasyRPGPlayer.Assembly.int>
EasyRPGPlayer.Assembly['int *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.int>):ptr<ptr<EasyRPGPlayer.Assembly.int>>
EasyRPGPlayer.Assembly['int *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.Pi.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.int>):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly['int *'].value = function(...)
  return EasyRPGPlayer.Assembly.Pi.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ref<EasyRPGPlayer.Assembly.longlong>
EasyRPGPlayer.Assembly['long long &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.longlong):ptr<EasyRPGPlayer.Assembly.longlong>
EasyRPGPlayer.Assembly['long long &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.x.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.bool
---@overload fun(arg0: EasyRPGPlayer.Assembly.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.Assembly.bool
---@overload fun():EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool.new = function(...)
  return EasyRPGPlayer.Assembly.b.new(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool.is_even = function(...)
  return EasyRPGPlayer.Assembly.b.is_even(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool, arg1: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.bool.add = function(...)
  return EasyRPGPlayer.Assembly.b.add(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool, arg1: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool.lt = function(...)
  return EasyRPGPlayer.Assembly.b.lt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool, arg1: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.bool.mul = function(...)
  return EasyRPGPlayer.Assembly.b.mul(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool, arg1: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool.ne = function(...)
  return EasyRPGPlayer.Assembly.b.ne(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool, arg1: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool.gt = function(...)
  return EasyRPGPlayer.Assembly.b.gt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool, arg1: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool.ge = function(...)
  return EasyRPGPlayer.Assembly.b.ge(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool, arg1: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.bool.sub = function(...)
  return EasyRPGPlayer.Assembly.b.sub(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool):ptr<EasyRPGPlayer.Assembly.bool>
EasyRPGPlayer.Assembly.bool.ptr = function(...)
  return EasyRPGPlayer.Assembly.b.ptr(...)
end
---@overload fun():EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool.max = function(...)
  return EasyRPGPlayer.Assembly.b.max(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool, arg1: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool.div = function(...)
  return EasyRPGPlayer.Assembly.b.div(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool.is_signed = function(...)
  return EasyRPGPlayer.Assembly.b.is_signed(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool, arg1: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool.eq = function(...)
  return EasyRPGPlayer.Assembly.b.eq(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool, arg1: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool.le = function(...)
  return EasyRPGPlayer.Assembly.b.le(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool.is_zero = function(...)
  return EasyRPGPlayer.Assembly.b.is_zero(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool.is_negative = function(...)
  return EasyRPGPlayer.Assembly.b.is_negative(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool.is_odd = function(...)
  return EasyRPGPlayer.Assembly.b.is_odd(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool.is_positive = function(...)
  return EasyRPGPlayer.Assembly.b.is_positive(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.bool):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool.abs = function(...)
  return EasyRPGPlayer.Assembly.b.abs(...)
end
---@overload fun():EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.bool.min = function(...)
  return EasyRPGPlayer.Assembly.b.min(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ref<EasyRPGPlayer.Assembly.bool>
EasyRPGPlayer.Assembly['bool &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.bool):ptr<EasyRPGPlayer.Assembly.bool>
EasyRPGPlayer.Assembly['bool &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.b.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly.float = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.float
---@overload fun(arg0: EasyRPGPlayer.Assembly.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.Assembly.float
---@overload fun():EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly.float.new = function(...)
  return EasyRPGPlayer.Assembly.f.new(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float, arg1: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly.float.add = function(...)
  return EasyRPGPlayer.Assembly.f.add(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float, arg1: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.float.lt = function(...)
  return EasyRPGPlayer.Assembly.f.lt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float, arg1: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.float.ne = function(...)
  return EasyRPGPlayer.Assembly.f.ne(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float, arg1: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.float.gt = function(...)
  return EasyRPGPlayer.Assembly.f.gt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float, arg1: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.float.ge = function(...)
  return EasyRPGPlayer.Assembly.f.ge(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float, arg1: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly.float.sub = function(...)
  return EasyRPGPlayer.Assembly.f.sub(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float):ptr<EasyRPGPlayer.Assembly.float>
EasyRPGPlayer.Assembly.float.ptr = function(...)
  return EasyRPGPlayer.Assembly.f.ptr(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly.float.sin = function(...)
  return EasyRPGPlayer.Assembly.f.sin(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float, arg1: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly.float.div = function(...)
  return EasyRPGPlayer.Assembly.f.div(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly.float.sqrt = function(...)
  return EasyRPGPlayer.Assembly.f.sqrt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.float.is_signed = function(...)
  return EasyRPGPlayer.Assembly.f.is_signed(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly.float.ceil = function(...)
  return EasyRPGPlayer.Assembly.f.ceil(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float, arg1: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.float.le = function(...)
  return EasyRPGPlayer.Assembly.f.le(...)
end
---@overload fun():EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly.float.epsilon = function(...)
  return EasyRPGPlayer.Assembly.f.epsilon(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly.float.cos = function(...)
  return EasyRPGPlayer.Assembly.f.cos(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.float.is_zero = function(...)
  return EasyRPGPlayer.Assembly.f.is_zero(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float, arg1: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.float.eq = function(...)
  return EasyRPGPlayer.Assembly.f.eq(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.float.is_negative = function(...)
  return EasyRPGPlayer.Assembly.f.is_negative(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.float.is_positive = function(...)
  return EasyRPGPlayer.Assembly.f.is_positive(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float, arg1: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly.float.mul = function(...)
  return EasyRPGPlayer.Assembly.f.mul(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly.float.abs = function(...)
  return EasyRPGPlayer.Assembly.f.abs(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly.float.floor = function(...)
  return EasyRPGPlayer.Assembly.f.floor(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly.float.round = function(...)
  return EasyRPGPlayer.Assembly.f.round(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly.float.tan = function(...)
  return EasyRPGPlayer.Assembly.f.tan(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.float.is_nan = function(...)
  return EasyRPGPlayer.Assembly.f.is_nan(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.float.is_inf = function(...)
  return EasyRPGPlayer.Assembly.f.is_inf(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.float):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.float.is_finite = function(...)
  return EasyRPGPlayer.Assembly.f.is_finite(...)
end
---@overload fun():EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly.float.min = function(...)
  return EasyRPGPlayer.Assembly.f.min(...)
end
---@overload fun():EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly.float.max = function(...)
  return EasyRPGPlayer.Assembly.f.max(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ptr<EasyRPGPlayer.Assembly.float>
EasyRPGPlayer.Assembly['float *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.float>):ptr<ptr<EasyRPGPlayer.Assembly.float>>
EasyRPGPlayer.Assembly['float *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.Pf.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.float>):EasyRPGPlayer.Assembly.float
EasyRPGPlayer.Assembly['float *'].value = function(...)
  return EasyRPGPlayer.Assembly.Pf.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ref<EasyRPGPlayer.Assembly.float>
EasyRPGPlayer.Assembly['float &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.float):ptr<EasyRPGPlayer.Assembly.float>
EasyRPGPlayer.Assembly['float &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.f.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly.double = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.double
---@overload fun(arg0: EasyRPGPlayer.Assembly.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.Assembly.double
---@overload fun():EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly.double.new = function(...)
  return EasyRPGPlayer.Assembly.d.new(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double, arg1: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly.double.add = function(...)
  return EasyRPGPlayer.Assembly.d.add(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double, arg1: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.double.lt = function(...)
  return EasyRPGPlayer.Assembly.d.lt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double, arg1: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.double.ne = function(...)
  return EasyRPGPlayer.Assembly.d.ne(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double, arg1: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.double.gt = function(...)
  return EasyRPGPlayer.Assembly.d.gt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double, arg1: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.double.ge = function(...)
  return EasyRPGPlayer.Assembly.d.ge(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double, arg1: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly.double.sub = function(...)
  return EasyRPGPlayer.Assembly.d.sub(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double):ptr<EasyRPGPlayer.Assembly.double>
EasyRPGPlayer.Assembly.double.ptr = function(...)
  return EasyRPGPlayer.Assembly.d.ptr(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly.double.sin = function(...)
  return EasyRPGPlayer.Assembly.d.sin(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double, arg1: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly.double.div = function(...)
  return EasyRPGPlayer.Assembly.d.div(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly.double.sqrt = function(...)
  return EasyRPGPlayer.Assembly.d.sqrt(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.double.is_signed = function(...)
  return EasyRPGPlayer.Assembly.d.is_signed(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly.double.ceil = function(...)
  return EasyRPGPlayer.Assembly.d.ceil(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double, arg1: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.double.le = function(...)
  return EasyRPGPlayer.Assembly.d.le(...)
end
---@overload fun():EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly.double.epsilon = function(...)
  return EasyRPGPlayer.Assembly.d.epsilon(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly.double.cos = function(...)
  return EasyRPGPlayer.Assembly.d.cos(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.double.is_zero = function(...)
  return EasyRPGPlayer.Assembly.d.is_zero(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double, arg1: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.double.eq = function(...)
  return EasyRPGPlayer.Assembly.d.eq(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.double.is_negative = function(...)
  return EasyRPGPlayer.Assembly.d.is_negative(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.double.is_positive = function(...)
  return EasyRPGPlayer.Assembly.d.is_positive(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double, arg1: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly.double.mul = function(...)
  return EasyRPGPlayer.Assembly.d.mul(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly.double.abs = function(...)
  return EasyRPGPlayer.Assembly.d.abs(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly.double.floor = function(...)
  return EasyRPGPlayer.Assembly.d.floor(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly.double.round = function(...)
  return EasyRPGPlayer.Assembly.d.round(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly.double.tan = function(...)
  return EasyRPGPlayer.Assembly.d.tan(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.double.is_nan = function(...)
  return EasyRPGPlayer.Assembly.d.is_nan(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.double.is_inf = function(...)
  return EasyRPGPlayer.Assembly.d.is_inf(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.double):EasyRPGPlayer.Assembly.bool
EasyRPGPlayer.Assembly.double.is_finite = function(...)
  return EasyRPGPlayer.Assembly.d.is_finite(...)
end
---@overload fun():EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly.double.min = function(...)
  return EasyRPGPlayer.Assembly.d.min(...)
end
---@overload fun():EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly.double.max = function(...)
  return EasyRPGPlayer.Assembly.d.max(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.std = _G.EasyRPGPlayer.Assembly.std or {}


---@class ptr<EasyRPGPlayer.Assembly.std.string_view>
EasyRPGPlayer.Assembly.std['string_view *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.std.string_view>):ptr<ptr<EasyRPGPlayer.Assembly.std.string_view>>
EasyRPGPlayer.Assembly.std['string_view *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.PNSt3__117basic_string_viewIcNS_11char_traitsIcEEEE.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.std.string_view>):EasyRPGPlayer.Assembly.std.string_view
EasyRPGPlayer.Assembly.std['string_view *'].value = function(...)
  return EasyRPGPlayer.Assembly.PNSt3__117basic_string_viewIcNS_11char_traitsIcEEEE.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ptr<EasyRPGPlayer.Assembly.double>
EasyRPGPlayer.Assembly['double *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.double>):ptr<ptr<EasyRPGPlayer.Assembly.double>>
EasyRPGPlayer.Assembly['double *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.Pd.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.double>):EasyRPGPlayer.Assembly.double
EasyRPGPlayer.Assembly['double *'].value = function(...)
  return EasyRPGPlayer.Assembly.Pd.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ptr<EasyRPGPlayer.Assembly.longdouble>
EasyRPGPlayer.Assembly['long double *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.longdouble>):ptr<ptr<EasyRPGPlayer.Assembly.longdouble>>
EasyRPGPlayer.Assembly['long double *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.Pe.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.longdouble>):EasyRPGPlayer.Assembly.longdouble
EasyRPGPlayer.Assembly['long double *'].value = function(...)
  return EasyRPGPlayer.Assembly.Pe.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ref<EasyRPGPlayer.Assembly.longdouble>
EasyRPGPlayer.Assembly['long double &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.longdouble):ptr<EasyRPGPlayer.Assembly.longdouble>
EasyRPGPlayer.Assembly['long double &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.e.ptr(...)
end
--- from Assembly 'ImageBMP'
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.ImageBMP = _G.EasyRPGPlayer.Assembly.ImageBMP or {}


---@class ref<EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader>
EasyRPGPlayer.Assembly.ImageBMP['BitmapHeader &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):ptr<EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader>
EasyRPGPlayer.Assembly.ImageBMP['BitmapHeader &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.N8ImageBMP12BitmapHeaderE.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.ImageBMP = _G.EasyRPGPlayer.Assembly.ImageBMP or {}


---@class ptr<EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader>
EasyRPGPlayer.Assembly.ImageBMP['BitmapHeader *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader>):ptr<ptr<EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader>>
EasyRPGPlayer.Assembly.ImageBMP['BitmapHeader *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.PN8ImageBMP12BitmapHeaderE.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader>):EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader
EasyRPGPlayer.Assembly.ImageBMP['BitmapHeader *'].value = function(...)
  return EasyRPGPlayer.Assembly.PN8ImageBMP12BitmapHeaderE.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.ImageBMP = _G.EasyRPGPlayer.Assembly.ImageBMP or {}


---@class EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):ptr<EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader>
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.ptr = function(...)
  return EasyRPGPlayer.Assembly.N8ImageBMP12BitmapHeaderE.ptr(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.num_colors = function(...)
  return EasyRPGPlayer.Assembly.N8ImageBMP12BitmapHeaderE.num_colors(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.size = function(...)
  return EasyRPGPlayer.Assembly.N8ImageBMP12BitmapHeaderE.size(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.w = function(...)
  return EasyRPGPlayer.Assembly.N8ImageBMP12BitmapHeaderE.w(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.depth = function(...)
  return EasyRPGPlayer.Assembly.N8ImageBMP12BitmapHeaderE.depth(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.h = function(...)
  return EasyRPGPlayer.Assembly.N8ImageBMP12BitmapHeaderE.h(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.planes = function(...)
  return EasyRPGPlayer.Assembly.N8ImageBMP12BitmapHeaderE.planes(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.compression = function(...)
  return EasyRPGPlayer.Assembly.N8ImageBMP12BitmapHeaderE.compression(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.palette_size = function(...)
  return EasyRPGPlayer.Assembly.N8ImageBMP12BitmapHeaderE.palette_size(...)
end
---@overload fun():EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader
EasyRPGPlayer.Assembly.ImageBMP.BitmapHeader.new = function(...)
  return EasyRPGPlayer.Assembly.N8ImageBMP12BitmapHeaderE.new(...)
end
--- from Assembly 'leasy::meta2::node'
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.leasy = _G.EasyRPGPlayer.Assembly.leasy or {}
_G.EasyRPGPlayer.Assembly.leasy.meta2 = _G.EasyRPGPlayer.Assembly.leasy.meta2 or {}
_G.EasyRPGPlayer.Assembly.leasy.meta2.node = _G.EasyRPGPlayer.Assembly.leasy.meta2.node or {}


---@class ref<EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D>
EasyRPGPlayer.Assembly.leasy.meta2.node['Node2D &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D):ptr<EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D>
EasyRPGPlayer.Assembly.leasy.meta2.node['Node2D &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.N5leasy5meta24node6Node2DE.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.leasy = _G.EasyRPGPlayer.Assembly.leasy or {}
_G.EasyRPGPlayer.Assembly.leasy.meta2 = _G.EasyRPGPlayer.Assembly.leasy.meta2 or {}
_G.EasyRPGPlayer.Assembly.leasy.meta2.node = _G.EasyRPGPlayer.Assembly.leasy.meta2.node or {}


---@class ptr<EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D>
EasyRPGPlayer.Assembly.leasy.meta2.node['Node2D *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D>):ptr<ptr<EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D>>
EasyRPGPlayer.Assembly.leasy.meta2.node['Node2D *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.PN5leasy5meta24node6Node2DE.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D>):EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D
EasyRPGPlayer.Assembly.leasy.meta2.node['Node2D *'].value = function(...)
  return EasyRPGPlayer.Assembly.PN5leasy5meta24node6Node2DE.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.leasy = _G.EasyRPGPlayer.Assembly.leasy or {}
_G.EasyRPGPlayer.Assembly.leasy.meta2 = _G.EasyRPGPlayer.Assembly.leasy.meta2 or {}
_G.EasyRPGPlayer.Assembly.leasy.meta2.node = _G.EasyRPGPlayer.Assembly.leasy.meta2.node or {}


---@class EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D
EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D):ptr<EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D>
EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D.ptr = function(...)
  return EasyRPGPlayer.Assembly.N5leasy5meta24node6Node2DE.ptr(...)
end
---@overload fun():EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D
---@overload fun(arg0: EasyRPGPlayer.Assembly.int, arg1: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D
EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D.new = function(...)
  return EasyRPGPlayer.Assembly.N5leasy5meta24node6Node2DE.new(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D):EasyRPGPlayer.Assembly.std.tuple<int,int>
EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D.pos = function(...)
  return EasyRPGPlayer.Assembly.N5leasy5meta24node6Node2DE.pos(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D, arg1: EasyRPGPlayer.Assembly.int, arg2: EasyRPGPlayer.Assembly.int):EasyRPGPlayer.Assembly.void
EasyRPGPlayer.Assembly.leasy.meta2.node.Node2D.move = function(...)
  return EasyRPGPlayer.Assembly.N5leasy5meta24node6Node2DE.move(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.leasy = _G.EasyRPGPlayer.Assembly.leasy or {}
_G.EasyRPGPlayer.Assembly.leasy.meta2 = _G.EasyRPGPlayer.Assembly.leasy.meta2 or {}
_G.EasyRPGPlayer.Assembly.leasy.meta2.node = _G.EasyRPGPlayer.Assembly.leasy.meta2.node or {}


---@class ptr<EasyRPGPlayer.Assembly.leasy.meta2.node.Node>
EasyRPGPlayer.Assembly.leasy.meta2.node['Node *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.leasy.meta2.node.Node>):ptr<ptr<EasyRPGPlayer.Assembly.leasy.meta2.node.Node>>
EasyRPGPlayer.Assembly.leasy.meta2.node['Node *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.PN5leasy5meta24node4NodeE.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.leasy.meta2.node.Node>):EasyRPGPlayer.Assembly.leasy.meta2.node.Node
EasyRPGPlayer.Assembly.leasy.meta2.node['Node *'].value = function(...)
  return EasyRPGPlayer.Assembly.PN5leasy5meta24node4NodeE.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.leasy = _G.EasyRPGPlayer.Assembly.leasy or {}
_G.EasyRPGPlayer.Assembly.leasy.meta2 = _G.EasyRPGPlayer.Assembly.leasy.meta2 or {}
_G.EasyRPGPlayer.Assembly.leasy.meta2.node = _G.EasyRPGPlayer.Assembly.leasy.meta2.node or {}


---@class ref<EasyRPGPlayer.Assembly.leasy.meta2.node.Node>
EasyRPGPlayer.Assembly.leasy.meta2.node['Node &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.leasy.meta2.node.Node):ptr<EasyRPGPlayer.Assembly.leasy.meta2.node.Node>
EasyRPGPlayer.Assembly.leasy.meta2.node['Node &'].ptr = function(...)
  return EasyRPGPlayer.Assembly.N5leasy5meta24node4NodeE.ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}
_G.EasyRPGPlayer.Assembly.leasy = _G.EasyRPGPlayer.Assembly.leasy or {}
_G.EasyRPGPlayer.Assembly.leasy.meta2 = _G.EasyRPGPlayer.Assembly.leasy.meta2 or {}
_G.EasyRPGPlayer.Assembly.leasy.meta2.node = _G.EasyRPGPlayer.Assembly.leasy.meta2.node or {}


---@class EasyRPGPlayer.Assembly.leasy.meta2.node.Node
EasyRPGPlayer.Assembly.leasy.meta2.node.Node = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.leasy.meta2.node.Node):ptr<EasyRPGPlayer.Assembly.leasy.meta2.node.Node>
EasyRPGPlayer.Assembly.leasy.meta2.node.Node.ptr = function(...)
  return EasyRPGPlayer.Assembly.N5leasy5meta24node4NodeE.ptr(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.leasy.meta2.node.Node):EasyRPGPlayer.Assembly.void
EasyRPGPlayer.Assembly.leasy.meta2.node.Node.ready = function(...)
  return EasyRPGPlayer.Assembly.N5leasy5meta24node4NodeE.ready(...)
end
---@overload fun():EasyRPGPlayer.Assembly.leasy.meta2.node.Node
EasyRPGPlayer.Assembly.leasy.meta2.node.Node.new = function(...)
  return EasyRPGPlayer.Assembly.N5leasy5meta24node4NodeE.new(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.leasy.meta2.node.Node, arg1: EasyRPGPlayer.Assembly.d):EasyRPGPlayer.Assembly.void
EasyRPGPlayer.Assembly.leasy.meta2.node.Node.update = function(...)
  return EasyRPGPlayer.Assembly.N5leasy5meta24node4NodeE.update(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.leasy.meta2.node.Node):ptr<EasyRPGPlayer.Assembly.leasy.iky7.Cursor<std.__wrap_iter<std.shared_ptr<leasy.meta2.node.Node>>
EasyRPGPlayer.Assembly.leasy.meta2.node.Node.visit = function(...)
  return EasyRPGPlayer.Assembly.N5leasy5meta24node4NodeE.visit(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.leasy.meta2.node.Node):EasyRPGPlayer.Assembly.void
EasyRPGPlayer.Assembly.leasy.meta2.node.Node.draw = function(...)
  return EasyRPGPlayer.Assembly.N5leasy5meta24node4NodeE.draw(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.leasy.meta2.node.Node):EasyRPGPlayer.Assembly.std.vector<std.shared_ptr<leasy.meta2.node.Node>>
EasyRPGPlayer.Assembly.leasy.meta2.node.Node.children = function(...)
  return EasyRPGPlayer.Assembly.N5leasy5meta24node4NodeE.children(...)
end
--- from Assembly 'bitmap.cpp'
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ptr<EasyRPGPlayer.Assembly.ImageOut>
EasyRPGPlayer.Assembly['ImageOut *'] = {}

---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.ImageOut>):ptr<ptr<EasyRPGPlayer.Assembly.ImageOut>>
EasyRPGPlayer.Assembly['ImageOut *'].ptr = function(...)
  return EasyRPGPlayer.Assembly.P8ImageOut.ptr(...)
end
---@overload fun(arg0: ptr<EasyRPGPlayer.Assembly.ImageOut>):EasyRPGPlayer.Assembly.ImageOut
EasyRPGPlayer.Assembly['ImageOut *'].value = function(...)
  return EasyRPGPlayer.Assembly.P8ImageOut.value(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class ref<EasyRPGPlayer.Assembly.ImageOut>
EasyRPGPlayer.Assembly['ImageOut &'] = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageOut):ptr<EasyRPGPlayer.Assembly.ImageOut>
EasyRPGPlayer.Assembly['ImageOut &'].ptr = function(...)
  return EasyRPGPlayer.Assembly['8ImageOut'].ptr(...)
end
_G.EasyRPGPlayer = _G.EasyRPGPlayer or {}
_G.EasyRPGPlayer.Assembly = _G.EasyRPGPlayer.Assembly or {}


---@class EasyRPGPlayer.Assembly.ImageOut
EasyRPGPlayer.Assembly.ImageOut = {}

---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageOut):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageOut.width = function(...)
  return EasyRPGPlayer.Assembly['8ImageOut'].width(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageOut):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageOut.height = function(...)
  return EasyRPGPlayer.Assembly['8ImageOut'].height(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageOut):ptr<EasyRPGPlayer.Assembly.void>
EasyRPGPlayer.Assembly.ImageOut.pixels = function(...)
  return EasyRPGPlayer.Assembly['8ImageOut'].pixels(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageOut):ptr<EasyRPGPlayer.Assembly.ImageOut>
EasyRPGPlayer.Assembly.ImageOut.ptr = function(...)
  return EasyRPGPlayer.Assembly['8ImageOut'].ptr(...)
end
---@overload fun(arg0: EasyRPGPlayer.Assembly.ImageOut):EasyRPGPlayer.Assembly.int
EasyRPGPlayer.Assembly.ImageOut.bpp = function(...)
  return EasyRPGPlayer.Assembly['8ImageOut'].bpp(...)
end
---@overload fun():EasyRPGPlayer.Assembly.ImageOut
EasyRPGPlayer.Assembly.ImageOut.new = function(...)
  return EasyRPGPlayer.Assembly['8ImageOut'].new(...)
end
--- from Assembly 'Algo'
