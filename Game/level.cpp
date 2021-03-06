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

auto hFont = CreateFont(
	48, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
	CLIP_DEFAULT_PRECIS, DRAFT_QUALITY, VARIABLE_PITCH, L"UD Digi Kyokasho N-B"
);
Bitmap level_background((wstring(textures_dir) + L"start.bmp").c_str());
LRESULT level_paint(HWND hWnd, WPARAM, LPARAM) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	level_background.paintOn(hdc, { 0, 0 });
	SelectObject(hdc, hFont);
	SetTextAlign(hdc, TA_CENTER | TA_BASELINE);
	TextOutW(hdc, vwidth / 2, vheight / 2, curr->c_str(), curr->size());
	EndPaint(hWnd, &ps);
	return 0;
}

LRESULT level_keydown(HWND hWnd, WPARAM keycode, LPARAM) {
	switch(keycode) {
	case VK_RETURN:
		loadTrack(curr, hWnd);
		return 0;
	case 'A':
	case VK_LEFT:
		if(curr == track_list.begin())
			curr = track_list.end();
		--curr;
		InvalidateRect(hWnd, NULL, true);
		return 0;
	case 'D':
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