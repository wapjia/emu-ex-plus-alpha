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

#include <imagine/config/defs.hh>
#include <imagine/base/baseDefs.hh>
#include <imagine/util/rectangle2.h>
#include <imagine/util/used.hh>
#include <imagine/util/2DOrigin.h>
#include <imagine/util/math/Point2D.hh>

namespace IG
{

class Viewport
{
public:
	constexpr Viewport() = default;
	constexpr Viewport(WRect originRect, WRect rect, Rotation softOrientation = Rotation::UP):
		originRect{originRect}, rect{rect}, softRotation_{softOrientation} {}
	constexpr Viewport(WRect rect, Rotation softOrientation = Rotation::UP):
		Viewport{rect, rect, softOrientation} {}
	constexpr Viewport(WSize size):
		Viewport{{{}, size}} {}
	[[nodiscard]]
	constexpr WRect realBounds() const { return isSideways() ? bounds().makeInverted() : bounds(); }
	[[nodiscard]]
	constexpr WRect bounds() const { return rect; }
	[[nodiscard]]
	constexpr WRect realOriginBounds() const { return isSideways() ? originBounds().makeInverted() : originBounds(); }
	[[nodiscard]]
	constexpr WRect originBounds() const { return originRect; }
	[[nodiscard]]
	constexpr int realWidth() const { return isSideways() ? height() : width(); }
	[[nodiscard]]
	constexpr int realHeight() const { return isSideways() ? width() : height(); }
	[[nodiscard]]
	constexpr int width() const { return rect.xSize(); }
	[[nodiscard]]
	constexpr int height() const { return rect.ySize(); }
	[[nodiscard]]
	constexpr float aspectRatio() const { return (float)width() / (float)height(); }
	[[nodiscard]]
	constexpr float realAspectRatio() const { return (float)realWidth() / (float)realHeight(); }
	[[nodiscard]]
	constexpr bool isPortrait() const { return width() < height(); }
	[[nodiscard]]
	constexpr bool isSideways() const { return IG::isSideways(softRotation_); }
	[[nodiscard]]
	constexpr bool operator==(Viewport const &) const = default;

	[[nodiscard]]
	constexpr WRect relRect(WPt pos, WSize size, _2DOrigin posOrigin, _2DOrigin screenOrigin) const
	{
		// adjust to the requested origin on the screen
		auto newX = LT2DO.adjustX(pos.x, width(), screenOrigin.invertYIfCartesian());
		auto newY = LT2DO.adjustY(pos.y, height(), screenOrigin.invertYIfCartesian());
		WRect rect;
		rect.setPosRel({newX, newY}, size, posOrigin);
		return rect;
	}

	[[nodiscard]]
	constexpr WRect relRectBestFit(WPt pos, float aspectRatio, _2DOrigin posOrigin, _2DOrigin screenOrigin) const
	{
		auto size = sizesWithRatioBestFit(aspectRatio, width(), height());
		return relRect(pos, size, posOrigin, screenOrigin);
	}

	// converts to a relative rectangle in OpenGL coordinate system
	[[nodiscard]]
	constexpr Rect2<int> asYUpRelRect() const
	{
		return {{realBounds().x, realOriginBounds().ySize() - realBounds().y2}, {realWidth(), realHeight()}};
	}

private:
	WRect originRect{};
	WRect rect{};
	ConditionalMemberOr<!Config::SYSTEM_ROTATES_WINDOWS, Rotation, Rotation::UP> softRotation_{Rotation::UP};
};

}
