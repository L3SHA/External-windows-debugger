#pragma once

#include <windows.h>

#define DEFAULT_WINDOW_WIDTH 800
#define DEFAULT_WINDOW_HEIGHT 600

const TCHAR CLASS_NAME[] = L"Debugger Main Menu";
const TCHAR WINDOW_NAME[] = L"Cool debugger";

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
