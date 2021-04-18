#include "includes.h"

using namespace Win32GameEngine;

LRESULT init(HWND hWnd) {
	SetWindowLong(hWnd, GWL_STYLE, NULL);
	// ShowWindow(hWnd, SW_MAXIMIZE);
	Scene::switchTo("level", hWnd, nullptr);
	return 0;
}

LRESULT CALLBACK eventProcessor(HWND hWnd, UINT type, WPARAM wParam, LPARAM lParam) {
	switch(type) {
	case WM_CREATE:
		return init(hWnd);
	case WM_DESTROY:
		return defaultDestroyHandler(hWnd, wParam, lParam);
	default:
		return Scene::passEvent(hWnd, type, wParam, lParam);
	}
	return DefWindowProc(hWnd, type, wParam, lParam);
}

int APIENTRY wWinMain(
	HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine, int nCmdShow
) {
	Window window = Window(hInstance, Window::InitArg{
		.class_name = L"Window",
		.title = L"NiVz",
		.width = vwidth, .height = vheight,
		.style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
		.event_processor = eventProcessor,
	});
	return window.run();
}