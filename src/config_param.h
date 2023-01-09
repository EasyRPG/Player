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

#include "string_view.h"
#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <lcf/flag_set.h>

namespace {
	inline const std::string& ParamValueToString(const std::string& s) {
		return s;
	}

	inline std::string ParamValueToString(int i) {
		return std::to_string(i);
	}
}

template <typename T>
class ConfigParamBase {
public:
	using value_type = T;

	ConfigParamBase(StringView name, StringView description, T value) : _name(name), _description(description), _value(value) {}

	T Get() const { return _value; }

	bool Set(const T& value) {
		if (IsLocked()) {
			return false;
		}

		if (IsValid(value)) {
			_value = std::move(value);
			return true;
		}
		return false;
	}

	bool IsValid(const T& value) const {
		if (!IsOptionVisible()) {
			return false;
		}

		if (IsLocked()) {
			return value == this->_value;
		}

		return vIsValid(value);
	}

	virtual bool vIsValid(const T& value) const = 0;

	bool IsOptionVisible() const {
		return _visible;
	}

	void SetOptionVisible(bool visible) {
		_visible = visible;
	}

	bool IsLocked() const {
		return _locked;
	}

	bool Lock(T value) {
		_locked = false;

		if (!Set(value)) {
			_locked = true;
			return false;
		}

		_locked = true;
		return true;
	}

	void SetLocked(bool locked) {
		_locked = locked;
	}

	StringView GetName() const {
		return _name;
	}

	StringView GetDescription() const {
		return _description;
	}

	virtual std::string ValueToString() const = 0;

protected:
	T _value = {};
	StringView _name;
	StringView _description;

private:
	bool _visible = true;
	bool _locked = false;
};

/** A configuration parameter with no restrictions */
template <typename T>
class ConfigParam : public ConfigParamBase<T> {
public:
	explicit ConfigParam(StringView name, StringView description, T value = {}) :
		ConfigParamBase<T>(name, description, std::move(value)) {}

	bool vIsValid(const T&) const override {
		return true;
	}

	std::string ValueToString() const override {
		return ParamValueToString(this->Get());
	}
};

/** A configuration parameter which is locked by default */
template <typename T>
class LockedConfigParam final : public ConfigParam<T> {
public:
    explicit LockedConfigParam(StringView name, StringView description, T value = {}) :
		ConfigParam<T>(name, description, value) {
		this->Lock(value);
	}
};

using StringConfigParam = ConfigParam<std::string>;

/** A configuration parameter with a range */
template <typename T>
class RangeConfigParam : public ConfigParamBase<T> {
public:
	/** Construct with name and initial value */
	explicit RangeConfigParam(StringView name, StringView description, T value = {}) :
		ConfigParamBase<T>(name, description, std::move(value)) {}

	/** Construct with name and initial value, min, and max */
	RangeConfigParam(StringView name, StringView description, T value, T minval, T maxval) :
		ConfigParamBase<T>(name, description, std::move(value)) { SetRange(minval, maxval); }

	/**
	 * Check if a value is valid
	 * @param value the value to check
	 * @return true if this value can be set
	 */
	bool vIsValid(const T& value) const override {
		return value >= _min && value <= _max;
	}

	T GetMin() const {
		return _min;
	}

	T GetMax() const {
		return _max;
	}

	/**
	 * Set minimum allowed value.
	 * @param minval the new minimum
	 * @post If the current value is < minval, it will be set equal to minval
	 * @post If the current maximum < minval, this parameter is disabled.
	 */
	void SetMin(T minval) { SetRange(minval, _max); }

	/**
	 * Set maximum allowed value.
	 * @param maxval the new maximum
	 * @post If the current value is > maxval, it will be set equal to maxval
	 * @post If the current minimum > maxval, this parameter is disabled.
	 */
	void SetMax(T maxval) { SetRange(_min, maxval); }

	/**
	 * Set allowed range of values.
	 * @param minval the new maximum
	 * @param maxval the new maximum
	 * @post If the current value is outside the range, it will be clamped.
	 * @post If the minval > maxval, this parameter is disabled.
	 */
	void SetRange(T minval, T maxval) {
		this->SetOptionVisible(true);
		_min = minval;
		_max = maxval;
		this->_value = (this->_value < _min) ? _min : this->_value;
		this->_value = (this->_value > _max) ? _max : this->_value;
	}

	std::string ValueToString() const override {
		return ParamValueToString(this->_value);
	}

private:
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
class BoolConfigParam : public ConfigParamBase<bool> {
public:
	explicit BoolConfigParam(StringView name, StringView description, bool value = false)  :
		ConfigParamBase<bool>(name, description, value) {}

	bool vIsValid(const bool&) const override {
		return true;
	}

	void Toggle() {
		if (Get()) {
			Set(false);
		} else {
			Set(true);
		}
	}

	std::string ValueToString() const override {
		return Get() ? "[ON]" : "[OFF]";
	}
};

template <typename E, size_t S>
class EnumConfigParam : public ConfigParamBase<E> {
public:
	EnumConfigParam(StringView name, StringView description, E value, std::array<StringView, S> values, std::array<StringView, S> value_descriptions) :
		ConfigParamBase<E>(name, description, value), _values{ values }, _value_descriptions{ value_descriptions } {
		for (size_t i = 0; i < S; ++i) {
			_valid[static_cast<E>(S)] = true;
		}
	}

	bool vIsValid(const E& value) const override {
		return _valid[value];
	}

	void ReplaceValidSet(lcf::FlagSet<E> valid) {
		_valid = std::move(valid);
		if (this->IsOptionVisible() && !this->IsValid(this->_value)) {
			this->_value = GetFirstValid();
		}
	}

	void AddToValidSet(E value) {
		_valid[value] = true;
	}

	void RemoveFromValidSet(const E& value) {
		_valid[value] = false;
		if (this->IsOptionVisible() && !this->IsValid(this->_value)) {
			this->_value = GetFirstValid();
		}
	}

	std::string ValueToString() const override {
		return ToString(_values[static_cast<int>(this->_value)]);
	}

	std::array<StringView, S> GetValues() const {
		return _values;
	}

	std::array<StringView, S> GetDescriptions() const {
		return _value_descriptions;
	}

private:
	lcf::FlagSet<E> _valid = ~lcf::FlagSet<E>();
	std::array<StringView, S> _values;
	std::array<StringView, S> _value_descriptions;
	bool _enabled = true;

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

#endif
