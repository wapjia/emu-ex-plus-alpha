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
#include <imagine/base/ApplicationContext.hh>
#include <imagine/pixmap/Pixmap.hh>
#include <imagine/logger/SystemLogger.hh>
#include <imagine/util/ranges.hh>
#include <android/bitmap.h>
#include <jni.h>
#include <sys/types.h>
#include <sys/resource.h>

export module imagine.internal.android;
import std;

namespace IG
{
static constexpr SystemLogger log("Android");
}

export namespace IG
{

class NoopThread
{
public:
	constexpr NoopThread() = default;

	void start()
	{
		if(isRunning.load(std::memory_order_relaxed))
			return;
		isRunning.store(true, std::memory_order_relaxed);
		makeDetachedThread(
			[this]()
			{
				// keep cpu governor busy by running a low priority thread executing no-op instructions
				setpriority(PRIO_PROCESS, 0, 19);
				log.info("started no-op thread");
				while(isRunning.load(std::memory_order_relaxed))
				{
					for([[maybe_unused]] auto i: iotaCount(16))
					{
						asm("nop");
					}
				}
				log.info("ended no-op thread");
			});
	}

	void stop()
	{
		if(!isRunning.load(std::memory_order_relaxed))
			return;
		isRunning.store(false, std::memory_order_relaxed);
	}

	explicit operator bool() { return isRunning.load(std::memory_order_relaxed); }

private:
	std::atomic_bool isRunning{};
};

pid_t mainThreadId{};

jobject makeSurfaceTexture(ApplicationContext, JNIEnv*, jint texName);
jobject makeSurfaceTexture(ApplicationContext, JNIEnv*, jint texName, jboolean singleBufferMode);
bool releaseSurfaceTextureImage(JNIEnv*, jobject surfaceTexture);
void updateSurfaceTextureImage(JNIEnv*, jobject surfaceTexture);
void releaseSurfaceTexture(JNIEnv*, jobject surfaceTexture);

jobject makeSurface(JNIEnv *env, jobject surfaceTexture);
void releaseSurface(JNIEnv *env, jobject surface);

uint32_t toAHardwareBufferFormat(PixelFormatId format)
{
	switch(format)
	{
		case PixelFormatId::RGBA8888: return AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
		case PixelFormatId::RGB565: return AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM;
		default: return 0;
	}
}

const char *aHardwareBufferFormatStr(uint32_t format)
{
	switch(format)
	{
		case 0: return "Unset";
		case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM: return "RGBA8888";
		case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM: return "RGBX8888";
		case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM: return "RGB888";
		case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM: return "RGB565";
	}
	return "Unknown";
}

PixelFormat makePixelFormatFromAndroidFormat(int32_t androidFormat)
{
	switch(androidFormat)
	{
		case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
		case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM: return PixelFmtRGBA8888;
		case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM: return PixelFmtRGB565;
		case ANDROID_BITMAP_FORMAT_RGBA_4444: return PixelFmtRGBA4444;
		case ANDROID_BITMAP_FORMAT_A_8: return PixelFmtI8;
		default:
		{
			log.error("unhandled format");
			return PixelFmtI8;
		}
	}
}

MutablePixmapView makePixmapView(JNIEnv *env, jobject bitmap, void *pixels, PixelFormat format)
{
	AndroidBitmapInfo info;
	auto res = AndroidBitmap_getInfo(env, bitmap, &info);
	if(res != ANDROID_BITMAP_RESULT_SUCCESS) [[unlikely]]
	{
		log.info("error getting bitmap info");
		return {};
	}
	//log.info("android bitmap info:size {}x{}, stride:{}", info.width, info.height, info.stride);
	if(format == PixelFmtUnset)
	{
		// use format from bitmap info
		format = makePixelFormatFromAndroidFormat(info.format);
	}
	return {{{(int)info.width, (int)info.height}, format}, pixels, {(int)info.stride, MutablePixmapView::Units::BYTE}};
}

}
