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
#include <imagine/base/GLContext.hh>
#include <imagine/util/memory/UniqueResource.hh>
#include <imagine/util/used.hh>
#include <imagine/util/rectangle2.h>
#include <imagine/util/math.hh>
#include <imagine/util/opengl/glHeaders.h>
#ifndef IG_USE_MODULE_STD
#include <variant>
#include <span>
#endif

namespace Config::Gfx
{
	#ifdef CONFIG_GFX_OPENGL_ES
	inline constexpr int OPENGL_ES = CONFIG_GFX_OPENGL_ES;
	#else
	inline constexpr int OPENGL_ES = 0;
	#endif

	#ifdef CONFIG_GFX_ANDROID_SURFACE_TEXTURE
	inline constexpr bool OPENGL_TEXTURE_TARGET_EXTERNAL = true;
	#else
	inline constexpr bool OPENGL_TEXTURE_TARGET_EXTERNAL = false;
	#endif

	#if defined CONFIG_OS_IOS
	inline constexpr bool GLDRAWABLE_NEEDS_FRAMEBUFFER = true;
	#else
	inline constexpr bool GLDRAWABLE_NEEDS_FRAMEBUFFER = false;
	#endif
}

namespace IG::Gfx
{

class RendererTask;

using TextureRef = GLuint;

using VertexIndexSpan = std::variant<std::span<const uint8_t>, std::span<const uint16_t>>;

inline constexpr int TRIANGLE_IMPL = GL_TRIANGLES;
inline constexpr int TRIANGLE_STRIP_IMPL = GL_TRIANGLE_STRIP;

inline constexpr int ZERO_IMPL = GL_ZERO;
inline constexpr int ONE_IMPL = GL_ONE;
inline constexpr int SRC_COLOR_IMPL = GL_SRC_COLOR;
inline constexpr int ONE_MINUS_SRC_COLOR_IMPL = GL_ONE_MINUS_SRC_COLOR;
inline constexpr int DST_COLOR_IMPL = GL_DST_COLOR;
inline constexpr int ONE_MINUS_DST_COLOR_IMPL = GL_ONE_MINUS_DST_COLOR;
inline constexpr int SRC_ALPHA_IMPL = GL_SRC_ALPHA;
inline constexpr int ONE_MINUS_SRC_ALPHA_IMPL = GL_ONE_MINUS_SRC_ALPHA;
inline constexpr int DST_ALPHA_IMPL = GL_DST_ALPHA;
inline constexpr int ONE_MINUS_DST_ALPHA_IMPL = GL_ONE_MINUS_DST_ALPHA;
inline constexpr int CONSTANT_COLOR_IMPL = GL_CONSTANT_COLOR;
inline constexpr int ONE_MINUS_CONSTANT_COLOR_IMPL = GL_ONE_MINUS_CONSTANT_COLOR;
inline constexpr int CONSTANT_ALPHA_IMPL = GL_CONSTANT_ALPHA;
inline constexpr int ONE_MINUS_CONSTANT_ALPHA_IMPL = GL_ONE_MINUS_CONSTANT_ALPHA;

inline constexpr int SYNC_FLUSH_COMMANDS_BIT = GL_SYNC_FLUSH_COMMANDS_BIT;

using ClipRect = WRect;
using Drawable = NativeGLDrawable;

enum class ShaderType : uint16_t
{
	VERTEX = GL_VERTEX_SHADER,
	FRAGMENT = GL_FRAGMENT_SHADER
};

enum class ColorSpace : uint8_t
{
	LINEAR = (uint8_t)GLColorSpace::LINEAR,
	SRGB = (uint8_t)GLColorSpace::SRGB,
};

using NativeBuffer = GLuint;

void destroyGLBuffer(RendererTask &, NativeBuffer);

struct GLBufferDeleter
{
	RendererTask *rTask{};

	void operator()(NativeBuffer s) const
	{
		destroyGLBuffer(*rTask, s);
	}
};
using UniqueGLBuffer = UniqueResource<NativeBuffer, GLBufferDeleter>;

struct TextureBinding
{
	TextureRef name{};
	GLenum target{};
};

struct TextureSizeSupport
{
	int maxXSize{}, maxYSize{};
	ConditionalMemberOr<(bool)Config::Gfx::OPENGL_ES, bool, true> nonPow2CanMipmap{};
	ConditionalMemberOr<(bool)Config::Gfx::OPENGL_ES, bool, true> nonPow2CanRepeat{};

	constexpr bool supportsMipmaps(int x, int y) const
	{
		return x && y && (nonPow2CanMipmap || (isPowerOf2(x) && isPowerOf2(y)));
	}
};

}
