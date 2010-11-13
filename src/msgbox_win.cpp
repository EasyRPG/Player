/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "msgbox.h"
#include "SDL_syswm.h"
#include <windows.h>
#include <Commctrl.h>

////////////////////////////////////////////////////////////
// Add Manifest depending on architecture
////////////////////////////////////////////////////////////
#ifdef _MSC_VER
	#if defined _M_IX86
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
	#elif defined _M_X64
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
	#else
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
	#endif
#endif

////////////////////////////////////////////////////////////
// Function prototype for TaskDialog.
// Without this we have to link against comctl32.lib and
// this makes the program incompatible with WinXP.
////////////////////////////////////////////////////////////
typedef HRESULT (WINAPI *TaskDialogFunc)(HWND, HINSTANCE, PCWSTR, PCWSTR, PCWSTR, TASKDIALOG_COMMON_BUTTON_FLAGS, PCWSTR, int);

////////////////////////////////////////////////////////////
/// MSVC Unicode std::string to LPCWSTR
////////////////////////////////////////////////////////////
#ifdef MSVC
static std::wstring s2ws(const std::string& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}
#else
static std::string s2ws(const std::string& s) {
	return s;
}
#endif

////////////////////////////////////////////////////////////
/// Detects the Windows version during runtime.
/// Vista (and later) have version 6 and higher.
////////////////////////////////////////////////////////////
int GetWindowsVersion() {
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);

	return osvi.dwMajorVersion;
}

////////////////////////////////////////////////////////////
/// Gets the HWND of the SDL-Window.
////////////////////////////////////////////////////////////
HWND GetHwnd() {
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);

	if(!SDL_GetWMInfo(&wmi)) {
		return NULL;
	}

	return wmi.window;
}

////////////////////////////////////////////////////////////
/// Displays the TaskDialog
////////////////////////////////////////////////////////////
void ShowTaskDialog(std::string msg, std::string title, PCWSTR icon) {
	TaskDialogFunc TheTaskDialogFunc;
	HINSTANCE hInstLibrary = LoadLibrary(L"comctl32.dll");
	TheTaskDialogFunc = (TaskDialogFunc)GetProcAddress(hInstLibrary, "TaskDialog");
	if (TheTaskDialogFunc != NULL) {
		TheTaskDialogFunc(GetHwnd(), NULL, s2ws(title).c_str(), s2ws(msg).c_str(), NULL, TDCBF_OK_BUTTON, icon, NULL);
	}
	FreeLibrary(hInstLibrary);
}

////////////////////////////////////////////////////////////
/// Default Message Box with OK button
////////////////////////////////////////////////////////////
void MsgBox::OK(std::string msg, std::string title) {
	if (GetWindowsVersion() >= 6) {
		ShowTaskDialog(msg, title, TD_INFORMATION_ICON);
	} else {
		MessageBox(GetHwnd(), s2ws(msg).c_str(), s2ws(title).c_str(), MB_OK);
	}
}

////////////////////////////////////////////////////////////
/// Error Message Box
////////////////////////////////////////////////////////////
void MsgBox::Error(std::string msg, std::string title) {
	if (GetWindowsVersion() >= 6) {
		ShowTaskDialog(msg, title, TD_ERROR_ICON);
	} else {
		MessageBox(GetHwnd(), s2ws(msg).c_str(), s2ws(title).c_str(), MB_OK | MB_ICONERROR);
	}
}

////////////////////////////////////////////////////////////
/// Warning Message Box
////////////////////////////////////////////////////////////
void MsgBox::Warning(std::string msg, std::string title) {
	if (GetWindowsVersion() >= 6) {
		ShowTaskDialog(msg, title, TD_WARNING_ICON);
	} else {
		MessageBox(GetHwnd(), s2ws(msg).c_str(), s2ws(title).c_str(), MB_OK | MB_ICONEXCLAMATION);
	}
}

#endif
