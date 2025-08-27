#pragma once

/*  This file is part of EmuFramework.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with EmuFramework.  If not, see <http://www.gnu.org/licenses/> */

#include <emuframework/EmuAppHelper.hh>
#include <emuframework/EmuSystemTask.hh>
#include <emuframework/EmuSystemTaskContext.hh>
#include <imagine/gfx/PixmapBufferTexture.hh>

namespace EmuEx
{

using namespace IG;
class EmuVideo;
class EmuSystem;

class [[nodiscard]] EmuVideoImage
{
public:
	constexpr EmuVideoImage() = default;
	EmuVideoImage(EmuSystemTaskContext, EmuVideo&, Gfx::LockedTextureBuffer);
	IG::MutablePixmapView pixmap() const;
	explicit operator bool() const;
	void endFrame();

protected:
	EmuSystemTaskContext taskCtx;
	EmuVideo* emuVideo{};
	Gfx::LockedTextureBuffer texBuff;
};

class EmuVideo : public EmuAppHelper
{
public:
	constexpr EmuVideo() = default;
	void setRendererTask(Gfx::RendererTask&);
	bool hasRendererTask() const;
	bool setFormat(IG::PixmapDesc, EmuSystemTaskContext _ = {});
	void resetImage(IG::PixelFormat newFmt = {});
	IG::PixmapDesc deleteImage();
	EmuVideoImage startFrame(EmuSystemTaskContext);
	void startFrame(EmuSystemTaskContext, IG::PixmapView);
	EmuVideoImage startFrameWithFormat(EmuSystemTaskContext, IG::PixmapDesc);
	void startFrameWithFormat(EmuSystemTaskContext, IG::PixmapView);
	void startFrameWithAltFormat(EmuSystemTaskContext, IG::PixmapView);
	void startUnchangedFrame(EmuSystemTaskContext);
	void finishFrame(EmuSystemTaskContext, Gfx::LockedTextureBuffer);
	void finishFrame(EmuSystemTaskContext, IG::PixmapView);
	void clear();
	void takeGameScreenshot();
    //region 爱吾：增加指定路径的截图
    void takeGameScreenshotAiWu();
    //endregion
	bool isExternalTexture() const;
	Gfx::PixmapBufferTexture& image();
	Gfx::Renderer& renderer() const;
	IG::ApplicationContext appContext() const;
	WSize size() const;
	bool formatIsEqual(IG::PixmapDesc desc) const;
	void setTextureBufferMode(EmuSystem&, Gfx::TextureBufferMode);
	void setSampler(Gfx::TextureSamplerConfig);
	constexpr auto colorSpace() const { return colSpace; }
	bool setRenderPixelFormat(EmuSystem&, IG::PixelFormat, Gfx::ColorSpace);
	IG::PixelFormat renderPixelFormat() const;
	IG::PixelFormat internalRenderPixelFormat() const;
	static Gfx::TextureSamplerConfig samplerConfigForLinearFilter(bool useLinearFilter);
	static MutablePixmapView takeInterlacedFields(MutablePixmapView, bool isOddField);

protected:
	Gfx::RendererTask* rTask{};
	Gfx::PixmapBufferTexture vidImg;
	IG::PixelFormat renderFmt;
	Gfx::TextureBufferMode bufferMode{};
	bool screenshotNextFrame{};
	Gfx::ColorSpace colSpace{Gfx::ColorSpace::LINEAR};
	bool useLinearFilter{true};
	//region 爱吾：增加截图路径
	bool screenshotNextFrameAiWu{};
	//endregion

	void doScreenshot(EmuSystemTaskContext, IG::PixmapView);
	void postFrameFinished(EmuSystemTaskContext);
	Gfx::TextureSamplerConfig samplerConfig() const { return samplerConfigForLinearFilter(useLinearFilter); }

public:
	bool isOddField{};
};

}
