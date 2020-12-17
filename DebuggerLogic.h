#pragma once
#pragma comment( lib, "dbghelp.lib" )

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <string>
#include <map>
#include <psapi.h>
#include <tchar.h>
#include <thread>
#include <dbghelp.h>

#define STR_CREATE_PROCESS L"Process created: "
#define STR_EXIT_PROCESS L"Process exit with code: %d"
#define STR_CREATE_THREAD L"Thread 0x%x (Id: %d) created at: 0x%x"
#define STR_EXIT_THREAD L"The thread %d exited with code: %d"
#define STR_LOAD_DLL L" DLL loaded: at %x"
#define STR_UNLOAD_DLL L"%s DLL unloaded: at %x"
#define STR_REGISTERS L"EAX = %08X\nEBX = %08X\nECX = %08X\nEDX = %08X\nESI = %08X\nEDI = %08X\nEIP = %08X\nESP = %08X\nEBP = %08X\nEFL = %08X"
#define STR_MEM_USAGE_MB L"Memory usage: %f MB"
#define STR_MEM_USAGE_KB L"Memory usage: %f KB"

#define STR_LABEL_THREADS_COUNT L"Threads count: %d"
#define STR_LABEL_EVENTS_COUNT L"Events count: %d"
#define STR_LABEL_EXCEPTION_COUNT L"Exceptions count: %d"
#define STR_LABEL_DLL_COUNT L"Dlls count: %d"

#define STR_EXCEPTION_BREAKPOINT L"Break point event"
#define STR_EXCEPTION_DIVIDE_BY_ZERO L"Exception: Division by zero!"
#define STR_EXCEPTION_ACCESS_VIOLATION L"Exception: Access violation!"
#define STR_EXCEPTION_ARRAY_BOUNDS_EXCEEDED L"Exception: Array bouns exceeded!"
#define STR_EXCEPTION_STACK_OVERFLOW L"Exception: Stack overflow!"

#define ASM_INT_3 0xCC

static const size_t MSG_EVENT_BUF_LENGTH = 512;
static int eventsCount = 0;
static int threadsCount = 0;
static int dllCount = 0;
static int exceptionCount = 0;
static std::map<LPVOID, TCHAR*> dllNameMap;
static DEBUG_EVENT debug_event = {  };
static DWORD dwContinueStatus = DBG_CONTINUE;
static STARTUPINFO si;
static PROCESS_INFORMATION pi;
static HANDLE hContinueEvent = CreateEvent(
	NULL,               
	TRUE,               
	FALSE,              
	TEXT("ContinueEvent") 
);

void SetGUILabel(HWND, int, LPCWSTR);
void SetMessageForGUI(TCHAR*, LPCWSTR);
int GetStringLength(TCHAR*);
TCHAR* GetFileNameFromHandle(HANDLE);
int LaunchDebugCycle(LPCWSTR, HWND*, int*);
void UpdateUsageCPUChart(HWND*, int newValue);
double GetCurrentCPULoadValue();
void InitPCSystemParams();
DWORD GetStartAddress(HANDLE, HANDLE);
void ReplaceInstruction();
void RetrieveCallstack(HANDLE);
void UpdateRegisterLabel(HANDLE, HWND*);
void UpdateMemUsageLabel(HWND);