addPkgOpenGL(imagine)

target_sources(
	imagine PRIVATE
	../common/GfxText.cc
	../common/GlyphTextureSet.cc
	../common/Mat4.cc
	BasicEffect.cc
	Buffer.cc
	DrawContextSupport.cc
	GLStateCache.cc
	GLTask.cc
	PixmapBufferTexture.cc
	Program.cc
	Renderer.cc
	RendererCommands.cc
	RendererTask.cc
	Texture.cc
	TextureSampler.cc
)

target_sources(imagine PRIVATE FILE_SET CXX_MODULES FILES internal.cc)

if(ENV STREQUAL ios)
	target_sources(imagine PRIVATE ios/drawable.mm)
endif()

if(ENV STREQUAL android)
	target_sources(
		imagine PRIVATE
		android/HardwareBufferStorage.cc
		android/SurfaceTextureStorage.cc
	)
endif()
