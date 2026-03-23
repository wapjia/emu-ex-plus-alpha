/*  This file is part of MSX.emu.

	MSX.emu is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	MSX.emu is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with MSX.emu.  If not, see <http://www.gnu.org/licenses/> */

extern "C"
{
	#include <blueMSX/VideoChips/FrameBuffer.h>
}
import system;
import imagine;
import std;

using namespace IG;
using namespace EmuEx;

static constexpr auto pixFmt = PIXEL_WIDTH == 16 ? PixelFmtRGB565 : PixelFmtRGBA8888;

class FrameBufferImpl
{
public:
	constexpr FrameBufferImpl() {}

	int width() const
	{
		return (doubleWidthFrame ? maxWidth * 2 : maxWidth) - widthCrop() * 2;
	}

	int widthCrop() const
	{
		return doubleWidthFrame ? 16 : 8;
	}

	void setMaxWidth(int w)
	{
		maxWidth = w;
	}

	bool doubleWidth() const
	{
		return doubleWidthFrame;
	}

	bool setDoubleWidth(bool val)
	{
		if(doubleWidthFrame == val)
			return false;
		doubleWidthFrame = val;
		updatePixmapSize();
		return true;
	}

	void setLines(int lines_)
	{
		lines = lines_;
		updatePixmapSize();
	}

	PixmapDesc makePixmapDesc() const
	{
		return {{width(), lines}, pixFmt};
	}

	void setPixmapData(void *data)
	{
		pix = {makePixmapDesc(), data};
	}

	PixmapView pixmap() const
	{
		assume(pix.format() == pixFmt);
		return pix;
	}

	void *setCurrentScanline(int line)
	{
		currentLine = line;
		return &pix[0, line];
	}

	int currentScanline() const
	{
		return currentLine;
	}

protected:
	MutablePixmapView pix{{{}, pixFmt}};
	int maxWidth = 1;
	int lines = 1;
	int currentLine = 0;
	bool doubleWidthFrame = false;

	void updatePixmapSize()
	{
		pix = {makePixmapDesc(), pix.data()};
	}
};

static FrameBufferImpl fb{};

PixmapView frameBufferPixmap()
{
	auto fbPix = fb.pixmap();
	return fbPix.subView({0, 8}, {(int)fbPix.w(), (int)fbPix.h() - 16});
}

FrameBufferData* frameBufferDataCreate(int maxWidth, int maxHeight, int defaultHorizZoom)
{
	MsxSystem::log.info("created data with max size:{}x{} zoom:{}", maxWidth, maxHeight, defaultHorizZoom);
	fb.setMaxWidth(maxWidth);
	return (FrameBufferData*)std::malloc(maxWidth * 2 * maxHeight * sizeof(Pixel));
}

void frameBufferDataDestroy(FrameBufferData* frameData)
{
	std::free(frameData);
}

FrameBuffer* frameBufferGetDrawFrame() { return &fb; }

FrameBuffer* frameBufferFlipDrawFrame() { return &fb; }

FrameBufferData* frameBufferGetActive()
{
	return (FrameBufferData*)fb.pixmap().data();
}

void frameBufferSetActive(FrameBufferData* frameData)
{
	fb.setPixmapData(frameData);
}

void frameBufferSetMixMode(FrameBufferMixMode mode, FrameBufferMixMode mask) {}

void frameBufferClearDeinterlace() {}

void frameBufferSetInterlace(FrameBuffer* frameBuffer, int val)
{
	//logMsg("setting interlace %d", val);
}

void frameBufferSetLineCount(FrameBuffer* frameBuffer, int val)
{
	//logMsg("setting line count %d", val);
	((FrameBufferImpl*)frameBuffer)->setLines(val);
}

Pixel* frameBufferGetLine(FrameBuffer* frameBuffer, int y)
{
	//logMsg("getting line %d", y);
	/*if(y < 8 || y >= (224+8)) return dummyLine;
    return (Pixel*)&screenBuff[(y - 8) * msxResX];*/
	return (Pixel*)((FrameBufferImpl*)frameBuffer)->setCurrentScanline(y);
}

int frameBufferGetDoubleWidth(FrameBuffer* frameBuffer, int y)
{
	return ((FrameBufferImpl*)frameBuffer)->doubleWidth();
}

void frameBufferSetDoubleWidth(FrameBuffer* frameBuffer, int y, int val)
{
	if(((FrameBufferImpl*)frameBuffer)->setDoubleWidth(val))
		MsxSystem::log.info("set double width:{} on line:{}", val, y);
}

// Used by gunstick and asciilaser, line is set in frameBufferGetLine()
void frameBufferSetScanline(int scanline) {}

int frameBufferGetScanline(FrameBuffer* frameBuffer)
{
	return ((FrameBufferImpl*)frameBuffer)->currentScanline();
}
