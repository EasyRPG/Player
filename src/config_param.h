/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EP_CONFIG_PARAM_H
#define EP_CONFIG_PARAM_H

#include <cstdint>
#include <limits>
#include <string>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <lcf/flag_set.h>

/** A configuration parameter with no restrictions */
template <typename T>
class ConfigParam {
public:
	using value_type = T;

	explicit ConfigParam(T value = {}) : _value(std::move(value)) {}

	const T& Get() const { return _value; }

	bool Set(T value) {
		if (IsValid(value)) {
			_value = std::move(value);
			return true;
		}
		return false;
	}

	bool IsValid(const T&) const {
		return true;
	}

	bool Enabled() const {
		return true;
	}

	bool Locked() const {
		return false;
	}

private:
	T _value = {};
};

using StringConfigParam = ConfigParam<std::string>;

/** A configuration parameter with a range */
template <typename T>
class RangeConfigParam {
public:
	using value_type = T;

	/** Construct with name and initial value */
	constexpr explicit RangeConfigParam(T value = {})
		: _value(value) {}

	/** Construct with name and initial value, min, and max */
	constexpr RangeConfigParam(T value, T minval, T maxval)
		: _value(value) { SetRange(minval, maxval); }

	/**
	 * Attempts to change the value to value. If IsValid(value), the new value
	 * is set the returns true.
	 * @param value the new value to set
	 * @return true if value was set, false otherwise
	 */
	constexpr bool Set(T value) {
		if (IsValid(value)) {
			_value = std::move(value);
			return true;
		}
		return false;
	}

	/** @return if Enabled(), return the current value. Otherwise the result is undefined. */
	constexpr T Get() const { return _value;  }

	/**
	 * Check if a value is valid
	 * @param value the value to check
	 * @return true if this value can be set
	 */
	constexpr bool IsValid(T value) const {
		return value >= _min && value <= _max;
	}

	/** @return true if this parameter can take a value, or false if disabled */
	constexpr bool Enabled() const {
		return _min <= _max;
	}

	/** @return true if this parameter is either disabled or locked to a single value and cannot be changed */
	constexpr bool Locked() const {
		return _min >= _max;
	}

	/** 
	 * Set minimum allowed value.
	 * @param minval the new minimum
	 * @post If the current value is < minval, it will be set equal to minval
	 * @post If the current maximum < minval, this parameter is disabled.
	 */
	constexpr void SetMin(T minval) { SetRange(minval, _max); }

	/** 
	 * Set maximum allowed value.
	 * @param maxval the new maximum
	 * @post If the current value is > maxval, it will be set equal to maxval
	 * @post If the current minimum > maxval, this parameter is disabled.
	 */
	constexpr void SetMax(T maxval) { SetRange(_min, maxval); }

	/** 
	 * Set allowed range of values.
	 * @param minval the new maximum
	 * @param maxval the new maximum
	 * @post If the current value is outside the range, it will be clamped.
	 * @post If the minval > maxval, this parameter is disabled.
	 */
	constexpr void SetRange(T minval, T maxval) {
		_min = minval;
		_max = maxval;
		_value = (_value < _min) ? _min : _value;
		_value = (_value > _max) ? _max : _value;
	}

	/** Disable this parameter, not allowing it to take on any valid values */
	constexpr void Disable() {
		_value = {};
		_min = std::numeric_limits<T>::max();
		_max = std::numeric_limits<T>::min();
	}

	/**
	 * Lock the parameter to value.
	 * @param value the value to lock to
	 */
	constexpr void Lock(T value) {
		_value = _min = _max = value;
	}
private:
	// FIXME: Storage can be optimized to 1 byte for bool case.
	T _value = {};
	T _min = std::numeric_limits<T>::min();
	T _max = std::numeric_limits<T>::max();
};

using IntConfigParam = RangeConfigParam<int>;
using LongConfigParam = RangeConfigParam<long>;
using Int32ConfigParam = RangeConfigParam<int32_t>;
using Int64ConfigParam = RangeConfigParam<int64_t>;
using FloatConfigParam = RangeConfigParam<float>;
using DoubleConfigParam = RangeConfigParam<double>;

/** A boolean configuration parameter */
class BoolConfigParam {
public:
	using value_type = bool;

	explicit constexpr BoolConfigParam(bool value = false) : _value(value) {}

	constexpr bool Get() const { return _value & 1; }

	constexpr bool Set(bool value) {
		if (IsValid(value)) {
			_value = value + (_value & 0b110);
			return true;
		}
		return false;
	}

