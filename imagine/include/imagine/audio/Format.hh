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

#include <imagine/time/Time.hh>
#include <imagine/util/concepts.hh>
#ifndef IG_USE_MODULE_STD
#include <cmath>
#include <cstdint>
#endif

namespace IG::Audio
{

class SampleFormat
{
public:
	constexpr SampleFormat() = default;
	constexpr SampleFormat(uint8_t bytes, bool isFloat = false):
		bytes_{bytes}, isFloat_{isFloat}{}

	constexpr int bytes() const
	{
		return bytes_;
	}

	constexpr int bits() const
	{
		return bytes() * 8;
	}

	constexpr bool isFloat() const
	{
		return isFloat_;
	}

	constexpr bool operator ==(SampleFormat const& rhs) const = default;

	constexpr explicit operator bool() const
	{
		return bytes();
	}

protected:
	uint8_t bytes_:7{};
	uint8_t isFloat_:1{};
};

namespace SampleFormats
{
	inline constexpr SampleFormat   i8{1};
	inline constexpr SampleFormat  i16{2};
	inline constexpr SampleFormat  i32{4};
	inline constexpr SampleFormat  f32{4, true};
	inline constexpr SampleFormat none;
}

struct Format
{
	int rate{};
	SampleFormat sample{};
	int8_t channels{};

	constexpr bool operator ==(Format const& rhs) const = default;

	constexpr explicit operator bool() const
	{
		return rate != 0 && sample && channels != 0;
	}

	constexpr auto bytesPerFrame() const
	{
		return sample.bytes() * channels;
	}

	constexpr auto framesToBytes(Arithmetic auto frames) const
	{
		return frames * bytesPerFrame();
	}

	constexpr auto bytesToFrames(Arithmetic auto bytes) const
	{
		return bytes / bytesPerFrame();
	}

	template<class T = FloatSeconds>
	constexpr T framesToTime(Arithmetic auto frames) const
	{
		return T{FloatSeconds{(double)frames / rate}};
	}

	template<class T = FloatSeconds>
	constexpr T bytesToTime(Arithmetic auto bytes) const
	{
		return framesToTime(bytesToFrames(bytes));
	}

	constexpr auto timeToFrames(ChronoDuration auto time) const
	{
		return std::ceil(std::chrono::duration_cast<FloatSeconds>(time).count() * rate);
	}

	constexpr auto timeToBytes(ChronoDuration auto time) const
	{
		return framesToBytes(timeToFrames(time));
	}

	void *copyFrames(void *dest, const void *src, size_t frames, Format srcFormat, float volume = 1.f) const;
};

}
