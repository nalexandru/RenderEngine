#include <assert.h>

#if defined(_WIN32)
#	include <Windows.h>
#	define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__APPLE__)
#elif defined(__linux__)
#	include <dlfcn.h>
#	define GLFW_EXPOSE_NATIVE_X11
#endif

#ifndef _countof
#	define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <RenderEngine/RenderEngine.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Resources
#include "lenna.h"

static inline struct ReTexture *_loadTexture(const struct RenderEngine *);

int
main(int argc, char *argv[])
{
	assert("Failed to initialize GLFW" && glfwInit());

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow *wnd{ glfwCreateWindow(1280, 720, "RenderEngine Test Program", nullptr, nullptr) };
	
	void *nativeWindow{ nullptr };
	ReCreateRenderEngineProc createRenderEngine;
#if defined(_WIN32)
	HMODULE renderEngine{ LoadLibrary(L"RenderEngine") };
	assert("Failed to load render engine library" && renderEngine);

	createRenderEngine = (ReCreateRenderEngineProc)GetProcAddress(renderEngine, "Re_CreateRenderEngine");
	nativeWindow = glfwGetWin32Window(wnd);
#else
	void *renderEngine{ dlopen("libRenderEngine.so", RTLD_NOW) };
	assert("Failed to load render engine library" && renderEngine);
	
	createRenderEngine = (ReCreateRenderEngineProc)dlsym(renderEngine, "Re_CreateRenderEngine");
	nativeWindow = (void *)glfwGetX11Window(wnd);
#endif

	assert("The library is not a valid render engine" && createRenderEngine);

	const struct RenderEngine *re{ createRenderEngine() };

	assert("Failed to initialize render engine" && re->Init(nativeWindow));

	struct ReRenderDeviceInfo deviceInfo[2];
	uint32_t count{ _countof(deviceInfo) };
	re->EnumerateDevices(&count, deviceInfo);

	assert("Failed to initialize render device" && re->InitDevice(&deviceInfo[0]));

	ReTexture *tex{ _loadTexture(re) };

	while (!glfwWindowShouldClose(wnd)) {
		re->RenderScene(nullptr, nullptr, nullptr);
	
		glfwPollEvents();
	}

	re->DestroyTexture(tex);

	re->TermDevice();
	re->Term();

#ifdef _WIN32
	FreeLibrary(renderEngine);
#else
	dlclose(renderEngine);
#endif

	glfwTerminate();

	return 0;
}

static inline struct ReTexture *
_loadTexture(const struct RenderEngine *re)
{
	struct ReTextureCreateInfo tci{};
	tci.type = RE_TEXTURE_2D;
	tci.usage = (ReTextureUsage)(RE_TU_TRANSFER_DST | RE_TU_SAMPLED);
	tci.format = RE_TF_R8G8B8A8_UNORM;
	tci.memoryType = RE_MT_GPU_LOCAL;
	tci.depth = 1;
	tci.mipLevels = 1;
	tci.arrayLayers = 1;
	tci.samples = 1;

	int x, y, c;
	uint8_t *data{ stbi_load_from_memory(lenna_png, sizeof(lenna_png), &x, &y, &c, 4) };

	tci.width = x;
	tci.height = y;

	struct ReTexture *tex{ re->CreateTexture(&tci) };
	assert(tex);

	re->UploadTexture(tex, data, x * y * 4);

	return tex;
}

