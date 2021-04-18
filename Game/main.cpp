#include "includes.h"
#include <io.h>
#include <string>

using namespace Win32GameEngine;

vector<wstring> track_list;
void getTrackList() {
	HANDLE hFile;
	WIN32_FIND_DATA fileinfo;
	hFile = FindFirstFile((wstring(tracks_dir) + L"*").c_str(), &fileinfo);
	if(hFile == INVALID_HANDLE_VALUE)
		return;
	do {
		auto folder_name = fileinfo.cFileName;
		if(!(fileinfo.dwFileAttributes & _A_SUBDIR))
			continue;
		if(!wcscmp(folder_name, L".") || !wcscmp(folder_name, L".."))
			continue;
		track_list.push_back(folder_name);
	} while(FindNextFile(hFile, &fileinfo));
	FindClose(hFile);
}

void loadTrack(vector<wstring>::iterator tr_top, HWND hWnd) {
	wstring track_name = *tr_top;
	Scene::switchTo("play", hWnd, &track_name);
}

LRESULT init(HWND hWnd) {
	SetWindowLong(hWnd, GWL_STYLE, NULL);
	// ShowWindow(hWnd, SW_MAXIMIZE);
	getTrackList();
	loadTrack(track_list.begin(), hWnd);
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