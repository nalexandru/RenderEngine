#include <Windows.h>

#include <RenderEngine/RenderEngine.h>

#define WND_CLASS L"RenderEngineTestProgramWindow"

static LRESULT CALLBACK
_WndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	break;
	default:
		return DefWindowProcW(wnd, msg, wParam, lParam);
	}
}

int APIENTRY
wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prevInstance, _In_ LPWSTR cmdLine, _In_ int showCmd)
{
	WNDCLASS wincl{};
	wincl.style = CS_HREDRAW | CS_VREDRAW;
	wincl.lpfnWndProc = _WndProc;
	wincl.hInstance = instance;
	wincl.hIcon = LoadIcon(instance, IDI_APPLICATION);
	wincl.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wincl.lpszClassName = WND_CLASS;

	if (!RegisterClass(&wincl)) {
		MessageBox(HWND_DESKTOP, L"Failed to register class", L"Fatal", MB_ICONERROR | MB_OK);
		return -1;
	}

	HWND window = CreateWindowEx(WS_EX_APPWINDOW, WND_CLASS, L"RenderEngine Test Program", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 1280, 720, HWND_DESKTOP, NULL, instance, NULL);
	if (!window) {
		MessageBox(HWND_DESKTOP, L"Failed to create window", L"Fatal", MB_ICONERROR | MB_OK);
		return -1;
	}

	ShowWindow(window, showCmd);

	HMODULE renderEngine = LoadLibrary(L"RenderEngine");
	if (!renderEngine) {
		MessageBox(window, L"Failed to load render engine library", L"Fatal", MB_ICONERROR | MB_OK);
		return -1;
	}

	ReCreateRenderEngineProc createRenderEngine;
	createRenderEngine = (ReCreateRenderEngineProc)GetProcAddress(renderEngine, "Re_CreateRenderEngine");
	if (!createRenderEngine) {
		MessageBox(window, L"The library is not a valid render engine", L"Fatal", MB_ICONERROR | MB_OK);
		return -1;
	}

	const struct RenderEngine *re = createRenderEngine();

	if (!re->Init(window)) {
		MessageBox(window, L"Failed to initialize render engine", L"Fatal", MB_ICONERROR | MB_OK);
		return -1;
	}

	struct ReRenderDeviceInfo deviceInfo[2];
	uint32_t count = _countof(deviceInfo);
	re->EnumerateDevices(&count, deviceInfo);

	if (!re->InitDevice(&deviceInfo[0])) {
		MessageBox(window, L"Failed to initialize render device", L"Fatal", MB_ICONERROR | MB_OK);
		return -1;

	}

	MSG msg;
	while (1) {
		bool exit = false;
		
		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				exit = true;
			} else {
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}

		if (exit)
			break;

		re->RenderScene(nullptr, nullptr, nullptr);
	}

	re->TermDevice();
	re->Term();

	FreeLibrary(renderEngine);
	UnregisterClass(WND_CLASS, instance);

	return msg.wParam;
}