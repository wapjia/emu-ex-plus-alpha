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
#include <mdspan>
#include <array>
#endif

namespace IG
{

using std::mdspan;
using std::extents;
using std::layout_stride;

constexpr auto stridedMdspan2(Pointer auto p, auto extents, size_t stride)
{
  return mdspan{p, layout_stride::mapping{extents, std::array{stride, 1uz}}};
}

}
