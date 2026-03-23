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

#include <imagine/util/opengl/glHeaders.h>
#include <imagine/util/ctype.hh>
#include <imagine/util/ranges.hh>
#include <imagine/util/utility.hh>
#ifndef IG_USE_MODULE_STD
#include <cstdio>
#include <string_view>
#endif

namespace IG::GL
{

inline int toVersion(const char* versionStr)
{
	// skip to version number
	while(!isDigit(*versionStr) && *versionStr != '\0')
		versionStr++;
	int major = 1, minor = 0;
	if(sscanf(versionStr, "%d.%d", &major, &minor) != 2)
	{
		return 10 * major;
	}
	return 10 * major + minor;
}

inline void forEachExtension(auto&& func)
{
	#ifdef IG_GL_API_OGL
	GLint numExtensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	for(auto i : iotaCount(numExtensions))
	{
		auto extStr = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
		func(std::string_view{extStr});
	}
	#else
	auto extStr = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
	for(auto s: std::string_view{extStr} | std::views::split(' '))
	{
		func(std::string_view{s});
	}
	#endif
}

constexpr auto errorToString(GLenum err)
{
	switch(err)
	{
		case GL_NO_ERROR: return "No Error";
		case GL_INVALID_ENUM: return "Invalid Enum";
		case GL_INVALID_VALUE: return "Invalid Value";
		case GL_INVALID_OPERATION: return "Invalid Operation";
		case GL_OUT_OF_MEMORY: return "Out of Memory";
		default: return "Unknown Error";
	}
}

constexpr auto dataTypeToString(int format)
{
	switch(format)
	{
		case GL_UNSIGNED_BYTE: return "B";
		#if !defined CONFIG_GFX_OPENGL_ES
		case GL_UNSIGNED_INT_8_8_8_8: return "I8888";
		case GL_UNSIGNED_INT_8_8_8_8_REV: return "I8888R";
		case GL_UNSIGNED_SHORT_1_5_5_5_REV: return "S1555";
		case GL_UNSIGNED_SHORT_4_4_4_4_REV: return "S4444R";
		#endif
		case GL_UNSIGNED_SHORT_5_6_5: return "S565";
		case GL_UNSIGNED_SHORT_5_5_5_1: return "S5551";
		case GL_UNSIGNED_SHORT_4_4_4_4: return "S4444";
		default: unreachable();
	}
}

constexpr auto imageFormatToString(int format)
{
	switch(format)
	{
		case GL_RGBA8: return "RGBA8";
		case GL_SRGB8_ALPHA8: return "SRGB8A8";
		case GL_RGB8: return "RGB8";
		case GL_RGB5_A1: return "RGB5_A1";
		#if defined CONFIG_GFX_OPENGL_ES
		case GL_RGB565: return "RGB565";
		#else
		case GL_RGB5: return "RGB5";
		#endif
		case GL_RGBA4: return "RGBA4";
		case GL_BGR: return "BGR";
		case GL_RED: return "RED";
		case GL_R8: return "R8";
		case GL_RG: return "RG";
		case GL_RG8: return "RG8";
		case GL_LUMINANCE8: return "LUMINANCE8";
		case GL_LUMINANCE8_ALPHA8: return "LUMINANCE8_ALPHA8";
		case GL_LUMINANCE_ALPHA: return "LUMINANCE_ALPHA";
		case GL_ALPHA8: return "ALPHA8";
		case GL_RGBA: return "RGBA";
		case GL_BGRA: return "BGRA";
		case GL_RGB: return "RGB";
		case GL_ALPHA: return "ALPHA";
		default: unreachable();
	}
}

inline bool clearErrors(std::invocable<GLenum, const char*> auto&& callback)
{
	bool gotError = false;
	GLenum error;
	while((error = glGetError()) != GL_NO_ERROR)
	{
		gotError = true;
		callback(error, GL::errorToString(error));
	}
	return gotError;
}

inline bool runCheckedAlways(auto&& func, auto& log, const char *label = nullptr)
{
	clearErrors([&](GLenum, const char *errorStr)
	{
		log.warn("clearing error:{}", errorStr);
	});
	func();
	return !clearErrors(
		[&](GLenum, const char *err)
		{
			if(label)
			{
				log.error("{} in {}", err, label);
			}
			else
			{
				log.error("{}", err);
			}
		});
}

inline bool runChecked(auto&& func, auto& log, bool shouldCheck, const char *label = nullptr)
{
	if(!shouldCheck)
	{
		func();
		return true;
	}
	return GL::runCheckedAlways(func, log, label);
}

inline GLuint makeTextureName(GLuint oldTex)
{
	if(oldTex)
	{
		glDeleteTextures(1, &oldTex);
	}
	GLuint tex;
	glGenTextures(1, &tex);
	return tex;
}


}
