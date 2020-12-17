#include "DebuggerLogic.h"

RECT rect = { 320, 10, 520, 110 };

static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;
static HANDLE self;
int* chartArray;
bool isFinished = false;
BYTE m_OriginalInstruction;
LPVOID pStartAddress;
TCHAR buffer[MSG_EVENT_BUF_LENGTH];
TCHAR tempBuffer[MSG_EVENT_BUF_LENGTH];
bool firstBreakPointHit = true;
CONTEXT lcContext;


int LaunchDebugCycle(LPCWSTR ProcessNameToDebug, HWND* hGUIElements, int* pChartArray)
{
	
	lcContext.ContextFlags = CONTEXT_ALL;
	isFinished = false;
	chartArray = pChartArray;
	eventsCount = 0;
	threadsCount = 0;
	dllCount = 0;
	exceptionCount = 0;
	dwContinueStatus = DBG_CONTINUE;
	bool isContinueDebugging = true;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	CreateProcess(ProcessNameToDebug,
		NULL,
		NULL,
		NULL,
		false,
		DEBUG_ONLY_THIS_PROCESS,
		NULL,
		NULL,
		&si,
		&pi
	);

	SymInitialize(pi.hProcess, NULL, false);

	InitPCSystemParams();

	std::thread cpuCount(UpdateUsageCPUChart, hGUIElements, 0);
	
	buffer[0] = NULL;
		
	while (isContinueDebugging)
	{
		if (!WaitForDebugEvent(&debug_event, INFINITE))
			return 0;
		
		switch (debug_event.dwDebugEventCode)
		{
			case CREATE_PROCESS_DEBUG_EVENT:
			{
				
				//ReplaceInstruction();

				EnableWindow(hGUIElements[1], false);

				TCHAR* pStrPart = buffer;

				memcpy(tempBuffer, GetFileNameFromHandle(debug_event.u.CreateProcessInfo.hFile), MSG_EVENT_BUF_LENGTH);

				_sntprintf_s(buffer,
					MSG_EVENT_BUF_LENGTH - 1,
					STR_CREATE_PROCESS,
					debug_event.u.LoadDll.lpBaseOfDll
				);

				pStrPart += GetStringLength(buffer);

				memcpy(pStrPart, tempBuffer, MAX_PATH);

			}
			break;

			case CREATE_THREAD_DEBUG_EVENT:
			{
				_sntprintf_s(buffer,
					MSG_EVENT_BUF_LENGTH - 1,
					STR_CREATE_THREAD,
					debug_event.u.CreateThread.hThread,
					debug_event.dwThreadId,
					debug_event.u.CreateThread.lpStartAddress);

				SetGUILabel(hGUIElements[2], ++threadsCount, STR_LABEL_THREADS_COUNT);
			}
			break;

			case EXIT_THREAD_DEBUG_EVENT:
			{
				_sntprintf_s(buffer,
					MSG_EVENT_BUF_LENGTH - 1,
					STR_EXIT_THREAD,
					debug_event.dwThreadId,
					debug_event.u.ExitThread.dwExitCode);
				SetGUILabel(hGUIElements[2], --threadsCount, STR_LABEL_THREADS_COUNT);
			}
			break;

			case EXIT_PROCESS_DEBUG_EVENT:
			{
				_sntprintf_s(buffer,
					MSG_EVENT_BUF_LENGTH - 1,
					STR_EXIT_PROCESS,
					(int)debug_event.u.ExitProcess.dwExitCode);

				EnableWindow(hGUIElements[1], true);

				dllNameMap.clear();

				for (int i = 0; i < 200; i++)
				{
					chartArray[i] = -1;
				}

				isFinished = true;

				cpuCount.join();

				isContinueDebugging = false;
			}
			break;

			case LOAD_DLL_DEBUG_EVENT:
			{	
				//RetrieveCallstack(pi.hThread);

				TCHAR* pStrPart = buffer;

				TCHAR* dllNameBuffer = new TCHAR[MSG_EVENT_BUF_LENGTH];

				memcpy(buffer, GetFileNameFromHandle(debug_event.u.LoadDll.hFile), MSG_EVENT_BUF_LENGTH);
				memcpy(dllNameBuffer, GetFileNameFromHandle(debug_event.u.LoadDll.hFile), MSG_EVENT_BUF_LENGTH);

				dllNameMap.insert(
					std::pair<LPVOID, TCHAR*>(debug_event.u.LoadDll.lpBaseOfDll, dllNameBuffer)
				);

				pStrPart += GetStringLength(buffer);

				_sntprintf_s(tempBuffer,
					MSG_EVENT_BUF_LENGTH - 1,
					STR_LOAD_DLL,
					debug_event.u.LoadDll.lpBaseOfDll
				);

				memcpy(pStrPart, tempBuffer, MAX_PATH);

				SetGUILabel(hGUIElements[4], ++dllCount, STR_LABEL_DLL_COUNT);

				UpdateRegisterLabel(pi.hThread, hGUIElements);

				WaitForSingleObject(hContinueEvent, INFINITE);//wait for user click to continue debugging
				ResetEvent(hContinueEvent);
			
			}
			break;

			case UNLOAD_DLL_DEBUG_EVENT:
			{
				
				_sntprintf_s(buffer,
					MSG_EVENT_BUF_LENGTH - 1,
					STR_UNLOAD_DLL,
					dllNameMap[debug_event.u.UnloadDll.lpBaseOfDll],
					debug_event.u.UnloadDll.lpBaseOfDll
				);

				SetGUILabel(hGUIElements[4], --dllCount, STR_LABEL_DLL_COUNT);

			}
			break;

			case EXCEPTION_DEBUG_EVENT:
			{
				EXCEPTION_DEBUG_INFO & exception = debug_event.u.Exception;
				
				switch (exception.ExceptionRecord.ExceptionCode)
				{
					case EXCEPTION_BREAKPOINT:
					{
						
						WaitForSingleObject(hContinueEvent, INFINITE);//wait for user click to continue debugging
						ResetEvent(hContinueEvent);
						
						GetThreadContext(pi.hThread, &lcContext);
						
						SetMessageForGUI(buffer, STR_EXCEPTION_BREAKPOINT);

						dwContinueStatus = DBG_CONTINUE;
					}
					break;

					case EXCEPTION_ACCESS_VIOLATION:
					{
						SetMessageForGUI(buffer, STR_EXCEPTION_ACCESS_VIOLATION);
						dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
					}
					break;

					case EXCEPTION_INT_DIVIDE_BY_ZERO:
					case EXCEPTION_FLT_DIVIDE_BY_ZERO:
					{
						SetMessageForGUI(buffer, STR_EXCEPTION_DIVIDE_BY_ZERO);
						dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
					}
					break;

					case EXCEPTION_STACK_OVERFLOW:
					{
						SetMessageForGUI(buffer, STR_EXCEPTION_STACK_OVERFLOW);
						dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
					}
					break;

					case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
					{
						SetMessageForGUI(buffer, STR_EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
						dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
					}
					break;
				}

				UpdateRegisterLabel(pi.hThread, hGUIElements);

				WaitForSingleObject(hContinueEvent, INFINITE);//wait for user click to continue debugging
				ResetEvent(hContinueEvent);

				SetGUILabel(hGUIElements[3], ++exceptionCount, STR_LABEL_EXCEPTION_COUNT);

			}
			break;

		}

		if (buffer[0] != NULL)
		{
			SendMessage(hGUIElements[0], LB_ADDSTRING, 0, (LPARAM)(LPCWSTR)buffer);

			buffer[0] = NULL;

			SetGUILabel(hGUIElements[5], ++eventsCount, STR_LABEL_EVENTS_COUNT);
		}

		ContinueDebugEvent(debug_event.dwProcessId,
			debug_event.dwThreadId,
			dwContinueStatus);

		dwContinueStatus = DBG_CONTINUE;
	}
	return 0;
}

int GetStringLength(TCHAR* string)
{
	int stringLength = 0;
	for (int i = 0; i < 512; i++) 
	{
		if (string[i] == 0)
		{
			stringLength = i;
			break;
		}
	}
	return stringLength;
}

TCHAR* GetFileNameFromHandle(HANDLE hFile)
{
	BOOL bSuccess = FALSE;
	TCHAR pszFilename[MAX_PATH + 1];
	HANDLE hFileMap;
	TCHAR buff[512];

	std::string strFilename;

	// Get the file size.
	DWORD dwFileSizeHi = 0;
	DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi);

	if (dwFileSizeLo == 0 && dwFileSizeHi == 0)
	{
		return FALSE;
	}

	// Create a file mapping object.
	hFileMap = CreateFileMapping(hFile,
		NULL,
		PAGE_READONLY,
		0,
		1,
		NULL);

	if (hFileMap)
	{
		// Create a file mapping to get the file name.
		void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

		if (pMem)
		{
			if (GetMappedFileName(GetCurrentProcess(),
				pMem,
				pszFilename,
				MAX_PATH))
			{

				// Translate path with device name to drive letters.
				TCHAR szTemp[512];
				szTemp[0] = '\0';

				if (GetLogicalDriveStrings(511, szTemp))
				{
					TCHAR szName[MAX_PATH];
					TCHAR szDrive[3] = TEXT(" :");
					BOOL bFound = FALSE;
					TCHAR* p = szTemp;

					do
					{
						// Copy the drive letter to the template string
						*szDrive = *p;

						// Look up each device name
						if (QueryDosDevice(szDrive, szName, MAX_PATH))
						{
							size_t uNameLen = _tcslen(szName);
							(szName);
							if (uNameLen < MAX_PATH)
							{
								bFound = _tcsnicmp(pszFilename, szName,
									uNameLen) == 0;

								if (bFound)
								{
									//TCHAR buff[512];
									_sntprintf_s(buff, sizeof(buff), _T("%s%s\0"), szDrive, pszFilename);
									//strFilename = buff;
									//std::wcout << szDrive << pszFilename << std::endl;
								}
							}
						}

						// Go to the next NULL character.
						while (*p++);
					} while (!bFound && *p); // end of string
				}
			}
			bSuccess = TRUE;
			UnmapViewOfFile(pMem);
		}

		CloseHandle(hFileMap);
	}

	return buff;
}

