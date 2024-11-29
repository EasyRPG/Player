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

#ifndef EP_CALLBACK_H
#define EP_CALLBACK_H

#include <lcf/scope_guard.h>
#include <vector>
#include <functional>
#include <utility>
#include <algorithm>

#if 0
Where to place a callback:
When it is related to something global put the callback where it is suitable.
When it is related to an instance of a class/struct add it as a public variable.

Adding a new callback:

Assuming you want to add a notification when a variable is changed.

In the header of Game_Variables (game_variables.h) add a public field

```cpp
Callback<int /* variable_id */, int /* value */> OnVariableChanged;
```

The /* names */ are for increated readability.
Functions that take two integer arguments can be bound to this callback.

Signal the callback at a suitable location.
Here this would be in SetOp in game_variables.cpp:

```cpp
OnVariableChanged.Call(variable_id, v);
```

This invocation will trigger all functions attached with Bind.

Attaching to the callback:

```cpp
// Attaching with a scope guard (the callback will automatically Unbind when the
// guard is not in scope anymore).
// This prevents stale handlers that crash.
auto guard = Main_Data::game_variables->OnVariableChanged.Bind([](int var_id, int val) {
	Output::Warning("{} = {}", var_id, val);
});

// In case of a class the scope guard should be a field.
// It is automatically unbound when the instance is destroyed.
class MyClass {
public:
	MyClass() {
		guard = Main_Data::game_variables->OnVariableChanged.Bind([](int var_id, int val) {
			Output::Warning("{} = {}", var_id, val);
		});

		// Alternative: Binding a method instead of a lambda
		guard = Main_Data::game_variables->OnVariableChanged.Bind(&MyClass::VariableChanged, this);
	}

	void VariableChanged(int var_id, int val) {
		Output::Warning("{} = {}", var_id, val);
	}

private:
	BindingScopeGuard guard;
}
```

Not recommended:

If you want to (for whatever reason) manually manage the lifetime of the binding
use BindUnmanaged.

This works the same as described above but the function returns an integer.

And you must that integer to Unbind(id) to remove the handler.
#endif

using BindingScopeGuard = lcf::ScopeGuard<std::function<void()>>;

template</*typename Ret, */typename ...Args>
class Callback {
public:
	using Ret = void;
	using Fn = std::function<Ret(Args...)>;

	template<typename T>
	int BindUnmanaged(Ret (T::*func)(Args...), T* that, Args... args) {
		Fn f = std::bind(std::mem_fn(func), that, std::placeholders::_1, args...);
		return BindUnmanaged(f);
	}

	int BindUnmanaged(Fn func) {
		listeners.push_back({next_id, func});
		return next_id++;
	}

	template<typename T>
	BindingScopeGuard Bind(Ret (T::*func)(Args...), T* that, Args... args) {
		int id = BindUnmanaged(func, that, args...);

		return lcf::ScopeGuard<std::function<void()>>([this, id=id]() {
			Unbind(id);
		});
	}

	BindingScopeGuard Bind(Fn func) {
		int id = BindUnmanaged(func);

		return lcf::ScopeGuard<std::function<void()>>([this, id]() {
			Unbind(id);
		});
	}

	void Unbind(int id) {
		auto it = std::find_if(listeners.begin(), listeners.end(), [id](auto& listener){
			return listener.first == id;
		});
		assert(it != listeners.end());

		listeners.erase(it);
	}

	void Call(Args... args) {
		for (auto& listener: listeners) {
			listener.second(std::forward<Args>(args)...);
		}
	}

	void Clear() {
		listeners.clear();
	}

private:
	std::vector<std::pair<int, Fn>> listeners;

	int next_id = 1;
};

#endif
