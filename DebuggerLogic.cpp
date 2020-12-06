#include "DebuggerLogic.h"

int LaunchDebugCycle()
{

	LPCWSTR ProcessNameToDebug = L"C:\\5 semester\\Course project\\DebugMe.exe";

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));


	CreateProcess(ProcessNameToDebug, NULL, NULL, NULL, false, DEBUG_ONLY_THIS_PROCESS,
		NULL, NULL, &si, &pi);

	std::string strEventMessage;
	std::map<LPVOID, std::string> DllNameMap;


	DEBUG_EVENT debug_event = { 0 };

	bool bContinueDebugging = true;

	DWORD dwContinueStatus = DBG_CONTINUE;

	char buff[100];

	while (bContinueDebugging)
	{
		if (!WaitForDebugEvent(&debug_event, INFINITE))
			return 0;

		switch (debug_event.dwDebugEventCode)
		{
		case CREATE_PROCESS_DEBUG_EVENT:
		{
			//strEventMessage = GetFileNameFromHandle(debug_event.u.CreateProcessInfo.hFile);
		}
		break;

		case CREATE_THREAD_DEBUG_EVENT:
			//char buff[100];
			snprintf(buff, sizeof(buff), "Thread 0x%x (Id: %d) created at: 0x%x",
				debug_event.u.CreateThread.hThread,
				debug_event.dwThreadId,
				debug_event.u.CreateThread.lpStartAddress);
			strEventMessage = buff;
			std::cout << strEventMessage;
			break;
		case EXIT_THREAD_DEBUG_EVENT:
			//char buff[100];
			snprintf(buff, sizeof(buff), "The thread %d exited with code: %d",
				debug_event.dwThreadId,
				debug_event.u.ExitThread.dwExitCode);
			strEventMessage = buff;
			std::cout << strEventMessage;
			// The thread 2760 exited with code: 0

			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			//strEventMessage.Format(L"0x%x", debug_event.u.ExitProcess.dwExitCode);
			bContinueDebugging = false;
			break;

		case LOAD_DLL_DEBUG_EVENT:
		{
			strEventMessage = GetFileNameFromHandle(debug_event.u.LoadDll.hFile);

			DllNameMap.insert(
				std::make_pair(debug_event.u.LoadDll.lpBaseOfDll, strEventMessage));

			snprintf(buff, sizeof(buff), "%x",
				debug_event.u.LoadDll.lpBaseOfDll);
			std::string strBaseOfDll = buff;

			strEventMessage.append(strBaseOfDll);

		}
		break;

		case UNLOAD_DLL_DEBUG_EVENT:
			//strEventMessage.Format(L"%s", DllNameMap[debug_event.u.UnloadDll.lpBaseOfDll]);
			break;

		case OUTPUT_DEBUG_STRING_EVENT:
		{
			OUTPUT_DEBUG_STRING_INFO& DebugString = debug_event.u.DebugString;
			// LPSTR p = ;

			WCHAR* msg = new WCHAR[DebugString.nDebugStringLength];
			ZeroMemory(msg, DebugString.nDebugStringLength);

			ReadProcessMemory(pi.hProcess, DebugString.lpDebugStringData, msg, DebugString.nDebugStringLength, NULL);

			if (DebugString.fUnicode)
				strEventMessage = (LPSTR)msg;
			else
				strEventMessage = (LPSTR)msg;

			delete[]msg;
		}

		break;

		case EXCEPTION_DEBUG_EVENT:
		{
			EXCEPTION_DEBUG_INFO& exception = debug_event.u.Exception;
			switch (exception.ExceptionRecord.ExceptionCode)
			{
			case STATUS_BREAKPOINT:
				strEventMessage = "Break point";
				break;

			default:
				if (exception.dwFirstChance == 1)
				{
					//char buff[100];
					snprintf(buff, sizeof(buff), "First chance exception at %x, exception-code: 0x%08x",
						exception.ExceptionRecord.ExceptionAddress,
						exception.ExceptionRecord.ExceptionCode);
					strEventMessage = buff;
					std::cout << strEventMessage;
				}
				// else
				// { Let the OS handle }


				// There are cases where OS ignores the dwContinueStatus, 
				// and executes the process in its own way.
				// For first chance exceptions, this parameter is not-important
				// but still we are saying that we have NOT handled this event.

				// Changing this to DBG_CONTINUE (for 1st chance exception also), 
				// may cause same debugging event to occur continously.
				// In short, this debugger does not handle debug exception events
				// efficiently, and let's keep it simple for a while!
				dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
			}

			break;
		}
		}

		//SendMessage(DEBUG_EVENT_MESSAGE, (WPARAM)&strEventMessage, debug_event.dwDebugEventCode);

		ContinueDebugEvent(debug_event.dwProcessId,
			debug_event.dwThreadId,
			dwContinueStatus);

		// Reset
		dwContinueStatus = DBG_CONTINUE;
	}
	return 0;
}

std::string GetFileNameFromHandle(HANDLE hFile)
{
	BOOL bSuccess = FALSE;
	TCHAR pszFilename[MAX_PATH + 1];
	HANDLE hFileMap;

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
									char buff[100];
									snprintf(buff, sizeof(buff), "%s%s", szDrive, pszFilename);
									strFilename = buff;
									std::wcout << szDrive << pszFilename << std::endl;

									//std::cout << strFilename;

									//strFilename.Format(L"%s%s", szDrive, pszFilename + uNameLen);
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

	return strFilename;
}