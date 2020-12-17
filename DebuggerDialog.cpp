#include "DebuggerDialog.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        WINDOW_NAME,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL
    ); 

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    HBRUSH hBrush = CreateSolidBrush(rgbRed);
    
    switch (uMsg)
    {

        case WM_CREATE:
        {           
            ClearChartArray(chartArray);
            InitControls(hwnd);
        }
        break;

        case WM_DESTROY:
        {
            PostQuitMessage(0);
        }
        break;

        case WM_PAINT:
        {
            hdc = BeginPaint(hwnd, &ps);

            SetDCBrushColor(hdc, RGB(255, 0, 0));

            Rectangle(hdc, 320, 10, 520, 110);

            DrawCPUChart(hdc, ps, hBrush);

            EndPaint(hwnd, &ps);
        }
        break;

        case WM_COMMAND:
        {
            switch (wParam)
            {
                case BTN_START_DEBUG_PROCESS:
                {                  

                    ChooseFile(&ofn, hwnd);
                    
                    HANDLE hThreadArray = CreateThread(
                        NULL,                
                        0,  
                        LaunchDebugCycleWrapper,      
                        NULL,       
                        0,                   
                        NULL);              
                    
                }
                break;

                case BTN_CONTINUE_DEBUG_PROCESS:
                {
                    SetEvent(hContinueEvent);//dispatch event to continue debugging process       
                }
                break;

            }
        }
        break;

        case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = DEFAULT_WINDOW_WIDTH;
            lpMMI->ptMinTrackSize.y = DEFAULT_WINDOW_HEIGHT;

            lpMMI->ptMaxTrackSize.x = DEFAULT_WINDOW_WIDTH;
            lpMMI->ptMaxTrackSize.y = DEFAULT_WINDOW_HEIGHT;
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

}

void ChooseFile(OPENFILENAME* ofn, HWND hwnd)
{
    ofn->lStructSize = sizeof(OPENFILENAME);
    ofn->hwndOwner = hwnd;
    ofn->lpstrFilter = OPEN_DIALOG_FILTER;
    ofn->lpstrFile = NULL;
    ofn->nMaxFile = MAX_PATH;
    ofn->Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn->lpstrDefExt = OPEN_DIALOG_DEFAULT_EXTENSION;
    ofn->lpstrFile = szFileName;
    ofn->lpstrFile[0] = '\0';

    GetOpenFileName(ofn);
}

void InitControls(HWND hwnd)
{

    hLabelThreads = CreateWindow(L"STATIC", L"Threads count: 0",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        10, 10, 150, 20,
        hwnd, NULL,
        NULL, NULL);

    hLabelDllCount = CreateWindow(L"STATIC", L"Dlls count: 0",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        10, 40, 150, 20,
        hwnd, NULL,
        NULL, NULL);

    hLabelExceptionCount = CreateWindow(L"STATIC", L"Exceptions count: 0",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        160, 10, 150, 20,
        hwnd, NULL,
        NULL, NULL);

    hLabelTotalEvents = CreateWindow(L"STATIC", L"Events count: 0",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        160, 40, 150, 20,
        hwnd, NULL,
        NULL, NULL);

    hLabelMemUsage = CreateWindow(L"STATIC", L"Memory usage: 0 MB",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        160, 70, 150, 40,
        hwnd, NULL,
        NULL, NULL);

    hLabelRegAndStack = CreateWindow(L"STATIC", L"Registers: ",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        860, 10, 120, 190,
        hwnd, NULL,
        NULL, NULL);

    hButtonDebug = CreateWindow(
        TEXT("BUTTON"),
        STR_START_DEBUG,
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        530, 75, 120, 25,
        hwnd, 
        (HMENU) BTN_START_DEBUG_PROCESS, 
        NULL, NULL
    );

    hButtonContinue = CreateWindow(
        TEXT("BUTTON"),
        STR_CONTINUE_DEBUG,
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        530, 45, 150, 25,
        hwnd, 
        (HMENU) BTN_CONTINUE_DEBUG_PROCESS,
        NULL, NULL
    );

    hListBoxEvents = CreateWindow(
        TEXT("ListBox"),
        TEXT("Debug Events"),
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | WS_BORDER | LBS_NOTIFY,
        5, 150, 845, 415,
        hwnd, NULL, NULL, NULL
    );

    SendMessage(hListBoxEvents, LB_SETHORIZONTALEXTENT, 1200, 0);

    pGUIElements[0] = hListBoxEvents;
    pGUIElements[1] = hButtonDebug;
    pGUIElements[2] = hLabelThreads;
    pGUIElements[3] = hLabelExceptionCount;
    pGUIElements[4] = hLabelDllCount;
    pGUIElements[5] = hLabelTotalEvents;
    pGUIElements[6] = hLabelMemUsage;
    pGUIElements[7] = hwnd;
    pGUIElements[8] = hButtonContinue;
    pGUIElements[9] = hLabelRegAndStack;
}

void DrawCPUChart(HDC hdc, PAINTSTRUCT ps, HBRUSH hBrush)
{
    for (int i = 1; i < 200; i++)
    {
        if (chartArray[i] != -1) {
            chartRect = { 320 + i, 105 - chartArray[i], 320 + i + 1, 106 };

            FillRect(hdc, &chartRect, hBrush);

        }
    }
}

void ClearChartArray(int* chartArray)
{
    for (int i = 0; i < 200; i++)
    {
        chartArray[i] = -1;
    }
}

//result and parameteres are not used, they are for winapi CreateThread() function
DWORD WINAPI LaunchDebugCycleWrapper(LPVOID lpParameter)
{
    LaunchDebugCycle(ofn.lpstrFile, pGUIElements, chartArray);

    return 0;
}