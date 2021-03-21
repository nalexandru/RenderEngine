#include <Windows.h>

#include <d3d9.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx9.h>

#include "GraphEditor.h"

using namespace ImGui;

#define WND_CLASS L"RenderEngineGraphEditorWindow"

static IDirect3D9 *_d3d;
static IDirect3DDevice9 *_device;
static D3DPRESENT_PARAMETERS _pp;

#pragma comment(lib, "d3d9.lib")

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK
_WndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{ 
	if (ImGui_ImplWin32_WndProcHandler(wnd, msg, wParam, lParam))
		return true;

	switch (msg) {
	case WM_CREATE:
		_d3d = Direct3DCreate9(D3D_SDK_VERSION);
		assert(_d3d);

		_pp.Windowed = TRUE;
		_pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		_pp.BackBufferFormat = D3DFMT_UNKNOWN;
		_pp.EnableAutoDepthStencil = FALSE;
		_pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

		_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &_pp, &_device);
		assert(_device);

		CreateContext();

		StyleColorsDark();
		ImGui_ImplWin32_Init(wnd);
		ImGui_ImplDX9_Init(_device);

		_device->SetRenderState(D3DRS_ZENABLE, FALSE);
		_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	break;
	case WM_SIZE:
		if (!_device)
			break;

		_pp.BackBufferWidth = LOWORD(lParam);
		_pp.BackBufferHeight = HIWORD(lParam);

		ImGui_ImplDX9_InvalidateDeviceObjects();
		_device->Reset(&_pp);
		ImGui_ImplDX9_CreateDeviceObjects();
	break;
	case WM_DESTROY:

		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		DestroyContext();
		
		_device->Release();
		_d3d->Release();

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

	HWND window = CreateWindowEx(WS_EX_APPWINDOW, WND_CLASS, L"RenderEngine GraphEditor", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 1280, 720, HWND_DESKTOP, NULL, instance, NULL);
	if (!window) {
		MessageBox(HWND_DESKTOP, L"Failed to create window", L"Fatal", MB_ICONERROR | MB_OK);
		return -1;
	}

	ShowWindow(window, showCmd);

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

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		NewFrame();

		GE_Draw();
	
		Render();

		_device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 255), 1.f, 0);
		_device->BeginScene();

		ImGui_ImplDX9_RenderDrawData(GetDrawData());

		_device->EndScene();
		_device->Present(nullptr, nullptr, nullptr, nullptr);
	}

	return msg.wParam;
}
