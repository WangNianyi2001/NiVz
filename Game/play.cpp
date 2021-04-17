#include "Win32GameEngine/engine.h"
#include "config.h"
#include <vector>
#include <string>
#include <fstream>

using namespace Win32GameEngine;

LONGLONG start_tick;

struct Note {
	char note_type;
	int offset;
};

wstring name;
unsigned scroll_speed;
unsigned offset;
unsigned bpm;

LONGLONG getCurrentProgress() {
	return GetTickCount64() - start_tick - offset;
}

Bitmap t_note(L"../Textures/note.bmp");
struct Track {
	RECT area;
	vector<Note> notes_buffer, visible_notes;
	void updateNotes() {
		LONGLONG const progress = getCurrentProgress();
		LONGLONG const appear_tick = progress + scroll_speed;
		while(notes_buffer.size() && notes_buffer[0].offset <= appear_tick) {
			visible_notes.push_back(notes_buffer[0]);
			notes_buffer.erase(notes_buffer.begin());
		}
		LONGLONG const disappear = progress - overshoot_limit;
		while(visible_notes.size() && visible_notes[0].offset < disappear)
			visible_notes.erase(visible_notes.begin());
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
			LONGLONG const delta = note.offset - progress;
			int screen_x = area.left + delta * (area.right - area.left) / scroll_speed;
			t_note.paintOn(hdc, { screen_x, area.top });
		}
	}
} tr_top{ RECT{ 0, 0, vwidth, 64 } }, tr_bottom{ { RECT{ 0, 64, vwidth, 128 } } };

bool loadTrack(wstring file_dir) {
	wstring buffer;
	wifstream file(file_dir, ios::in);
	if(!file.is_open())
		return false;
	file >> buffer;
	name = buffer;
	file >> buffer;
	scroll_speed = stoi(buffer);
	file >> buffer;
	offset = stoi(buffer);
	file >> buffer;
	bpm = stoi(buffer);
	float top_offset = .0f, bottom_offset = .0f;
	int beat_length = 60000 / bpm;
	while(!file.eof()) {
		Note note;
		file >> buffer;
		auto const player_bits = stoi(buffer);
		file >> buffer;
		note.note_type = buffer[0];
		file >> buffer;
		auto const interval = stoi(buffer);
		if(player_bits & 1) {
			note.offset = top_offset * beat_length;
			top_offset += 1.0f / interval;
			tr_top.notes_buffer.push_back(note);
		}
		if(player_bits & 2) {
			note.offset = bottom_offset * beat_length;
			bottom_offset += 1.0f / interval;
			tr_bottom.notes_buffer.push_back(note);
		}
	}
	file.close();
	return true;
}

void init(HWND hWnd, void *p_track_name) {
	wstring &track_name = *(wstring *)p_track_name;
	// Load track from file
	auto file_dir = wstring(tracks_dir) + track_name + L"/score.txt";
	if(!loadTrack(file_dir)) {
		return;
	}
	start_tick = GetTickCount64() + (ULONGLONG)offset;
	SetTimer(hWnd, timer_id, 1000 / fps, NULL);
	return;
}

void quit(HWND hWnd) {
	KillTimer(hWnd, timer_id);
	tr_top.clear();
	tr_bottom.clear();
	return;
}

LRESULT paint(HWND hWnd, WPARAM, LPARAM) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	LONGLONG const progress = getCurrentProgress();
	tr_top.paint(vscreen.hdc, progress);
	tr_bottom.paint(vscreen.hdc, progress);
	vscreen.paintOn(hdc, { 0, 0 });
	EndPaint(hWnd, &ps);
	return 0;
}

LRESULT timer(HWND hWnd, WPARAM, LPARAM) {
	tr_top.updateNotes();
	tr_bottom.updateNotes();
	if(tr_top.done() && tr_bottom.done())
		quit(hWnd);
	InvalidateRect(hWnd, NULL, true);
	return 0;
}

Scene scene("play", &init, {
	{ WM_PAINT, { (EventHandler::Handler)&paint } },
	{ WM_TIMER, { (EventHandler::Handler)&timer } },
});