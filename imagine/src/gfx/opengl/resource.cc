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

#include <imagine/gfx/Renderer.hh>

namespace Gfx
{

Texture Renderer::makeTexture(TextureConfig config)
{
	return {task(), config};
}

Texture Renderer::makeTexture(GfxImageSource &img, const TextureSampler *compatSampler, bool makeMipmaps)
{
	return {task(), img, compatSampler, makeMipmaps};
}

PixmapTexture Renderer::makePixmapTexture(TextureConfig config)
{
	return {task(), config};
}

PixmapTexture Renderer::makePixmapTexture(GfxImageSource &img, const TextureSampler *compatSampler, bool makeMipmaps)
{
	return {task(), img, compatSampler, makeMipmaps};
}

PixmapBufferTexture Renderer::makePixmapBufferTexture(TextureConfig config, TextureBufferMode mode, bool singleBuffer)
{
	return {task(), config, mode, singleBuffer};
}

TextureSampler Renderer::makeTextureSampler(TextureSamplerConfig config)
{
	return {task(), config};
}

TextureSampler &GLRenderer::commonTextureSampler(CommonTextureSampler sampler)
{
	switch(sampler)
	{
		default: bug_unreachable("sampler:%d", (int)sampler); [[fallthrough]];
		case CommonTextureSampler::CLAMP: return commonSampler.clamp;
		case CommonTextureSampler::NEAREST_MIP_CLAMP: return commonSampler.nearestMipClamp;
		case CommonTextureSampler::NO_MIP_CLAMP: return commonSampler.noMipClamp;
		case CommonTextureSampler::NO_LINEAR_NO_MIP_CLAMP: return commonSampler.noLinearNoMipClamp;
		case CommonTextureSampler::REPEAT: return commonSampler.repeat;
		case CommonTextureSampler::NEAREST_MIP_REPEAT: return commonSampler.nearestMipRepeat;
	}
}

static TextureSamplerConfig commonTextureSamplerConfig(CommonTextureSampler sampler)
{
	TextureSamplerConfig conf;
	switch(sampler)
	{
		default: bug_unreachable("sampler:%d", (int)sampler); [[fallthrough]];
		case CommonTextureSampler::CLAMP:
			conf.setDebugLabel("CommonClamp");
			return conf;
		case CommonTextureSampler::NEAREST_MIP_CLAMP:
			conf.setDebugLabel("CommonNearestMipClamp");
			conf.setMipFilter(MIP_FILTER_NEAREST);
			return conf;
		case CommonTextureSampler::NO_MIP_CLAMP:
			conf.setDebugLabel("CommonNoMipClamp");
			conf.setMipFilter(MIP_FILTER_NONE);
			return conf;
		case CommonTextureSampler::NO_LINEAR_NO_MIP_CLAMP:
			conf.setDebugLabel("CommonNoLinearNoMipClamp");
			conf.setLinearFilter(false);
			conf.setMipFilter(MIP_FILTER_NONE);
			return conf;
		case CommonTextureSampler::REPEAT:
			conf.setDebugLabel("CommonRepeat");
			conf.setWrapMode(WRAP_REPEAT);
			return conf;
		case CommonTextureSampler::NEAREST_MIP_REPEAT:
			conf.setDebugLabel("CommonNearestMipRepeat");
			conf.setMipFilter(MIP_FILTER_NEAREST);
			conf.setWrapMode(WRAP_REPEAT);
			return conf;
	}
}

TextureSampler &Renderer::makeCommonTextureSampler(CommonTextureSampler sampler)
{
	auto &commonSampler = commonTextureSampler(sampler);
	if(!commonSampler)
		commonSampler = makeTextureSampler(commonTextureSamplerConfig(sampler));
	return commonSampler;
}

}
