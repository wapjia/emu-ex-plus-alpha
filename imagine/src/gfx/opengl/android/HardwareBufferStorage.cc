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
#include <imagine/gfx/opengl/android/HardwareBufferStorage.hh>
#include <imagine/gfx/Renderer.hh>
#include <imagine/logger/SystemLogger.hh>
#include <android/hardware_buffer.h>

namespace IG::Gfx
{

static SystemLogger log{"HardwareBuffStorage"};
constexpr uint32_t allocateUsage = AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN | AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
constexpr uint32_t lockUsage = AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;

template<class Buffer>
HardwareSingleBufferStorage<Buffer>::HardwareSingleBufferStorage(RendererTask &r, TextureConfig config):
	Texture{r}
{
	config = baseInit(r, config);
	if(!setFormat(config.pixmapDesc, config.colorSpace, config.samplerConfig)) [[unlikely]]
	{
		throw std::runtime_error("Error creating hardware buffer");
	}
}

template<class Buffer>
bool HardwareSingleBufferStorage<Buffer>::setFormat(PixmapDesc desc, ColorSpace colorSpace, TextureSamplerConfig samplerConf)
{
	buffer = {desc, allocateUsage};
	if(!buffer)
	{
		log.info("error allocating {}x{} format:{} buffer", desc.w(), desc.h(), desc.format.name());
		return false;
	}
	log.info("allocated buffer:{} size:{}x{} format:{} pitch:{}",
		(void*)buffer.nativeObject(), desc.w(), desc.h(), desc.format.name(), buffer.pitch());
	pitchBytes = buffer.pitch() * desc.format.bytesPerPixel();
	auto dpy = renderer().glDisplay();
	auto eglImg = makeAndroidNativeBufferEGLImage(dpy, buffer.eglClientBuffer(), colorSpace == ColorSpace::SRGB);
	if(!eglImg) [[unlikely]]
	{
		log.error("error creating EGL image");
		return false;
	}
	initWithEGLImage(eglImg, desc, asSamplerParams(samplerConf), false);
	eglDestroyImageKHR(dpy, eglImg);
	return true;
}

template<class Buffer>
LockedTextureBuffer HardwareSingleBufferStorage<Buffer>::lock(TextureBufferFlags bufferFlags)
{
	void *data{};
	if(!buffer.lock(lockUsage, &data)) [[unlikely]]
	{
		log.error("error locking");
		return {};
	}
	return lockedBuffer(data, pitchBytes, bufferFlags);
}

template<class Buffer>
void HardwareSingleBufferStorage<Buffer>::unlock(LockedTextureBuffer, TextureWriteFlags)
{
	buffer.unlock();
}

template<class Buffer>
HardwareBufferStorage<Buffer>::HardwareBufferStorage(RendererTask &r, TextureConfig config):
	Texture{r}
{
	config = baseInit(r, config);
	if(!setFormat(config.pixmapDesc, config.colorSpace, config.samplerConfig)) [[unlikely]]
	{
		throw std::runtime_error("Error creating hardware buffer");
	}
}

template<class Buffer>
bool HardwareBufferStorage<Buffer>::setFormat(PixmapDesc desc, ColorSpace colorSpace, TextureSamplerConfig samplerConf)
{
	auto dpy = renderer().glDisplay();
	for(auto &[buff, eglImg, pitchBytes] : bufferInfo)
	{
		buff = {desc, allocateUsage};
		if(!buff)
		{
			log.info("error allocating {}x{} format:{} buffer", desc.w(), desc.h(), desc.format.name());
			return false;
		}
		log.info("allocated buffer:{} size:{}x{} format:{} pitch:{}",
			(void*)buff.nativeObject(), desc.w(), desc.h(), desc.format.name(), buff.pitch());
		pitchBytes = buff.pitch() * desc.format.bytesPerPixel();
		eglImg.reset(makeAndroidNativeBufferEGLImage(dpy, buff.eglClientBuffer(), colorSpace == ColorSpace::SRGB));
		if(!eglImg) [[unlikely]]
		{
			log.error("error creating EGL image");
			return false;
		}
	}
	initWithEGLImage({}, desc, asSamplerParams(samplerConf), true);
	return true;
}

template<class Buffer>
LockedTextureBuffer HardwareBufferStorage<Buffer>::lock(TextureBufferFlags bufferFlags)
{
	void *data{};
	auto &[buff, eglImg, pitchBytes] = bufferInfo[bufferIdx];
	if(!buff.lock(lockUsage, &data)) [[unlikely]]
	{
		log.error("error locking");
		return {};
	}
	return lockedBuffer(data, pitchBytes, bufferFlags);
}

template<class Buffer>
void HardwareBufferStorage<Buffer>::unlock(LockedTextureBuffer, TextureWriteFlags)
{
	bufferInfo[bufferIdx].buffer.unlock();
	swapBuffer();
}

template<class Buffer>
void HardwareBufferStorage<Buffer>::swapBuffer()
{
	updateWithEGLImage(bufferInfo[bufferIdx].eglImg.get());
	bufferIdx = (bufferIdx + 1) % 2;
}

template class HardwareSingleBufferStorage<HardwareBuffer>;
template class HardwareSingleBufferStorage<GraphicBuffer>;
template class HardwareBufferStorage<HardwareBuffer>;
template class HardwareBufferStorage<GraphicBuffer>;

}
