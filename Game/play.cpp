#include "includes.h"
#include <vector>
#include <map>
#include <string>
#include <fstream>

using namespace Win32GameEngine;

struct Note {
	char type;
	int track_offset;
};

constexpr char const null_note_type = 'n';

wstring name;
LONGLONG scroll_speed;
LONGLONG start_tick;
LONGLONG track_offset;
unsigned bpm;
int health;

void quit(HWND hWnd);

LONGLONG getCurrentProgress() {
	return GetTickCount64() - start_tick;
}

Bitmap *makeTexture(wchar_t const *file_name) {
	return new Bitmap((wstring(textures_dir) + file_name + L".bmp").c_str());
}

map<char, Texture> note_textures{
	{ 'w', Texture({
		{ 64, 64 },
		{ 0, 0 },
		makeTexture(L"w"),
		makeTexture(L"w.mask"),
	}) },
	{ 's', Texture({
		{ 64, 64 },
		{ 0, 0 },
		makeTexture(L"s"),
		makeTexture(L"s.mask"),
	}) },
	{ 'a', Texture({
		{ 64, 64 },
		{ 0, 0 },
		makeTexture(L"a"),
		makeTexture(L"a.mask"),
	}) },
	{ 'd', Texture({
		{ 64, 64 },
		{ 0, 0 },
		makeTexture(L"d"),
		makeTexture(L"d.mask"),
	}) },
};
struct Track {
	RECT area;
	vector<Note> notes_buffer, visible_notes;
	void updateNotes() {
		LONGLONG const progress = getCurrentProgress();
		LONGLONG const appear_tick = progress + scroll_speed;
		while(notes_buffer.size() && notes_buffer[0].track_offset <= appear_tick) {
			visible_notes.push_back(notes_buffer[0]);
			notes_buffer.erase(notes_buffer.begin());
		}
		LONGLONG const disappear = progress - overshoot_limit;
		while(visible_notes.size() && visible_notes[0].track_offset < disappear) {
			health -= 10;
			visible_notes.erase(visible_notes.begin());
		}
	}
	bool done() {
		return !visible_notes.size() && !notes_buffer.size();
	}
	void clear() {
		notes_buffer.clear();
		visible_notes.clear();
	}
	void paint(HDC hdc, LONGLONG progress) {
		for(Note note : visible_notes) {
			LONGLONG const delta = note.track_offset - progress;
			int screen_x = area.left + delta * (area.right - area.left) / scroll_speed;
			note_textures[note.type].paintOn(hdc, { screen_x, area.top });
		}
	}
	vector<Note>::iterator getHitNote(LONGLONG progress) {
		if(!visible_notes.size())
			return visible_notes.end();
		auto res = visible_notes.begin();
		// Find closest note
		for(
			auto it = res;
			it != visible_notes.end() &&
			abs(it->track_offset - progress) < abs(res->track_offset - progress);
			res = it++
		);
		if(res->track_offset - progress > overshoot_limit)
			return visible_notes.end();
		return res;
	}
	void hit(LONGLONG progress, char note_type) {
		auto it = getHitNote(getCurrentProgress());
		if(it == visible_notes.end())
			return;
		Note note = *it;
		visible_notes.erase(it);
		if(note.type != note_type)
			health -= 10;
		else {
			++health;
			if(health > 100)
				health = 100;
		}
	}
} tr_top{ RECT{ 0, 0, vwidth, 0 } }, tr_bottom{ { RECT{ 0, 128, vwidth, 0 } } };