	constexpr bool IsValid(bool value) const {
		return !(_value & (1 << (1 + value)));
	}

	constexpr bool Enabled() const {
		return (_value >> 1) != 0b11;
	}

	constexpr bool Locked() const {
		return (_value >> 1) != 0b00;
	}

	constexpr void Lock(bool value) {
		_value = value + (1 << (2 - value));
	}

	constexpr void Disable() {
		_value = 0b110;
	}

private:
	//1st bit: value, 2nd bit: false disabled, 3rd bit: true disabled
	uint8_t _value = 0;
};

/** A ConfigParam for any type which is limited to a fixed set of values */
template <typename T>
class SetConfigParam {
public:
	using value_type = T;

	explicit SetConfigParam(T value = {}) : _value{ value }, _valid{ {value} } {}
	SetConfigParam(T value, std::initializer_list<T> valid) : _value{ value }, _valid{ valid } { AddToValidSet(_value); }

	bool Set(T value) {
		if (IsValid(value)) {
			_value = value;
			return true;
		}
		return false;
	}

	const T& Get() const {
		return _value;
	}

	bool IsValid(const T& value) const {
		auto iter = std::find(_valid.begin(), _valid.end(), value);
		return iter != _valid.end();
	}

	bool Enabled() const {
		return !_valid.empty();
	}

	bool Locked() const {
		return _valid.size() <= 1;
	}

	void ReplaceValidSet(std::vector<T> v) {
		_valid = std::move(v);
		if (!_valid.empty() && !IsValid(_value)) {
			_value = _valid.front();
		}
	}

	void AddToValidSet(T value) {
		auto iter = std::find(_valid.begin(), _valid.end(), value);
		if (iter != _valid.end()) {
			_valid.push_back(std::move(value));
		}
	}

	void RemoveFromValidSet(const T& value) {
		auto iter = std::find(_valid.begin(), _valid.end(), value);
		if (iter != _valid.end()) {
			iter = _valid.erase(iter);
			if (!_valid.empty() && value == _value) {
				_value = _valid.front();
			}
		}
	}

	void Disable() {
		_valid.clear();
	}

	void Lock(T value) {
		_value = value;
		_valid = { _value };
	}
private:
	T _value = {};
	std::vector<T> _valid = {};
};

template <typename E>
class EnumConfigParam {
public:
	using value_type = E;

	explicit EnumConfigParam(E value = {}) : _value{ value } {}
	EnumConfigParam(E value, std::initializer_list<E> valid) : _value{ value }, _valid{ valid } { _valid[_value] = true; }

	bool Set(E value) {
		if (IsValid(value)) {
			_value = value;
			return true;
		}
		return false;
	}

	const E& Get() const {
		return _value;
	}

	bool IsValid(const E& value) const {
		return _valid[value];
	}

	bool Enabled() const {
		return _valid.any();
	}

	bool Locked() const {
		return _valid.count() <= 1;
	}

	void ReplaceValidSet(lcf::FlagSet<E> valid) {
		_valid = std::move(valid);
		if (Enabled() && !IsValid(_value)) {
			_value = GetFirstValid();
		}
	}

	void AddToValidSet(E value) {
		_valid[value] = true;
	}

	void RemoveFromValidSet(const E& value) {
		_valid[value] = false;
		if (Enabled() && !IsValid(_value)) {
			_value = GetFirstValid();
		}
	}

	void Disable() {
		_valid = {};
	}

	void Lock(E value) {
		_value = value;
		_valid = {};
		_valid[value] = true;
	}
private:
	E _value = {};
	lcf::FlagSet<E> _valid = ~lcf::FlagSet<E>();

	E GetFirstValid() const {
		for (size_t i = 0; i < _valid.size(); ++i) {
			auto e = static_cast<E>(i);
			if (_valid[e]) {
				return e;
			}
		}
		return E{};
	}
};

// Type trait which detects whether CP meets the ConfigParam concept
template <typename CP>
struct IsConfigParamT {
private:
	static std::false_type test(...);
	template <typename U,
			 typename T = typename U::value_type,
			 typename = decltype(std::declval<U>().Get()),
			 typename = decltype(std::declval<U>().Set(std::declval<T>())),
			 typename = decltype(std::declval<U>().IsValid(std::declval<T>())),
			 typename = decltype(std::declval<U>().Enabled()),
			 typename = decltype(std::declval<U>().Locked())
				 > static std::true_type test(U&&);
public:
	static constexpr auto value = decltype(test(std::declval<CP>()))::value;
};

#endif
