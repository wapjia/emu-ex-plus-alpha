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
#include <imagine/gfx/opengl/GLStateCache.hh>
#include <imagine/gfx/Renderer.hh>
#include <imagine/util/opengl/glUtils.hh>
#include <imagine/logger/SystemLogger.hh>
#include <imagine/util/opengl/glHeaders.h>

namespace IG::Gfx
{

static SystemLogger log{"GLStateCache"};
bool GLStateCache::verifyState = false;

void GLStateCache::blendFunc(GLenum sfactor, GLenum dfactor)
{
	if(!(sfactor == blendFuncSfactor && dfactor == blendFuncDfactor))
	{
		GL::runChecked([&]()
		{
			glBlendFunc(sfactor, dfactor);
		}, log, Renderer::checkGLErrorsVerbose, "glBlendFunc()");
		blendFuncSfactor = sfactor;
		blendFuncDfactor = dfactor;
	}
}

void GLStateCache::blendEquation(GLenum mode)
{
	if(mode != blendEquationState)
	{
		GL::runChecked([&]()
		{
			glBlendEquation(mode);
		}, log, Renderer::checkGLErrorsVerbose, "glBlendEquation()");
		blendEquationState = mode;
	}
}

int8_t *GLStateCache::getCap(GLenum cap)
{
	#define GLCAP_CASE(cap) case cap: return &stateCap.cap ## _state
	switch(cap)
	{
		GLCAP_CASE(GL_BLEND);
		GLCAP_CASE(GL_SCISSOR_TEST);
	}
	#undef GLCAP_CASE
	return nullptr;
}

void GLStateCache::enable(GLenum cap)
{
	auto state = getCap(cap);
	if(!state) [[unlikely]]
	{
		// unmanaged cap
		log.debug("glEnable unmanaged:{}", cap);
		GL::runChecked([&]()
		{
			glEnable(cap);
		}, log, Renderer::checkGLErrorsVerbose, "glEnable()");
		return;
	}

	if(!(*state) || *state == -1)
	{
		// not enabled or unset
		//logMsg("glEnable %d", (int)cap);
		GL::runChecked([&]()
		{
			glEnable(cap);
		}, log, Renderer::checkGLErrorsVerbose, "glEnable()");
		*state = 1;
	}

	if(verifyState)
	{
		GLboolean enabled = true;
		if(GL::runChecked([&]() { enabled = glIsEnabled(cap); }, log, Renderer::checkGLErrorsVerbose)
				&& !enabled)
		{
			log.error("state {} out of sync", cap);
			unreachable();
		}
	}
}

void GLStateCache::disable(GLenum cap)
{
	auto state = getCap(cap);
	if(!state) [[unlikely]]
	{
		// unmanaged cap
		log.debug("glDisable unmanaged:{}", cap);
		GL::runChecked([&]()
		{
			glDisable(cap);
		}, log, Renderer::checkGLErrorsVerbose, "glDisable()");
		return;
	}

	if((*state))
	{
		// is enabled or unset
		//logMsg("glDisable %d", (int)cap);
		GL::runChecked([&]()
		{
			glDisable(cap);
		}, log, Renderer::checkGLErrorsVerbose, "glDisable()");
		*state = 0;
	}

	if(verifyState)
	{
		GLboolean enabled = false;
		if(GL::runChecked([&]() { enabled = glIsEnabled(cap); }, log, Renderer::checkGLErrorsVerbose)
				&& enabled)
		{
			log.error("state {} out of sync", cap);
			unreachable();
		}
	}
}

GLboolean GLStateCache::isEnabled(GLenum cap)
{
	auto state = getCap(cap);
	if(!state) [[unlikely]]
	{
		// unmanaged cap
		log.debug("glIsEnabled unmanaged:{}", cap);
		return glIsEnabled(cap);
	}

	return *state && *state != -1;
}

}
