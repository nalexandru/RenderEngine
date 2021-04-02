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

	while (!glfwWindowShouldClose(wnd)) {
		re->RenderScene(nullptr, nullptr, nullptr);
	
		glfwPollEvents();
	}

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