void SetGUILabel(HWND hLabel, int count, LPCWSTR str) 
{
	TCHAR tempBuffer[MSG_EVENT_BUF_LENGTH];

	_sntprintf_s(tempBuffer,
		MSG_EVENT_BUF_LENGTH - 1,
		str,
		count);

	SetWindowText(hLabel, tempBuffer);
}

void SetMessageForGUI(TCHAR* buffer, LPCWSTR str)
{
	_snwprintf_s(buffer,
		MSG_EVENT_BUF_LENGTH,
		MSG_EVENT_BUF_LENGTH,
		str
	);
}

void InitPCSystemParams() {

	self = pi.hProcess;

	for (int i = 0; i < 200; i++)
	{
		chartArray[i] = -1;
	}

	SYSTEM_INFO sysInfo;
	FILETIME ftime, fsys, fuser;

	GetSystemInfo(&sysInfo);
	numProcessors = sysInfo.dwNumberOfProcessors;

	GetSystemTimeAsFileTime(&ftime);
	memcpy(&lastCPU, &ftime, sizeof(FILETIME));

	GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
	memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
	memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
}

double GetCurrentCPULoadValue() 
{
	FILETIME ftime, fsys, fuser;
	ULARGE_INTEGER now, sys, user;
	double percent;

	GetSystemTimeAsFileTime(&ftime);
	memcpy(&now, &ftime, sizeof(FILETIME));

	GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
	memcpy(&sys, &fsys, sizeof(FILETIME));
	memcpy(&user, &fuser, sizeof(FILETIME));
	percent = (sys.QuadPart - lastSysCPU.QuadPart) +
		(user.QuadPart - lastUserCPU.QuadPart);
	percent /= (now.QuadPart - lastCPU.QuadPart);
	percent /= numProcessors;
	lastCPU = now;
	lastUserCPU = user;
	lastSysCPU = sys;

	return percent * 100;
}

