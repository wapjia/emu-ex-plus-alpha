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

#include <imagine/util/DelegateFunc.hh>
#include <imagine/util/algorithm.h>
#include <imagine/util/concepts.hh>
#ifndef IG_USE_MODULE_STD
#include <flat_set>
#endif

namespace IG
{

enum class DelegateFuncEqualsMode: uint8_t {normal, byFunc};

template <class Delegate>
class DelegateFuncSet
{
public:
	constexpr DelegateFuncSet() = default;

	bool insert(Delegate del, int priority = 0, InsertMode mode = InsertMode::normal)
	{
		if(mode == InsertMode::unique)
		{
			if(contains(del))
				return false;
			delegates.emplace(del, priority);
			return true;
		}
		else
		{
			delegates.emplace(del, priority);
			return true;
		}
	}

	bool removeFirst(Delegate del, DelegateFuncEqualsMode mode = DelegateFuncEqualsMode::normal)
	{
		if(mode == DelegateFuncEqualsMode::byFunc)
		{
			return eraseFirst(delegates, [&](const auto& e){ return e.del.equalsByFunc(del); });
		}
		return eraseFirst(delegates, del);
	}

	bool contains(Delegate del) const
	{
		return std::ranges::contains(delegates, del);
	}

	auto size() const
	{
		return delegates.size();
	}

	void runAll(Callable<bool, Delegate> auto&& exec, bool skipRemovedDelegates = false)
	{
		auto delegatesSize = size();
		if(!delegatesSize)
			return;
		DelegateEntry delegateCopy[delegatesSize];
		std::copy_n(delegates.begin(), delegatesSize, delegateCopy);
		for(auto &d : delegateCopy)
		{
			if(skipRemovedDelegates && !contains(d.del))
			{
				continue;
			}
			if(!exec(d.del))
			{
				eraseFirst(delegates, d);
			}
		}
	}

protected:
	struct DelegateEntry
	{
		Delegate del{};
		int priority{};

		constexpr DelegateEntry() = default;
		constexpr DelegateEntry(Delegate del, int priority = 0):
			del{del}, priority{priority} {}
		bool operator==(const DelegateEntry& rhs) const { return del == rhs.del; }
		bool operator==(const Delegate& rhs) const { return del == rhs; }
		bool operator<(const DelegateEntry& rhs) const { return priority < rhs.priority; }
	};

	std::flat_multiset<DelegateEntry> delegates;
};

}
