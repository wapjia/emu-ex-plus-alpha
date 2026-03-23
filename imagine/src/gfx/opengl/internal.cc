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

module;

#include <imagine/base/GLContext.hh>
#include <imagine/base/Window.hh>
#include <imagine/gfx/defs.hh>
#include <imagine/time/Time.hh>
#include <imagine/util/opengl/glHeaders.h>
#include <imagine/util/Interpolator.hh>
#include <imagine/util/macros.h>

export module imagine.internal.gfxOpengl;

export namespace IG::Gfx
{

struct GLRendererWindowData
{
	constexpr GLRendererWindowData() = default;
	GLDrawable drawable;
	GLBufferConfig bufferConfig{};
	InterpolatorValue<float, SteadyClockTimePoint, InterpolatorType::EASEOUTQUAD> projAngleM{};
	GLColorSpace colorSpace{};
	int8_t swapInterval{1};
	Rect2<int> viewportRect{};
};

GLRendererWindowData& winData(Window& win)
{
	return *win.rendererData<GLRendererWindowData>();
}

const GLRendererWindowData& winData(const Window& win)
{
	return *win.rendererData<GLRendererWindowData>();
}

inline constexpr bool forceNoVAOs = false; // for testing non-VAO code paths
inline constexpr bool defaultToFullErrorChecks = true;
inline constexpr GLuint VATTR_POS = 0, VATTR_TEX_UV = 1, VATTR_COLOR = 2;

inline constexpr GL::API glAPI = Config::Gfx::OPENGL_ES ? GL::API::OpenGLES : GL::API::OpenGL;

inline constexpr GLenum asGLType(AttribType type)
{
	switch(type)
	{
		case AttribType::UByte: return GL_UNSIGNED_BYTE;
		case AttribType::Short: return GL_SHORT;
		case AttribType::UShort: return GL_UNSIGNED_SHORT;
		case AttribType::Float: return GL_FLOAT;
	}
	unreachable();
}

}