bool loadTrack(wstring file_dir) {
	wstring buffer;
	wifstream file(file_dir, ios::in);
	if(!file.is_open())
		return false;
	getline(file, buffer);
	name = buffer;
	file >> buffer;
	scroll_speed = stoi(buffer);
	file >> buffer;
	track_offset = stoi(buffer);
	file >> buffer;
	bpm = stoi(buffer);
	float top_offset = .0f, bottom_offset = .0f;
	int beat_length = 60000 / bpm;
	while(!file.eof()) {
		Note note;
		file >> buffer;
		auto const player_bits = stoi(buffer);
		file >> buffer;
		note.type = buffer[0];
		file >> buffer;
		auto const interval = stoi(buffer);
		if(player_bits & 1) {
			note.track_offset = top_offset * beat_length;
			top_offset += 1.0f / interval;
			if(note.type != null_note_type)
				tr_top.notes_buffer.push_back(note);
		}
		if(player_bits & 2) {
			note.track_offset = bottom_offset * beat_length;
			bottom_offset += 1.0f / interval;
			if(note.type != null_note_type)
				tr_bottom.notes_buffer.push_back(note);
		}
	}
	file.close();
	return true;
}

void init(HWND hWnd, void *p_track_name) {
	health = 100;
	wstring &track_name = *(wstring *)p_track_name;
	// Load track from file
	auto file_dir = wstring(tracks_dir) + track_name + L"/score.txt";
	if(!loadTrack(file_dir)) {
		quit(hWnd);
		return;
	}
	start_tick = GetTickCount64() + (ULONGLONG)track_offset;
	auto bgm_dir = (wstring(tracks_dir) + track_name + L"/bgm.wav");
	playSound(bgm_dir.c_str());
	SetTimer(hWnd, timer_id, 1000 / fps, NULL);
	return;
}

void quit(HWND hWnd) {
	KillTimer(hWnd, timer_id);
	tr_top.clear();
	tr_bottom.clear();
	Scene::switchTo("level", hWnd, nullptr);
	playSound(nullptr, NULL);
	return;
}

Bitmap background((wstring(textures_dir) + L"background.bmp").c_str());
PureColor vinner(0, { 512, 192 });
LRESULT paint(HWND hWnd, WPARAM, LPARAM) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	background.paintOn(vscreen.hdc, { 0, 0 });
	background.paintOn(vinner.hdc, { -64, -84 });
	LONGLONG const progress = getCurrentProgress();
	tr_top.paint(vinner.hdc, progress);
	tr_bottom.paint(vinner.hdc, progress);
	vinner.paintOn(vscreen.hdc, { 64, 84 });
	RECT health_bar{ 0, 0, (long)(vwidth * health / 100.0), 8 };
	FillRect(vscreen.hdc, &health_bar, (HBRUSH)GetStockObject(WHITE_BRUSH));
	vscreen.paintOn(hdc, { 0, 0 });
	EndPaint(hWnd, &ps);
	return 0;
}

LRESULT timer(HWND hWnd, WPARAM, LPARAM) {
	tr_top.updateNotes();
	tr_bottom.updateNotes();
	if(health <= 0 || tr_top.done() && tr_bottom.done())
		quit(hWnd);
	InvalidateRect(hWnd, NULL, true);
	return 0;
}

map<char, pair<int, char>> key_map{
	{ 'W', { 1, 'w' } },
	{ 'S', { 1, 's' } },
	{ 'A', { 1, 'a' } },
	{ 'D', { 1, 'd' } },
	{ 'I', { 2, 'w' } },
	{ 'K', { 2, 's' } },
	{ 'J', { 2, 'a' } },
	{ 'L', { 2, 'd' } },
};
LRESULT keydown(HWND hWnd, WPARAM keycode, LPARAM) {
	if(key_map.count(keycode)) {
		auto p = key_map[(char)keycode];
		(p.first == 1 ? tr_top : tr_bottom).hit(getCurrentProgress(), p.second);
	}
	return 0;
}

Scene scene("play", &init, {
	{ WM_PAINT, { (EventHandler::Handler)&paint } },
	{ WM_TIMER, { (EventHandler::Handler)&timer } },
	{ WM_KEYDOWN, { (EventHandler::Handler)&keydown } },
});