#pragma once

/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#include <imagine/util/concepts.hh>
#ifndef IG_USE_MODULE_STD
#include <utility>
#include <new>
#include <cstddef>
#include <array>
#endif

namespace IG
{

inline constexpr struct DelegateFuncDefaultInit{} delegateFuncDefaultInit;

template <size_t StorageSize, size_t Align, class Func> class DelegateFuncBase;

template <size_t StorageSize, size_t Align, class R, class... Args>
class DelegateFuncBase<StorageSize, Align, R(Args...)>
{
public:
	using FreeFuncPtr = R (*)(Args...);

	constexpr DelegateFuncBase() = default;

	constexpr DelegateFuncBase(std::nullptr_t) {}

	constexpr DelegateFuncBase(DelegateFuncDefaultInit):
		DelegateFuncBase{[](Args...){ return R(); }} {}

	template<CallableClass<R, Args...> F>
	requires (sizeof(F) <= StorageSize && Align >= std::alignment_of_v<F>)
	constexpr DelegateFuncBase(F const &funcObj) :
		exec
		{
			[](const Storage &funcObj, Args ...args) -> R
			{
				return ((F*)funcObj.data())->operator()(std::forward<Args>(args)...);
			}
		}
	{
		// construct from lambda/function object
		new (store.data()) F(funcObj);
	}

#ifdef IG_DELEGATE_FUNC_POINTER_SUPPORT
	constexpr DelegateFuncBase(CallableFunctionPointer<R, Args...> auto const &funcObj)
		requires (StorageSize >= sizeof(void*) && Align >= sizeof(void*)):
		exec
		{
			[](const Storage &funcObj, Args ...args) -> R
			{
				return (*((FreeFuncPtr*)funcObj.data()))(std::forward<Args>(args)...);
			}
		}
	{
		// construct from free function
		new (store.data()) FreeFuncPtr(funcObj);
	}
#endif

	explicit constexpr operator bool() const
	{
		return exec;
	}

	template<class... CallArgs>
	constexpr R operator()(CallArgs&&... args) const
		requires ValidInvokeArgs<FreeFuncPtr, CallArgs...>
	{
		return exec(store, std::forward<CallArgs>(args)...);
	}

	constexpr bool operator ==(DelegateFuncBase const&) const = default;
	constexpr bool equalsByFunc(DelegateFuncBase const& other) const { return exec == other.exec; }

	template<class... CallArgs>
	constexpr R callCopy(CallArgs&&... args) const
	{
		// Call a copy to avoid trashing captured variables
		// if delegate's function can modify the delegate
		return ({auto copy = *this; copy;})(std::forward<CallArgs>(args)...);
	}

	template<class... CallArgs>
	constexpr R callSafe(CallArgs&&... args) const
	{
		if(exec)
			return this->operator()(std::forward<CallArgs>(args)...);
		return R();
	}

	template<class... CallArgs>
	constexpr R callCopySafe(CallArgs&&... args) const
	{
		if(exec)
			return callCopy(std::forward<CallArgs>(args)...);
		return R();
	}

private:
	using Storage = std::array<unsigned char, StorageSize>;

	alignas(Align) Storage store{};
	R (*exec)(const Storage &, Args...){};
};

template <size_t StorageSize, size_t Align, class Func>
using DelegateFuncA = DelegateFuncBase<StorageSize, Align, Func>;

template <size_t StorageSize, class Func>
using DelegateFuncS = DelegateFuncBase<StorageSize, sizeof(void*), Func>;

template <class Func>
using DelegateFunc = DelegateFuncBase<sizeof(void*)*2, sizeof(void*), Func>;

}
