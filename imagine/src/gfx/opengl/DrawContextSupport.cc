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

#include <imagine/config/macros.h>
#include <imagine/gfx/Renderer.hh>
#include <imagine/gfx/BasicEffect.hh>
#include <imagine/logger/SystemLogger.hh>
#include <imagine/util/opengl/glHeaders.h>
#ifdef CONFIG_BASE_GL_PLATFORM_EGL
	#ifndef EGL_NO_X11
	#define EGL_NO_X11
	#endif
#include <EGL/egl.h>
#endif
import imagine.internal.gfxOpengl;

namespace IG::Gfx
{

static SystemLogger log{"GLRenderer"};

bool DrawContextSupport::hasSyncFences() const
{
	if constexpr((bool)Config::Gfx::OPENGL_ES)
	{
		#ifdef CONFIG_BASE_GL_PLATFORM_EGL
		return (bool)eglCreateSync;
		#else
		return glFenceSync;
		#endif
	}
	else
	{
		return true;
	}
}

bool DrawContextSupport::hasServerWaitSync() const
{
	return false;
	/*
	if constexpr(Config::Gfx::OPENGL_ES)
	{
		#ifdef CONFIG_BASE_GL_PLATFORM_EGL
		return eglWaitSync;
		#else
		return glWaitSync;
		#endif
	}
	else
	{
		return true;
	}
	*/
}

GLsync DrawContextSupport::fenceSync([[maybe_unused]] GLDisplay dpy)
{
	#ifdef CONFIG_BASE_GL_PLATFORM_EGL
	return static_cast<GLsync>(eglCreateSync(dpy, EGL_SYNC_FENCE, nullptr));
	#else
	return glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	#endif
}

void DrawContextSupport::deleteSync([[maybe_unused]] GLDisplay dpy, GLsync sync)
{
	#ifdef CONFIG_BASE_GL_PLATFORM_EGL
	if(bool success = eglDestroySync(dpy, sync);
		Config::DEBUG_BUILD && !success)
	{
		log.error("error:{} in eglDestroySync({}, {})", GLManager::errorString(eglGetError()), (EGLDisplay)dpy, (void*)sync);
	}
	#else
	glDeleteSync(sync);
	#endif
}

GLenum DrawContextSupport::clientWaitSync([[maybe_unused]] GLDisplay dpy, GLsync sync, GLbitfield flags, GLuint64 timeout)
{
	#ifdef CONFIG_BASE_GL_PLATFORM_EGL
	if(auto status = eglClientWaitSync(dpy, sync, flags, timeout);
		Config::DEBUG_BUILD && !status)
	{
		log.error("error:{} in eglClientWaitSync({}, {}, {:X}, {})",
			GLManager::errorString(eglGetError()), (EGLDisplay)dpy, (void*)sync, flags, timeout);
		return status;
	}
	else
	{
		return status;
	}
	#else
	return glClientWaitSync(sync, flags, timeout);
	#endif
}

void DrawContextSupport::waitSync(GLDisplay, GLsync)
{
	log.warn("waitSync() not currently used");
	/*#ifdef CONFIG_BASE_GL_PLATFORM_EGL
	if constexpr(Config::Gfx::OPENGL_ES)
	{
		if(!eglWaitSync)
		{
			eglClientWaitSync(dpy, sync, 0, SyncFence::IGNORE_TIMEOUT);
			return;
		}
	}
	eglWaitSync(dpy, sync, 0);
	#else
	glWaitSync(sync, 0, SyncFence::IGNORE_TIMEOUT);
	#endif*/
}

bool DrawContextSupport::hasDrawReadBuffers() const
{
	#ifdef CONFIG_GFX_OPENGL_ES
	return false; //glDrawBuffers;
	#else
	return true;
	#endif
}

#ifdef __ANDROID__
bool DrawContextSupport::hasEGLTextureStorage() const
{
	return glEGLImageTargetTexStorageEXT;
}
#endif

bool DrawContextSupport::hasImmutableBufferStorage() const
{
	#ifdef CONFIG_GFX_OPENGL_ES
	return glBufferStorage;
	#else
	return hasBufferStorage;
	#endif
}

bool DrawContextSupport::hasMemoryBarriers() const
{
	return false;
	/*#ifdef CONFIG_GFX_OPENGL_ES
	return glMemoryBarrier;
	#else
	return hasMemoryBarrier;
	#endif*/
}

bool DrawContextSupport::hasVAOFuncs() const
{
	if constexpr(Config::DEBUG_BUILD && forceNoVAOs)
		return false;
	#ifdef CONFIG_GFX_OPENGL_ES
	return glBindVertexArray;
	#else
	return true;
	#endif
}

static const char *debugTypeToStr(GLenum type)
{
	switch(type)
	{
		case GL_DEBUG_TYPE_ERROR: return "Error";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behavior";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "Undefined Behavior";
		case GL_DEBUG_TYPE_PORTABILITY: return "Portability";
		case GL_DEBUG_TYPE_PERFORMANCE: return "Performance";
		default: [[fallthrough]];
		case GL_DEBUG_TYPE_OTHER: return "Other";
	}
}

constexpr IG::Log::Level severityToLogger(GLenum severity)
{
	switch(severity)
	{
		default: [[fallthrough]];
		case GL_DEBUG_SEVERITY_LOW: return IG::Log::Level::I;
		case GL_DEBUG_SEVERITY_MEDIUM: return IG::Log::Level::W;
		case GL_DEBUG_SEVERITY_HIGH: return IG::Log::Level::E;
	}
}

void DrawContextSupport::setGLDebugOutput(bool on)
{
	if(!hasDebugOutput)
		return;
	if(!on)
	{
		glDisable(GL_DEBUG_OUTPUT);
	}
	else
	{
		if(!glDebugMessageCallback) [[unlikely]]
		{
			log.warn("enabling debug output with {}", GL_DEBUG_MESSAGE_CALLBACK_NAME);
			glDebugMessageCallback = (typeof(glDebugMessageCallback))GLManager::procAddress(GL_DEBUG_MESSAGE_CALLBACK_NAME);
		}
		glDebugMessageCallback(
			GL_APIENTRY []([[maybe_unused]] GLenum source, GLenum type, [[maybe_unused]] GLuint id,
				GLenum severity, [[maybe_unused]] GLsizei length, const GLchar* message, [[maybe_unused]] const void* userParam)
			{
				std::string_view msgString{message, size_t(length)};
				if(Config::envIsAndroid && (msgString == "FreeAllocationOnTimestamp - WaitForTimestamp"
					|| msgString.contains("Submission has been flushed")))
				{
					return;
				}
				if(type == GL_DEBUG_TYPE_OTHER)
				{
					return;
				}
				log.print(severityToLogger(severity), "{}: {}", debugTypeToStr(type), message);
				if(severity == GL_DEBUG_SEVERITY_HIGH && type != GL_DEBUG_TYPE_PERFORMANCE)
					std::abort();
			}, nullptr);
		glEnable(GL_DEBUG_OUTPUT);
	}
}

}
