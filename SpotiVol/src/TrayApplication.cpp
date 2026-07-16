#include "TrayApplication.h"

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

TrayApplication::TrayApplication(const std::wstring Title, HWND ConsoleHandle)
    : m_ConsoleHandle(ConsoleHandle)
{
    m_MessageLoopThread = std::thread(&TrayApplication::MessageLoop, this);

    while (m_HiddenWindow == NULL && m_Running) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (m_HiddenWindow != NULL) 
    {
        m_IconData.cbSize = sizeof(NOTIFYICONDATA);
        m_IconData.hWnd = m_HiddenWindow;
        m_IconData.uID = 100;
        m_IconData.uVersion = NOTIFYICON_VERSION;
        m_IconData.uCallbackMessage = WM_MYMESSAGE;
        m_IconData.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        _tcscpy_s(m_IconData.szTip, Title.c_str());
        m_IconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

        if (!Shell_NotifyIcon(NIM_ADD, &m_IconData)) 
        {
            printf("Failed to create tray icon\n");
        }
        else 
        {
            printf("Tray application interface loaded cleanly!\n");
        }
    }
}

TrayApplication::~TrayApplication()
{
    m_Running = false;
    if (m_HiddenWindow) 
    {
        Shell_NotifyIcon(NIM_DELETE, &m_IconData);
        PostMessage(m_HiddenWindow, WM_CLOSE, 0, 0);
    }
    if (m_MessageLoopThread.joinable()) 
    {
        m_MessageLoopThread.join();
    }
}
void TrayApplication::MessageLoop()
{
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = HiddenWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = _T("TrayAppHiddenWindowClass");

    RegisterClassEx(&wc);

    m_HiddenWindow = CreateWindowEx(0, wc.lpszClassName, _T(""), 0,
        0, 0, 0, 0, HWND_MESSAGE, NULL, wc.hInstance, this);

    if (!m_HiddenWindow) return;

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterClass(wc.lpszClassName, wc.hInstance);
}

LRESULT CALLBACK TrayApplication::HiddenWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_NCCREATE) 
    {
        LPCREATESTRUCT pCreate = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreate->lpCreateParams));
    }

    TrayApplication* pThis = reinterpret_cast<TrayApplication*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (!pThis && uMsg == WM_CREATE) 
    {
        LPCREATESTRUCT pCreate = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreate->lpCreateParams));
        pThis = reinterpret_cast<TrayApplication*>(pCreate->lpCreateParams);
    }

    switch (uMsg)
    {
        case WM_MYMESSAGE:
        {
            switch (lParam)
            {
            case WM_LBUTTONDBLCLK:
            {
                if (pThis == nullptr || pThis->GetConsoleHandle() == NULL || !IsWindow(pThis->GetConsoleHandle()))
                    break;

                if (IsWindowVisible(pThis->GetConsoleHandle()))
                {
                    ShowWindow(pThis->GetConsoleHandle(), SW_HIDE);
                }
                else
                {
                    ShowWindow(pThis->GetConsoleHandle(), SW_SHOW);
                    SetForegroundWindow(pThis->GetConsoleHandle());
                }
                break;
            }

            case WM_CLOSE:
                ShowWindow(pThis->GetConsoleHandle(), SW_HIDE);
                return 0;

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            }
        }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}