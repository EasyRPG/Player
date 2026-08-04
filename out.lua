---@diagnostic disable: missing-return, undefined-doc-name
EasyRPGPlayer = {}

---@class EasyRPGPlayer.Color
EasyRPGPlayer.Color = {}
---@overload fun(arg1: EasyRPGPlayer.uint8_t, arg2: EasyRPGPlayer.uint8_t, arg3: EasyRPGPlayer.uint8_t, arg4: EasyRPGPlayer.uint8_t):EasyRPGPlayer.Color
---@overload fun():EasyRPGPlayer.Color

function EasyRPGPlayer.Color.new(...) end

EasyRPGPlayer.Color = EasyRPGPlayer.Color
---@alias EasyRPGPlayer.Color 5Color---@class EasyRPGPlayer.double
EasyRPGPlayer.double = {}
---@overload fun(arg1: EasyRPGPlayer.double):EasyRPGPlayer.double
---@overload fun(arg1: EasyRPGPlayer.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.double
---@overload fun():EasyRPGPlayer.double

function EasyRPGPlayer.double.new(...) end

---@param arg1 EasyRPGPlayer.double
---@param arg2 EasyRPGPlayer.double
---@return EasyRPGPLayer.double
function EasyRPGPlayer.double.add(arg1, arg2) end

---@param arg1 EasyRPGPlayer.double
---@param arg2 EasyRPGPlayer.double
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.double.lt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.double
---@return EasyRPGPLayer.double
function EasyRPGPlayer.double.sin(arg1) end

---@param arg1 EasyRPGPlayer.double
---@param arg2 EasyRPGPlayer.double
---@return EasyRPGPLayer.double
function EasyRPGPlayer.double.sub(arg1, arg2) end

---@param arg1 EasyRPGPlayer.double
---@param arg2 EasyRPGPlayer.double
---@return EasyRPGPLayer.double
function EasyRPGPlayer.double.div(arg1, arg2) end

---@param arg1 EasyRPGPlayer.double
---@return EasyRPGPLayer.double
function EasyRPGPlayer.double.sqrt(arg1) end

---@param arg1 EasyRPGPlayer.double
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.double.is_signed(arg1) end

---@param arg1 EasyRPGPlayer.double
---@return EasyRPGPLayer.double
function EasyRPGPlayer.double.ceil(arg1) end

---@param arg1 EasyRPGPlayer.double
---@param arg2 EasyRPGPlayer.double
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.double.le(arg1, arg2) end

---@return EasyRPGPLayer.double
function EasyRPGPlayer.double.epsilon() end

---@param arg1 EasyRPGPlayer.double
---@return EasyRPGPLayer.double
function EasyRPGPlayer.double.cos(arg1) end

---@param arg1 EasyRPGPlayer.double
---@param arg2 EasyRPGPlayer.double
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.double.gt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.double
---@param arg2 EasyRPGPlayer.double
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.double.ne(arg1, arg2) end

---@param arg1 EasyRPGPlayer.double
---@param arg2 EasyRPGPlayer.double
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.double.ge(arg1, arg2) end

---@param arg1 EasyRPGPlayer.double
---@return EasyRPGPLayer.double
function EasyRPGPlayer.double.tan(arg1) end

---@param arg1 EasyRPGPlayer.double
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.double.is_zero(arg1) end

---@param arg1 EasyRPGPlayer.double
---@param arg2 EasyRPGPlayer.double
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.double.eq(arg1, arg2) end

---@param arg1 EasyRPGPlayer.double
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.double.is_negative(arg1) end

---@param arg1 EasyRPGPlayer.double
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.double.is_positive(arg1) end

---@param arg1 EasyRPGPlayer.double
---@param arg2 EasyRPGPlayer.double
---@return EasyRPGPLayer.double
function EasyRPGPlayer.double.mul(arg1, arg2) end

---@param arg1 EasyRPGPlayer.double
---@return EasyRPGPLayer.double
function EasyRPGPlayer.double.abs(arg1) end

---@param arg1 EasyRPGPlayer.double
---@return EasyRPGPLayer.double
function EasyRPGPlayer.double.floor(arg1) end

---@param arg1 EasyRPGPlayer.double
---@return EasyRPGPLayer.double
function EasyRPGPlayer.double.round(arg1) end

---@param arg1 EasyRPGPlayer.double
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.double.is_nan(arg1) end

---@param arg1 EasyRPGPlayer.double
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.double.is_inf(arg1) end

---@param arg1 EasyRPGPlayer.double
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.double.is_finite(arg1) end

---@return EasyRPGPLayer.double
function EasyRPGPlayer.double.min() end

---@return EasyRPGPLayer.double
function EasyRPGPlayer.double.max() end

EasyRPGPlayer.double = EasyRPGPlayer.double
---@alias EasyRPGPlayer.double d---@class EasyRPGPlayer.void
EasyRPGPlayer.void = {}

EasyRPGPlayer.void = EasyRPGPlayer.void
---@alias EasyRPGPlayer.void v---@class EasyRPGPlayer.int64_t
EasyRPGPlayer.int64_t = {}
---@overload fun(arg1: EasyRPGPlayer.int64_t):EasyRPGPlayer.int64_t
---@overload fun(arg1: EasyRPGPlayer.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.int64_t
---@overload fun():EasyRPGPlayer.int64_t

function EasyRPGPlayer.int64_t.new(...) end

---@param arg1 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int64_t.is_even(arg1) end

---@param arg1 EasyRPGPlayer.int64_t
---@param arg2 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.int64_t
function EasyRPGPlayer.int64_t.add(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int64_t
---@param arg2 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int64_t.lt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int64_t
---@param arg2 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.int64_t
function EasyRPGPlayer.int64_t.mul(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int64_t
---@param arg2 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.int64_t
function EasyRPGPlayer.int64_t.sub(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int64_t
---@param arg2 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.int64_t
function EasyRPGPlayer.int64_t.div(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int64_t.is_signed(arg1) end

---@param arg1 EasyRPGPlayer.int64_t
---@param arg2 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int64_t.eq(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int64_t
---@param arg2 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int64_t.le(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int64_t
---@param arg2 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int64_t.gt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int64_t
---@param arg2 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int64_t.ne(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int64_t
---@param arg2 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int64_t.ge(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int64_t.is_zero(arg1) end

---@param arg1 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int64_t.is_negative(arg1) end

---@param arg1 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int64_t.is_odd(arg1) end

---@param arg1 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int64_t.is_positive(arg1) end

---@param arg1 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.int64_t
function EasyRPGPlayer.int64_t.abs(arg1) end

---@param arg1 EasyRPGPlayer.int64_t
---@param arg2 EasyRPGPlayer.int64_t
---@return EasyRPGPLayer.int64_t
function EasyRPGPlayer.int64_t.mod(arg1, arg2) end

---@return EasyRPGPLayer.int64_t
function EasyRPGPlayer.int64_t.min() end

---@return EasyRPGPLayer.int64_t
function EasyRPGPlayer.int64_t.max() end

EasyRPGPlayer.int64_t = EasyRPGPlayer.int64_t
---@alias EasyRPGPlayer.int64_t x---@class EasyRPGPlayer.uint64_t
EasyRPGPlayer.uint64_t = {}
---@overload fun(arg1: EasyRPGPlayer.uint64_t):EasyRPGPlayer.uint64_t
---@overload fun(arg1: EasyRPGPlayer.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.uint64_t
---@overload fun():EasyRPGPlayer.uint64_t

function EasyRPGPlayer.uint64_t.new(...) end

---@param arg1 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint64_t.is_even(arg1) end

---@param arg1 EasyRPGPlayer.uint64_t
---@param arg2 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.uint64_t
function EasyRPGPlayer.uint64_t.add(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint64_t
---@param arg2 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint64_t.lt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint64_t
---@param arg2 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.uint64_t
function EasyRPGPlayer.uint64_t.mul(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint64_t
---@param arg2 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.uint64_t
function EasyRPGPlayer.uint64_t.sub(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint64_t
---@param arg2 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.uint64_t
function EasyRPGPlayer.uint64_t.div(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint64_t.is_signed(arg1) end

---@param arg1 EasyRPGPlayer.uint64_t
---@param arg2 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint64_t.eq(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint64_t
---@param arg2 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint64_t.le(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint64_t
---@param arg2 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint64_t.gt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint64_t
---@param arg2 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint64_t.ne(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint64_t
---@param arg2 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint64_t.ge(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint64_t.is_zero(arg1) end

---@param arg1 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint64_t.is_negative(arg1) end

---@param arg1 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint64_t.is_odd(arg1) end

---@param arg1 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint64_t.is_positive(arg1) end

---@param arg1 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.uint64_t
function EasyRPGPlayer.uint64_t.abs(arg1) end

---@param arg1 EasyRPGPlayer.uint64_t
---@param arg2 EasyRPGPlayer.uint64_t
---@return EasyRPGPLayer.uint64_t
function EasyRPGPlayer.uint64_t.mod(arg1, arg2) end

---@return EasyRPGPLayer.uint64_t
function EasyRPGPlayer.uint64_t.min() end

---@return EasyRPGPLayer.uint64_t
function EasyRPGPlayer.uint64_t.max() end

EasyRPGPlayer.uint64_t = EasyRPGPlayer.uint64_t
---@alias EasyRPGPlayer.uint64_t y---@class EasyRPGPlayer.bool
EasyRPGPlayer.bool = {}
---@overload fun(arg1: EasyRPGPlayer.bool):EasyRPGPlayer.bool
---@overload fun(arg1: EasyRPGPlayer.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.bool
---@overload fun():EasyRPGPlayer.bool

function EasyRPGPlayer.bool.new(...) end

---@param arg1 EasyRPGPlayer.bool
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.bool.is_even(arg1) end

---@param arg1 EasyRPGPlayer.bool
---@param arg2 EasyRPGPlayer.bool
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.bool.add(arg1, arg2) end

---@param arg1 EasyRPGPlayer.bool
---@param arg2 EasyRPGPlayer.bool
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.bool.lt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.bool
---@param arg2 EasyRPGPlayer.bool
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.bool.mul(arg1, arg2) end

---@param arg1 EasyRPGPlayer.bool
---@param arg2 EasyRPGPlayer.bool
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.bool.sub(arg1, arg2) end

---@param arg1 EasyRPGPlayer.bool
---@param arg2 EasyRPGPlayer.bool
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.bool.div(arg1, arg2) end

---@param arg1 EasyRPGPlayer.bool
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.bool.is_signed(arg1) end

---@param arg1 EasyRPGPlayer.bool
---@param arg2 EasyRPGPlayer.bool
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.bool.eq(arg1, arg2) end

---@param arg1 EasyRPGPlayer.bool
---@param arg2 EasyRPGPlayer.bool
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.bool.le(arg1, arg2) end

---@param arg1 EasyRPGPlayer.bool
---@param arg2 EasyRPGPlayer.bool
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.bool.gt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.bool
---@param arg2 EasyRPGPlayer.bool
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.bool.ne(arg1, arg2) end

---@param arg1 EasyRPGPlayer.bool
---@param arg2 EasyRPGPlayer.bool
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.bool.ge(arg1, arg2) end

---@param arg1 EasyRPGPlayer.bool
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.bool.is_zero(arg1) end

---@param arg1 EasyRPGPlayer.bool
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.bool.is_negative(arg1) end

---@param arg1 EasyRPGPlayer.bool
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.bool.is_odd(arg1) end

---@param arg1 EasyRPGPlayer.bool
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.bool.is_positive(arg1) end

---@param arg1 EasyRPGPlayer.bool
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.bool.abs(arg1) end

---@return EasyRPGPLayer.bool
function EasyRPGPlayer.bool.min() end

---@return EasyRPGPLayer.bool
function EasyRPGPlayer.bool.max() end

EasyRPGPlayer.bool = EasyRPGPlayer.bool
---@alias EasyRPGPlayer.bool b---@class EasyRPGPlayer.uint8_t
EasyRPGPlayer.uint8_t = {}
---@overload fun(arg1: EasyRPGPlayer.uint8_t):EasyRPGPlayer.uint8_t
---@overload fun(arg1: EasyRPGPlayer.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.uint8_t
---@overload fun():EasyRPGPlayer.uint8_t

function EasyRPGPlayer.uint8_t.new(...) end

---@param arg1 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint8_t.is_even(arg1) end

---@param arg1 EasyRPGPlayer.uint8_t
---@param arg2 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.uint8_t.add(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint8_t
---@param arg2 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint8_t.lt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint8_t
---@param arg2 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.uint8_t.mul(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint8_t
---@param arg2 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.uint8_t.sub(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint8_t
---@param arg2 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.uint8_t.div(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint8_t.is_signed(arg1) end

---@param arg1 EasyRPGPlayer.uint8_t
---@param arg2 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint8_t.eq(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint8_t
---@param arg2 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint8_t.le(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint8_t
---@param arg2 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint8_t.gt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint8_t
---@param arg2 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint8_t.ne(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint8_t
---@param arg2 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint8_t.ge(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint8_t.is_zero(arg1) end

---@param arg1 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint8_t.is_negative(arg1) end

---@param arg1 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint8_t.is_odd(arg1) end

---@param arg1 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint8_t.is_positive(arg1) end

---@param arg1 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.uint8_t
function EasyRPGPlayer.uint8_t.abs(arg1) end

---@param arg1 EasyRPGPlayer.uint8_t
---@param arg2 EasyRPGPlayer.uint8_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.uint8_t.mod(arg1, arg2) end

---@return EasyRPGPLayer.uint8_t
function EasyRPGPlayer.uint8_t.min() end

---@return EasyRPGPLayer.uint8_t
function EasyRPGPlayer.uint8_t.max() end

EasyRPGPlayer.uint8_t = EasyRPGPlayer.uint8_t
---@alias EasyRPGPlayer.uint8_t h---@class EasyRPGPlayer.char
EasyRPGPlayer.char = {}
---@overload fun(arg1: EasyRPGPlayer.char):EasyRPGPlayer.char
---@overload fun(arg1: EasyRPGPlayer.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.char
---@overload fun():EasyRPGPlayer.char

function EasyRPGPlayer.char.new(...) end

---@param arg1 EasyRPGPlayer.char
---@return EasyRPGPLayer.b
function EasyRPGPlayer.char.is_even(arg1) end

---@param arg1 EasyRPGPlayer.char
---@param arg2 EasyRPGPlayer.char
---@return EasyRPGPLayer.i
function EasyRPGPlayer.char.add(arg1, arg2) end

---@param arg1 EasyRPGPlayer.char
---@param arg2 EasyRPGPlayer.char
---@return EasyRPGPLayer.b
function EasyRPGPlayer.char.lt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.char
---@param arg2 EasyRPGPlayer.char
---@return EasyRPGPLayer.i
function EasyRPGPlayer.char.mul(arg1, arg2) end

---@param arg1 EasyRPGPlayer.char
---@param arg2 EasyRPGPlayer.char
---@return EasyRPGPLayer.i
function EasyRPGPlayer.char.sub(arg1, arg2) end

---@param arg1 EasyRPGPlayer.char
---@param arg2 EasyRPGPlayer.char
---@return EasyRPGPLayer.i
function EasyRPGPlayer.char.div(arg1, arg2) end

---@param arg1 EasyRPGPlayer.char
---@return EasyRPGPLayer.b
function EasyRPGPlayer.char.is_signed(arg1) end

---@param arg1 EasyRPGPlayer.char
---@param arg2 EasyRPGPlayer.char
---@return EasyRPGPLayer.b
function EasyRPGPlayer.char.eq(arg1, arg2) end

---@param arg1 EasyRPGPlayer.char
---@param arg2 EasyRPGPlayer.char
---@return EasyRPGPLayer.b
function EasyRPGPlayer.char.le(arg1, arg2) end

---@param arg1 EasyRPGPlayer.char
---@param arg2 EasyRPGPlayer.char
---@return EasyRPGPLayer.b
function EasyRPGPlayer.char.gt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.char
---@param arg2 EasyRPGPlayer.char
---@return EasyRPGPLayer.b
function EasyRPGPlayer.char.ne(arg1, arg2) end

---@param arg1 EasyRPGPlayer.char
---@param arg2 EasyRPGPlayer.char
---@return EasyRPGPLayer.b
function EasyRPGPlayer.char.ge(arg1, arg2) end

---@param arg1 EasyRPGPlayer.char
---@return EasyRPGPLayer.b
function EasyRPGPlayer.char.is_zero(arg1) end

---@param arg1 EasyRPGPlayer.char
---@return EasyRPGPLayer.b
function EasyRPGPlayer.char.is_negative(arg1) end

---@param arg1 EasyRPGPlayer.char
---@return EasyRPGPLayer.b
function EasyRPGPlayer.char.is_odd(arg1) end

---@param arg1 EasyRPGPlayer.char
---@return EasyRPGPLayer.b
function EasyRPGPlayer.char.is_positive(arg1) end

---@param arg1 EasyRPGPlayer.char
---@return EasyRPGPLayer.i
function EasyRPGPlayer.char.abs(arg1) end

---@param arg1 EasyRPGPlayer.char
---@param arg2 EasyRPGPlayer.char
---@return EasyRPGPLayer.i
function EasyRPGPlayer.char.mod(arg1, arg2) end

---@return EasyRPGPLayer.char
function EasyRPGPlayer.char.min() end

---@return EasyRPGPLayer.char
function EasyRPGPlayer.char.max() end

EasyRPGPlayer.char = EasyRPGPlayer.char
---@alias EasyRPGPlayer.char c---@class EasyRPGPlayer.uint32_t
EasyRPGPlayer.uint32_t = {}
---@overload fun(arg1: EasyRPGPlayer.uint32_t):EasyRPGPlayer.uint32_t
---@overload fun(arg1: EasyRPGPlayer.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.uint32_t
---@overload fun():EasyRPGPlayer.uint32_t

function EasyRPGPlayer.uint32_t.new(...) end

---@param arg1 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint32_t.is_even(arg1) end

---@param arg1 EasyRPGPlayer.uint32_t
---@param arg2 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.uint32_t
function EasyRPGPlayer.uint32_t.add(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint32_t
---@param arg2 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint32_t.lt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint32_t
---@param arg2 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.uint32_t
function EasyRPGPlayer.uint32_t.mul(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint32_t
---@param arg2 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.uint32_t
function EasyRPGPlayer.uint32_t.sub(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint32_t
---@param arg2 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.uint32_t
function EasyRPGPlayer.uint32_t.div(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint32_t.is_signed(arg1) end

---@param arg1 EasyRPGPlayer.uint32_t
---@param arg2 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint32_t.eq(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint32_t
---@param arg2 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint32_t.le(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint32_t
---@param arg2 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint32_t.gt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint32_t
---@param arg2 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint32_t.ne(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint32_t
---@param arg2 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint32_t.ge(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint32_t.is_zero(arg1) end

---@param arg1 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint32_t.is_negative(arg1) end

---@param arg1 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint32_t.is_odd(arg1) end

---@param arg1 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint32_t.is_positive(arg1) end

---@param arg1 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.uint32_t
function EasyRPGPlayer.uint32_t.abs(arg1) end

---@param arg1 EasyRPGPlayer.uint32_t
---@param arg2 EasyRPGPlayer.uint32_t
---@return EasyRPGPLayer.uint32_t
function EasyRPGPlayer.uint32_t.mod(arg1, arg2) end

---@return EasyRPGPLayer.uint32_t
function EasyRPGPlayer.uint32_t.min() end

---@return EasyRPGPLayer.uint32_t
function EasyRPGPlayer.uint32_t.max() end

EasyRPGPlayer.uint32_t = EasyRPGPlayer.uint32_t
---@alias EasyRPGPlayer.uint32_t j---@class EasyRPGPlayer.uint16_t
EasyRPGPlayer.uint16_t = {}
---@overload fun(arg1: EasyRPGPlayer.uint16_t):EasyRPGPlayer.uint16_t
---@overload fun(arg1: EasyRPGPlayer.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.uint16_t
---@overload fun():EasyRPGPlayer.uint16_t

function EasyRPGPlayer.uint16_t.new(...) end

---@param arg1 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint16_t.is_even(arg1) end

---@param arg1 EasyRPGPlayer.uint16_t
---@param arg2 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.uint16_t.add(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint16_t
---@param arg2 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint16_t.lt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint16_t
---@param arg2 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.uint16_t.mul(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint16_t
---@param arg2 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.uint16_t.sub(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint16_t
---@param arg2 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.uint16_t.div(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint16_t.is_signed(arg1) end

---@param arg1 EasyRPGPlayer.uint16_t
---@param arg2 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint16_t.eq(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint16_t
---@param arg2 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint16_t.le(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint16_t
---@param arg2 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint16_t.gt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint16_t
---@param arg2 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint16_t.ne(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint16_t
---@param arg2 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint16_t.ge(arg1, arg2) end

---@param arg1 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint16_t.is_zero(arg1) end

---@param arg1 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint16_t.is_negative(arg1) end

---@param arg1 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint16_t.is_odd(arg1) end

---@param arg1 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.uint16_t.is_positive(arg1) end

---@param arg1 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.uint16_t
function EasyRPGPlayer.uint16_t.abs(arg1) end

---@param arg1 EasyRPGPlayer.uint16_t
---@param arg2 EasyRPGPlayer.uint16_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.uint16_t.mod(arg1, arg2) end

---@return EasyRPGPLayer.uint16_t
function EasyRPGPlayer.uint16_t.min() end

---@return EasyRPGPLayer.uint16_t
function EasyRPGPlayer.uint16_t.max() end

EasyRPGPlayer.uint16_t = EasyRPGPlayer.uint16_t
---@alias EasyRPGPlayer.uint16_t t---@class EasyRPGPlayer.float
EasyRPGPlayer.float = {}
---@overload fun(arg1: EasyRPGPlayer.float):EasyRPGPlayer.float
---@overload fun(arg1: EasyRPGPlayer.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.float
---@overload fun():EasyRPGPlayer.float

function EasyRPGPlayer.float.new(...) end

---@param arg1 EasyRPGPlayer.float
---@param arg2 EasyRPGPlayer.float
---@return EasyRPGPLayer.float
function EasyRPGPlayer.float.add(arg1, arg2) end

---@param arg1 EasyRPGPlayer.float
---@param arg2 EasyRPGPlayer.float
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.float.lt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.float
---@return EasyRPGPLayer.float
function EasyRPGPlayer.float.sin(arg1) end

---@param arg1 EasyRPGPlayer.float
---@param arg2 EasyRPGPlayer.float
---@return EasyRPGPLayer.float
function EasyRPGPlayer.float.sub(arg1, arg2) end

---@param arg1 EasyRPGPlayer.float
---@param arg2 EasyRPGPlayer.float
---@return EasyRPGPLayer.float
function EasyRPGPlayer.float.div(arg1, arg2) end

---@param arg1 EasyRPGPlayer.float
---@return EasyRPGPLayer.float
function EasyRPGPlayer.float.sqrt(arg1) end

---@param arg1 EasyRPGPlayer.float
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.float.is_signed(arg1) end

---@param arg1 EasyRPGPlayer.float
---@return EasyRPGPLayer.float
function EasyRPGPlayer.float.ceil(arg1) end

---@param arg1 EasyRPGPlayer.float
---@param arg2 EasyRPGPlayer.float
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.float.le(arg1, arg2) end

---@return EasyRPGPLayer.float
function EasyRPGPlayer.float.epsilon() end

---@param arg1 EasyRPGPlayer.float
---@return EasyRPGPLayer.float
function EasyRPGPlayer.float.cos(arg1) end

---@param arg1 EasyRPGPlayer.float
---@param arg2 EasyRPGPlayer.float
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.float.gt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.float
---@param arg2 EasyRPGPlayer.float
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.float.ne(arg1, arg2) end

---@param arg1 EasyRPGPlayer.float
---@param arg2 EasyRPGPlayer.float
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.float.ge(arg1, arg2) end

---@param arg1 EasyRPGPlayer.float
---@return EasyRPGPLayer.float
function EasyRPGPlayer.float.tan(arg1) end

---@param arg1 EasyRPGPlayer.float
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.float.is_zero(arg1) end

---@param arg1 EasyRPGPlayer.float
---@param arg2 EasyRPGPlayer.float
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.float.eq(arg1, arg2) end

---@param arg1 EasyRPGPlayer.float
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.float.is_negative(arg1) end

---@param arg1 EasyRPGPlayer.float
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.float.is_positive(arg1) end

---@param arg1 EasyRPGPlayer.float
---@param arg2 EasyRPGPlayer.float
---@return EasyRPGPLayer.float
function EasyRPGPlayer.float.mul(arg1, arg2) end

---@param arg1 EasyRPGPlayer.float
---@return EasyRPGPLayer.float
function EasyRPGPlayer.float.abs(arg1) end

---@param arg1 EasyRPGPlayer.float
---@return EasyRPGPLayer.float
function EasyRPGPlayer.float.floor(arg1) end

---@param arg1 EasyRPGPlayer.float
---@return EasyRPGPLayer.float
function EasyRPGPlayer.float.round(arg1) end

---@param arg1 EasyRPGPlayer.float
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.float.is_nan(arg1) end

---@param arg1 EasyRPGPlayer.float
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.float.is_inf(arg1) end

---@param arg1 EasyRPGPlayer.float
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.float.is_finite(arg1) end

---@return EasyRPGPLayer.float
function EasyRPGPlayer.float.min() end

---@return EasyRPGPLayer.float
function EasyRPGPlayer.float.max() end

EasyRPGPlayer.float = EasyRPGPlayer.float
---@alias EasyRPGPlayer.float f---@class EasyRPGPlayer.ldouble
EasyRPGPlayer.ldouble = {}
---@overload fun(arg1: EasyRPGPlayer.ldouble):EasyRPGPlayer.ldouble
---@overload fun(arg1: EasyRPGPlayer.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.ldouble
---@overload fun():EasyRPGPlayer.ldouble

function EasyRPGPlayer.ldouble.new(...) end

---@param arg1 EasyRPGPlayer.ldouble
---@param arg2 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.ldouble
function EasyRPGPlayer.ldouble.add(arg1, arg2) end

---@param arg1 EasyRPGPlayer.ldouble
---@param arg2 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.ldouble.lt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.ldouble
function EasyRPGPlayer.ldouble.sin(arg1) end

---@param arg1 EasyRPGPlayer.ldouble
---@param arg2 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.ldouble
function EasyRPGPlayer.ldouble.sub(arg1, arg2) end

---@param arg1 EasyRPGPlayer.ldouble
---@param arg2 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.ldouble
function EasyRPGPlayer.ldouble.div(arg1, arg2) end

---@param arg1 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.ldouble
function EasyRPGPlayer.ldouble.sqrt(arg1) end

---@param arg1 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.ldouble.is_signed(arg1) end

---@param arg1 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.ldouble
function EasyRPGPlayer.ldouble.ceil(arg1) end

---@param arg1 EasyRPGPlayer.ldouble
---@param arg2 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.ldouble.le(arg1, arg2) end

---@return EasyRPGPLayer.ldouble
function EasyRPGPlayer.ldouble.epsilon() end

---@param arg1 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.ldouble
function EasyRPGPlayer.ldouble.cos(arg1) end

---@param arg1 EasyRPGPlayer.ldouble
---@param arg2 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.ldouble.gt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.ldouble
---@param arg2 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.ldouble.ne(arg1, arg2) end

---@param arg1 EasyRPGPlayer.ldouble
---@param arg2 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.ldouble.ge(arg1, arg2) end

---@param arg1 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.ldouble
function EasyRPGPlayer.ldouble.tan(arg1) end

---@param arg1 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.ldouble.is_zero(arg1) end

---@param arg1 EasyRPGPlayer.ldouble
---@param arg2 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.ldouble.eq(arg1, arg2) end

---@param arg1 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.ldouble.is_negative(arg1) end

---@param arg1 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.ldouble.is_positive(arg1) end

---@param arg1 EasyRPGPlayer.ldouble
---@param arg2 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.ldouble
function EasyRPGPlayer.ldouble.mul(arg1, arg2) end

---@param arg1 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.ldouble
function EasyRPGPlayer.ldouble.abs(arg1) end

---@param arg1 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.ldouble
function EasyRPGPlayer.ldouble.floor(arg1) end

---@param arg1 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.ldouble
function EasyRPGPlayer.ldouble.round(arg1) end

---@param arg1 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.ldouble.is_nan(arg1) end

---@param arg1 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.ldouble.is_inf(arg1) end

---@param arg1 EasyRPGPlayer.ldouble
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.ldouble.is_finite(arg1) end

---@return EasyRPGPLayer.ldouble
function EasyRPGPlayer.ldouble.min() end

---@return EasyRPGPLayer.ldouble
function EasyRPGPlayer.ldouble.max() end

EasyRPGPlayer.ldouble = EasyRPGPlayer.ldouble
---@alias EasyRPGPlayer.ldouble e---@class EasyRPGPlayer.int32_t
EasyRPGPlayer.int32_t = {}
---@overload fun(arg1: EasyRPGPlayer.int32_t):EasyRPGPlayer.int32_t
---@overload fun(arg1: EasyRPGPlayer.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.int32_t
---@overload fun():EasyRPGPlayer.int32_t

function EasyRPGPlayer.int32_t.new(...) end

---@param arg1 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int32_t.is_even(arg1) end

---@param arg1 EasyRPGPlayer.int32_t
---@param arg2 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.int32_t.add(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int32_t
---@param arg2 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int32_t.lt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int32_t
---@param arg2 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.int32_t.mul(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int32_t
---@param arg2 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.int32_t.sub(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int32_t
---@param arg2 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.int32_t.div(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int32_t.is_signed(arg1) end

---@param arg1 EasyRPGPlayer.int32_t
---@param arg2 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int32_t.eq(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int32_t
---@param arg2 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int32_t.le(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int32_t
---@param arg2 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int32_t.gt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int32_t
---@param arg2 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int32_t.ne(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int32_t
---@param arg2 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int32_t.ge(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int32_t.is_zero(arg1) end

---@param arg1 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int32_t.is_negative(arg1) end

---@param arg1 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int32_t.is_odd(arg1) end

---@param arg1 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int32_t.is_positive(arg1) end

---@param arg1 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.int32_t.abs(arg1) end

---@param arg1 EasyRPGPlayer.int32_t
---@param arg2 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.int32_t.mod(arg1, arg2) end

---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.int32_t.min() end

---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.int32_t.max() end

EasyRPGPlayer.int32_t = EasyRPGPlayer.int32_t
---@alias EasyRPGPlayer.int32_t i---@class EasyRPGPlayer.StdClock
EasyRPGPlayer.StdClock = {}

---@return EasyRPGPLayer.PKc
function EasyRPGPlayer.StdClock.Name() end

---@return EasyRPGPLayer.NSt3__16chrono10time_pointINS0_12steady_clockENS0_8durationIxNS_5ratioILl1ELl1000000000EEEEEEE
function EasyRPGPlayer.StdClock.now() end

EasyRPGPlayer.StdClock = EasyRPGPlayer.StdClock
---@alias EasyRPGPlayer.StdClock 8StdClock---@class EasyRPGPlayer.int16_t
EasyRPGPlayer.int16_t = {}
---@overload fun(arg1: EasyRPGPlayer.int16_t):EasyRPGPlayer.int16_t
---@overload fun(arg1: EasyRPGPlayer.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.int16_t
---@overload fun():EasyRPGPlayer.int16_t

function EasyRPGPlayer.int16_t.new(...) end

---@param arg1 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int16_t.is_even(arg1) end

---@param arg1 EasyRPGPlayer.int16_t
---@param arg2 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.i
function EasyRPGPlayer.int16_t.add(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int16_t
---@param arg2 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int16_t.lt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int16_t
---@param arg2 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.i
function EasyRPGPlayer.int16_t.mul(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int16_t
---@param arg2 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.i
function EasyRPGPlayer.int16_t.sub(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int16_t
---@param arg2 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.i
function EasyRPGPlayer.int16_t.div(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int16_t.is_signed(arg1) end

---@param arg1 EasyRPGPlayer.int16_t
---@param arg2 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int16_t.eq(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int16_t
---@param arg2 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int16_t.le(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int16_t
---@param arg2 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int16_t.gt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int16_t
---@param arg2 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int16_t.ne(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int16_t
---@param arg2 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int16_t.ge(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int16_t.is_zero(arg1) end

---@param arg1 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int16_t.is_negative(arg1) end

---@param arg1 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int16_t.is_odd(arg1) end

---@param arg1 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int16_t.is_positive(arg1) end

---@param arg1 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.i
function EasyRPGPlayer.int16_t.abs(arg1) end

---@param arg1 EasyRPGPlayer.int16_t
---@param arg2 EasyRPGPlayer.int16_t
---@return EasyRPGPLayer.i
function EasyRPGPlayer.int16_t.mod(arg1, arg2) end

---@return EasyRPGPLayer.int16_t
function EasyRPGPlayer.int16_t.min() end

---@return EasyRPGPLayer.int16_t
function EasyRPGPlayer.int16_t.max() end

EasyRPGPlayer.int16_t = EasyRPGPlayer.int16_t
---@alias EasyRPGPlayer.int16_t s---@class EasyRPGPlayer.int8_t
EasyRPGPlayer.int8_t = {}
---@overload fun(arg1: EasyRPGPlayer.int8_t):EasyRPGPlayer.int8_t
---@overload fun(arg1: EasyRPGPlayer.NSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEE):EasyRPGPlayer.int8_t
---@overload fun():EasyRPGPlayer.int8_t

function EasyRPGPlayer.int8_t.new(...) end

---@param arg1 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int8_t.is_even(arg1) end

---@param arg1 EasyRPGPlayer.int8_t
---@param arg2 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.i
function EasyRPGPlayer.int8_t.add(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int8_t
---@param arg2 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int8_t.lt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int8_t
---@param arg2 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.i
function EasyRPGPlayer.int8_t.mul(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int8_t
---@param arg2 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.i
function EasyRPGPlayer.int8_t.sub(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int8_t
---@param arg2 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.i
function EasyRPGPlayer.int8_t.div(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int8_t.is_signed(arg1) end

---@param arg1 EasyRPGPlayer.int8_t
---@param arg2 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int8_t.eq(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int8_t
---@param arg2 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int8_t.le(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int8_t
---@param arg2 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int8_t.gt(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int8_t
---@param arg2 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int8_t.ne(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int8_t
---@param arg2 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int8_t.ge(arg1, arg2) end

---@param arg1 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int8_t.is_zero(arg1) end

---@param arg1 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int8_t.is_negative(arg1) end

---@param arg1 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int8_t.is_odd(arg1) end

---@param arg1 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.b
function EasyRPGPlayer.int8_t.is_positive(arg1) end

---@param arg1 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.i
function EasyRPGPlayer.int8_t.abs(arg1) end

---@param arg1 EasyRPGPlayer.int8_t
---@param arg2 EasyRPGPlayer.int8_t
---@return EasyRPGPLayer.i
function EasyRPGPlayer.int8_t.mod(arg1, arg2) end

---@return EasyRPGPLayer.int8_t
function EasyRPGPlayer.int8_t.min() end

---@return EasyRPGPLayer.int8_t
function EasyRPGPlayer.int8_t.max() end

EasyRPGPlayer.int8_t = EasyRPGPlayer.int8_t
---@alias EasyRPGPlayer.int8_t a---@diagnostic disable: missing-return
leasy = {}

leasy = {}
---@diagnostic disable: missing-return
meta2 = {}

meta2 = {}
---@diagnostic disable: missing-return
node = {}

node = {}
---@class EasyRPGPlayer.leasy.meta2.node.Node2D
EasyRPGPlayer.leasy.meta2.node.Node2D = {}
---@overload fun():EasyRPGPlayer.leasy::meta2::node::Node2D
---@overload fun(arg1: EasyRPGPlayer.int32_t, arg2: EasyRPGPlayer.int32_t):EasyRPGPlayer.leasy::meta2::node::Node2D

function EasyRPGPlayer.leasy.meta2.node.Node2D.new(...) end

---@param arg1 EasyRPGPlayer.PN5leasy5meta24node6Node2DE
---@return EasyRPGPLayer.N5leasy5meta24node6Node2D8positionE
function EasyRPGPlayer.leasy.meta2.node.Node2D.pos(arg1) end

---@param arg1 EasyRPGPlayer.PN5leasy5meta24node6Node2DE
---@param arg2 EasyRPGPlayer.int32_t
---@param arg3 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.void
function EasyRPGPlayer.leasy.meta2.node.Node2D.move(arg1, arg2, arg3) end

EasyRPGPlayer.leasy.meta2.node.Node2D = EasyRPGPlayer.leasy.meta2.node.Node2D
---@alias EasyRPGPlayer.leasy.meta2.node.Node2D N5leasy5meta24node6Node2DE---@class EasyRPGPlayer.leasy.meta2.node.Node
EasyRPGPlayer.leasy.meta2.node.Node = {}

---@param arg1 EasyRPGPlayer.PN5leasy5meta24node4NodeE
---@return EasyRPGPLayer.void
function EasyRPGPlayer.leasy.meta2.node.Node.ready(arg1) end

---@return EasyRPGPLayer.leasy::meta2::node::Node
function EasyRPGPlayer.leasy.meta2.node.Node.new() end

---@param arg1 EasyRPGPlayer.PN5leasy5meta24node4NodeE
---@param arg2 EasyRPGPlayer.double
---@return EasyRPGPLayer.void
function EasyRPGPlayer.leasy.meta2.node.Node.update(arg1, arg2) end

---@param arg1 EasyRPGPlayer.PN5leasy5meta24node4NodeE
---@return EasyRPGPLayer.N5leasy4iky76CursorINSt3__111__wrap_iterIPNS2_10shared_ptrINS_5meta24node4NodeEEEEEEE
function EasyRPGPlayer.leasy.meta2.node.Node.visit(arg1) end

---@param arg1 EasyRPGPlayer.PN5leasy5meta24node4NodeE
---@return EasyRPGPLayer.void
function EasyRPGPlayer.leasy.meta2.node.Node.draw(arg1) end

---@param arg1 EasyRPGPlayer.PN5leasy5meta24node4NodeE
---@return EasyRPGPLayer.NSt3__16vectorINS_10shared_ptrIN5leasy5meta24node4NodeEEENS_9allocatorIS6_EEEE
function EasyRPGPlayer.leasy.meta2.node.Node.children(arg1) end

EasyRPGPlayer.leasy.meta2.node.Node = EasyRPGPlayer.leasy.meta2.node.Node
---@alias EasyRPGPlayer.leasy.meta2.node.Node N5leasy5meta24node4NodeE---@diagnostic disable: missing-return
Algo = {}

Algo = {}

---@param arg1 EasyRPGPlayer.N3lcf3rpg5SkillE
---@param arg2 EasyRPGPlayer.int32_t
---@param arg3 EasyRPGPlayer.bool
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.Algo.CalcSkillCost(arg1, arg2, arg3) end

---@param arg1 EasyRPGPlayer.N3lcf3rpg5SkillE
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.Algo.SkillTargetsEnemies(arg1) end
---@overload fun(arg1: EasyRPGPlayer.N3lcf3rpg9SaveActor7RowTypeE, arg2: EasyRPGPlayer.N3lcf3rpg6System15BattleConditionE, arg3: EasyRPGPlayer.bool):EasyRPGPlayer.bool
---@overload fun(arg1: EasyRPGPlayer.12Game_Battler, arg2: EasyRPGPlayer.N3lcf3rpg6System15BattleConditionE, arg3: EasyRPGPlayer.bool, arg4: EasyRPGPlayer.bool):EasyRPGPlayer.bool

function EasyRPGPlayer.Algo.IsRowAdjusted(...) end

---@param arg1 EasyRPGPlayer.int32_t
---@param arg2 EasyRPGPlayer.12Game_Battler
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.Algo.AdjustDamageForDefend(arg1, arg2) end

---@param arg1 EasyRPGPlayer.12Game_Battler
---@param arg2 EasyRPGPlayer.12Game_Battler
---@param arg3 EasyRPGPlayer.bool
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.Algo.CalcSelfDestructEffect(arg1, arg2, arg3) end

---@param arg1 EasyRPGPlayer.12Game_Battler
---@param arg2 EasyRPGPlayer.12Game_Battler
---@param arg3 EasyRPGPlayer.N12Game_Battler6WeaponE
---@param arg4 EasyRPGPlayer.bool
---@param arg5 EasyRPGPlayer.bool
---@param arg6 EasyRPGPlayer.bool
---@param arg7 EasyRPGPlayer.N3lcf3rpg6System15BattleConditionE
---@param arg8 EasyRPGPlayer.bool
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.Algo.CalcNormalAttackEffect(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) end

---@param arg1 EasyRPGPlayer.int32_t
---@param arg2 EasyRPGPlayer.N3lcf3rpg4ItemE
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.Algo.GetNumberOfAttacks(arg1, arg2) end

---@param arg1 EasyRPGPlayer.12Game_Battler
---@param arg2 EasyRPGPlayer.12Game_Battler
---@param arg3 EasyRPGPlayer.N3lcf3rpg5SkillE
---@param arg4 EasyRPGPlayer.N3lcf3rpg6System15BattleConditionE
---@param arg5 EasyRPGPlayer.bool
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.Algo.CalcSkillToHit(arg1, arg2, arg3, arg4, arg5) end

---@param arg1 EasyRPGPlayer.N3lcf3rpg5SkillE
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.Algo.SkillTargetsAllies(arg1) end

---@param arg1 EasyRPGPlayer.N3lcf3rpg5SkillE
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.Algo.IsNormalOrSubskill(arg1) end

---@param arg1 EasyRPGPlayer.N3lcf3rpg5SkillE
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.Algo.SkillTargetsOne(arg1) end

---@param arg1 EasyRPGPlayer.N3lcf3rpg5SkillE
---@param arg2 EasyRPGPlayer.bool
---@return EasyRPGPLayer.bool
function EasyRPGPlayer.Algo.IsSkillUsable(arg1, arg2) end

---@param arg1 EasyRPGPlayer.12Game_Battler
---@param arg2 EasyRPGPlayer.12Game_Battler
---@param arg3 EasyRPGPlayer.N12Game_Battler6WeaponE
---@param arg4 EasyRPGPlayer.N3lcf3rpg6System15BattleConditionE
---@param arg5 EasyRPGPlayer.bool
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.Algo.CalcNormalAttackToHit(arg1, arg2, arg3, arg4, arg5) end

---@param arg1 EasyRPGPlayer.N3lcf3rpg5SkillE
---@param arg2 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.Algo.CalcSkillHpCost(arg1, arg2) end

---@param arg1 EasyRPGPlayer.12Game_Battler
---@param arg2 EasyRPGPlayer.12Game_Battler
---@param arg3 EasyRPGPlayer.N12Game_Battler6WeaponE
---@param arg4 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.Algo.CalcCriticalHitChance(arg1, arg2, arg3, arg4) end

---@param arg1 EasyRPGPlayer.12Game_Battler
---@param arg2 EasyRPGPlayer.12Game_Battler
---@param arg3 EasyRPGPlayer.N3lcf3rpg5SkillE
---@param arg4 EasyRPGPlayer.bool
---@param arg5 EasyRPGPlayer.bool
---@param arg6 EasyRPGPlayer.N3lcf3rpg6System15BattleConditionE
---@param arg7 EasyRPGPlayer.bool
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.Algo.CalcSkillEffect(arg1, arg2, arg3, arg4, arg5, arg6, arg7) end

---@param arg1 EasyRPGPlayer.int32_t
---@param arg2 EasyRPGPlayer.int32_t
---@return EasyRPGPLayer.int32_t
function EasyRPGPlayer.Algo.VarianceAdjustEffect(arg1, arg2) end
