#pragma once

#include <windows.h>
#include <shellapi.h>
#include <string>
#include <thread>


#define WM_MYMESSAGE (WM_USER + 1)

class TrayApplication
{
public:
	TrayApplication(const std::wstring Application, HWND ConsoleHandle);
	~TrayApplication();


	HWND GetConsoleHandle() { return m_ConsoleHandle; }
private:
	HWND m_ConsoleHandle = NULL;
	HWND m_HiddenWindow = NULL;
	NOTIFYICONDATA m_IconData = { 0 };
	std::thread m_MessageLoopThread;
	bool m_Running = true;

	static LRESULT CALLBACK HiddenWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void MessageLoop();
};

