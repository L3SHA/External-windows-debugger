#pragma once

#include "DebuggerLogic.h"

#define DEFAULT_WINDOW_WIDTH 1000
#define DEFAULT_WINDOW_HEIGHT 600

#define STR_START_DEBUG L"Start debugging"
#define STR_CONTINUE_DEBUG L"Continue debugging"
#define OPEN_DIALOG_FILTER L"Executable Files (*.exe)\0*.exe\0"
#define OPEN_DIALOG_DEFAULT_EXTENSION L"exe"

#define BTN_START_DEBUG_PROCESS 0x00
#define BTN_CONTINUE_DEBUG_PROCESS 0x01

static HWND hListBoxEvents = NULL;
static HWND hButtonDebug = NULL;
static HWND hLabelThreads = NULL;
static HWND hLabelTotalEvents = NULL;
static HWND hLabelExceptionCount = NULL;
static HWND hLabelDllCount = NULL;
static HWND hLabelMemUsage = NULL;
static HWND hButtonContinue = NULL;
static HWND hLabelRegAndStack = NULL;
static RECT chartRect = {};
static int* chartArray = new int[200];
static OPENFILENAME ofn = {};
static WCHAR szFileName[MAX_PATH];
static HWND* pGUIElements = new HWND[10];
static HDC hdc = NULL;
static PAINTSTRUCT ps = {};
static WNDCLASS wc = { };

const TCHAR CLASS_NAME[] = L"Debugger Main Menu";
const TCHAR WINDOW_NAME[] = L"Cool debugger";
const COLORREF rgbRed = 0x000000FF;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitControls(HWND);
void ChooseFile(OPENFILENAME*, HWND);
void DrawCPUChart(HDC, PAINTSTRUCT, HBRUSH);
DWORD WINAPI LaunchDebugCycleWrapper(LPVOID);
void ClearChartArray(int*);