void UpdateUsageCPUChart(HWND* hGUIElements, int newValue)
{

	/* this code is for implementing memory usage feature
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
	virtualMemUsedByMe / (1024 * 1024);
	*/

	for (;;)
	{
		if (isFinished) {
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		for (int i = 1; i < 200; i++)
		{
			chartArray[i - 1] = chartArray[i];
		}
		chartArray[199] = GetCurrentCPULoadValue();

		InvalidateRect(hGUIElements[7], &rect, FALSE);

		UpdateMemUsageLabel(hGUIElements[6]);
	}
}

DWORD GetStartAddress(HANDLE hProcess, HANDLE hThread)
{
	SymInitialize(pi.hProcess, NULL, false);
	SYMBOL_INFO* pSymbol;
	pSymbol = (SYMBOL_INFO*)new BYTE[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
	pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	pSymbol->MaxNameLen = MAX_SYM_NAME;
	SymFromName(hProcess, "mainCRTStartup", pSymbol);

	DWORD dwAddress = pSymbol->Address;

	delete[](BYTE*)pSymbol;

	return dwAddress;
}

void ReplaceInstruction()
{
	pStartAddress = (LPVOID)debug_event.u.CreateProcessInfo.lpStartAddress;

	DWORD dwStartAddress = GetStartAddress(pi.hProcess, pi.hThread);
	//DWORD dwStartAddress = ;
	BYTE currentInstruction;
	DWORD dwReadBytes;

	// Read the first instruction    
	ReadProcessMemory(pi.hProcess, (void*)dwStartAddress, &currentInstruction, 1, &dwReadBytes);

	// Save first instruction!
	m_OriginalInstruction = currentInstruction;

	// Replace it with Breakpoint
	currentInstruction = 0xCC;//ASM_INT_3;
	WriteProcessMemory(pi.hProcess, (void*)dwStartAddress, &currentInstruction, 1, &dwReadBytes);
	FlushInstructionCache(pi.hProcess, (void*)dwStartAddress, 1);
}

void RetrieveCallstack(HANDLE hThread)
{
	STACKFRAME64 stack = { 0 };
	// Initialize 'stack' with some required stuff.

	CONTEXT context;
	context.ContextFlags = CONTEXT_FULL;
	GetThreadContext(hThread, &context);

	// Must be like this
	stack.AddrPC.Offset = context.Eip; // EIP - Instruction Pointer
	stack.AddrPC.Mode = AddrModeFlat;
	stack.AddrFrame.Offset = context.Ebp; // EBP
	stack.AddrFrame.Mode = AddrModeFlat;
	stack.AddrStack.Offset = context.Esp; // ESP - Stack Pointer
	stack.AddrStack.Mode = AddrModeFlat;

	

	BOOL bSuccess;
	do
	{
		bSuccess = StackWalk64(IMAGE_FILE_MACHINE_I386, pi.hProcess, hThread, &stack,
			&context, NULL, SymFunctionTableAccess64,
			SymGetModuleBase64, 0);
		if (!bSuccess)

			break;

		IMAGEHLP_MODULE64 module = { 0 };
		module.SizeOfStruct = sizeof(module);
		SymGetModuleInfo64(pi.hProcess, (DWORD64)stack.AddrPC.Offset, &module);

		IMAGEHLP_SYMBOL64 * pSymbol;
		DWORD dwDisplacement;
		pSymbol = (IMAGEHLP_SYMBOL64*)new BYTE[sizeof(IMAGEHLP_SYMBOL64) + MAX_SYM_NAME];

		memset(pSymbol, 0, sizeof(IMAGEHLP_SYMBOL64) + MAX_SYM_NAME);
		pSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64); // Required
		pSymbol->MaxNameLength = MAX_SYM_NAME;             // Required

		SymGetSymFromAddr64(pi.hProcess, stack.AddrPC.Offset,
			(PDWORD64)&dwDisplacement, pSymbol);

	} while (stack.AddrReturn.Offset != 0);

}

void UpdateRegisterLabel(HANDLE hThread, HWND* hGUIElements)
{
	CONTEXT cpuContext;
	cpuContext.ContextFlags = CONTEXT_ALL;

	GetThreadContext(hThread, &lcContext);

	TCHAR buf[512];

	_sntprintf_s(buf,
		MSG_EVENT_BUF_LENGTH - 1,
		STR_REGISTERS,
		lcContext.Eax, lcContext.Ebx, lcContext.Ecx,
		lcContext.Edx, lcContext.Esi, lcContext.Edi,
		lcContext.Eip, lcContext.Esp, lcContext.Ebp,
		lcContext.EFlags
	);

	SetWindowText(hGUIElements[9], buf);
}

void UpdateMemUsageLabel(HWND hLabel)
{
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(pi.hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
	TCHAR buf[512];

	if (virtualMemUsedByMe / (1024 * 1024))
	{
		float memUsage = virtualMemUsedByMe / (1024 * 1024);
		_sntprintf_s(buf,
			MSG_EVENT_BUF_LENGTH - 1,
			STR_MEM_USAGE_MB,
			memUsage
		);
	}
	else
	{
		float memUsage = virtualMemUsedByMe / 1024;
		_sntprintf_s(buf,
			MSG_EVENT_BUF_LENGTH - 1,
			STR_MEM_USAGE_KB,
			(float)memUsage
		);
	}
	

	SetWindowText(hLabel, buf);
}