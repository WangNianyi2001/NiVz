#pragma once

#include "includes.h"
#include <windows.h>

static constexpr float const fps = 60.0;
static constexpr unsigned const timer_id = 0;
static constexpr int const overshoot_limit = 200;

static constexpr int const vwidth = 640, vheight = 360;
static Win32GameEngine::PureColor vscreen(0, { vwidth, vheight });
static RECT vrect = { 0, 0, vwidth, vheight };

static constexpr wchar_t const *tracks_dir = L"../Tracks/";