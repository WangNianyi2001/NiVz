#include "includes.h"
#include <io.h>
#include <string>

vector<wstring> track_list;
vector<wstring>::iterator curr;
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
	curr = track_list.begin();
}

void loadTrack(vector<wstring>::iterator tr_top, HWND hWnd) {
	wstring track_name = *tr_top;
	Scene::switchTo("play", hWnd, &track_name);
}

void level_init(HWND hWnd, void *) {
	getTrackList();
	InvalidateRect(hWnd, NULL, true);
}

LRESULT level_paint(HWND hWnd, WPARAM, LPARAM) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	TextOutW(hdc, 0, 0, curr->c_str(), curr->size());
	EndPaint(hWnd, &ps);
	return 0;
}

LRESULT level_keydown(HWND hWnd, WPARAM keycode, LPARAM) {
	switch(keycode) {
	case VK_RETURN:
		loadTrack(curr, hWnd);
		return 0;
	case VK_LEFT:
		if(curr == track_list.begin())
			curr = track_list.end();
		--curr;
		InvalidateRect(hWnd, NULL, true);
		return 0;
	case VK_RIGHT:
		++curr;
		if(curr == track_list.end())
			curr = track_list.begin();
		InvalidateRect(hWnd, NULL, true);
		return 0;
	}
	return 0;
}

Scene level_scene("level", &level_init, {
	{ WM_KEYDOWN, { (EventHandler::Handler)&level_keydown } },
	{ WM_PAINT, { (EventHandler::Handler)&level_paint } },
});