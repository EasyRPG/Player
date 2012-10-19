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
#include "utils.h"
#include "util_win.h"
#include <windows.h>
#ifdef _MSC_VER
#include <Commctrl.h>
#endif

// MinGW commctrl.h does not define this
#ifndef SHGFP_TYPE_CURRENT
#define SHGFP_TYPE_CURRENT 0
#endif

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
// Because of missing stuff in MinGw, TaskDialog is only
// used when compiled with MSVC.
////////////////////////////////////////////////////////////
#ifdef _MSC_VER
typedef HRESULT (WINAPI *TaskDialogIndirectFunc)(const TASKDIALOGCONFIG*, int*, int*, BOOL*);
#endif

////////////////////////////////////////////////////////////
/// Needed for MsgBox-Center-Hack
////////////////////////////////////////////////////////////
INT CBTMessageBox(HWND,LPSTR,LPSTR,UINT);
LRESULT CALLBACK CBTProc(INT, WPARAM, LPARAM);
HHOOK hhk;

////////////////////////////////////////////////////////////
/// Displays a centered message box.
////////////////////////////////////////////////////////////
INT CBTMessageBox(HWND hwnd, LPCTSTR lpText, LPCTSTR lpCaption,
				UINT uType) {
	hhk = SetWindowsHookEx(WH_CBT, &CBTProc, 0, GetCurrentThreadId());
	return MessageBox(hwnd, lpText, lpCaption, uType);
}

////////////////////////////////////////////////////////////
/// Hack to center the old msgbox.
////////////////////////////////////////////////////////////
LRESULT CALLBACK CBTProc(INT nCode, WPARAM wParam, LPARAM lParam) {
	HWND  hParentWnd, hChildWnd;    // msgbox is "child"
	RECT  rParent, rChild, rDesktop;
	POINT pCenter, pStart;
	INT   nWidth, nHeight;

	// notification that a window is about to be activated
	// window handle is wParam
	if (nCode == HCBT_ACTIVATE) {
		// set window handles
		hParentWnd = WindowsUtils::GetHwnd();
		hChildWnd  = (HWND)wParam;

		if ((hParentWnd != 0) && (hChildWnd != 0) &&
			(GetWindowRect(GetDesktopWindow(), &rDesktop) != 0) &&
			(GetWindowRect(hParentWnd, &rParent) != 0) &&
			(GetWindowRect(hChildWnd, &rChild) != 0)) {

			// calculate message box dimensions
			nWidth  = (rChild.right - rChild.left);
			nHeight = (rChild.bottom - rChild.top);

			// calculate parent window center point
			pCenter.x = rParent.left+((rParent.right - rParent.left) / 2);
			pCenter.y = rParent.top+((rParent.bottom - rParent.top) / 2);

			// calculate message box starting point
			pStart.x = (pCenter.x - (nWidth / 2));
			pStart.y = (pCenter.y - (nHeight / 2));

			// adjust if message box is off desktop
			if (pStart.x < 0) pStart.x = 0;
			if (pStart.y < 0) pStart.y = 0;
			if (pStart.x + nWidth > rDesktop.right) {
				pStart.x = rDesktop.right - nWidth;
			}
			if (pStart.y + nHeight > rDesktop.bottom) {
				pStart.y = rDesktop.bottom - nHeight;
			}

			// move message box
			MoveWindow(hChildWnd, pStart.x, pStart.y, nWidth, nHeight, FALSE);
		}
	// exit CBT hook
	UnhookWindowsHookEx(hhk);
	}
	// otherwise, continue with any possible chained hooks
	else {
		CallNextHookEx(hhk, nCode, wParam, lParam);
	}
	return 0;
}

////////////////////////////////////////////////////////////
/// Displays the TaskDialog
////////////////////////////////////////////////////////////
#ifdef _MSC_VER
void ShowTaskDialog(std::string& msg, std::string& title, LPCTSTR icon) {
	TaskDialogIndirectFunc TheTaskDialogIndirectFunc;
	HINSTANCE hInstLibrary = LoadLibrary(L"comctl32.dll");
	TheTaskDialogIndirectFunc = (TaskDialogIndirectFunc)GetProcAddress(hInstLibrary, "TaskDialogIndirect");

	if (TheTaskDialogIndirectFunc != NULL) {
		// The Text after the first \n is placed at the content area of the
		// TaskDialog
		size_t pos = msg.find_first_of('\n');
		std::string content = "";

		if (pos == msg.length() - 1) {
			msg.resize(pos);
		}
		else if (pos != std::string::npos) {
			content = msg.substr(pos + 1);
			msg = msg.substr(0, pos);
		}

		std::wstring windowTitle = Utils::DecodeUTF(title);
		std::wstring mainInstruction = Utils::DecodeUTF(msg);
		std::wstring wcontent = Utils::DecodeUTF(content);

		// Prepare the Dialog
		TASKDIALOGCONFIG config;
		memset(&config, '\0', sizeof(config));

		config.cbSize = sizeof(config);
		config.hwndParent = WindowsUtils::GetHwnd();
		config.dwFlags = TDF_POSITION_RELATIVE_TO_WINDOW;
		config.pszWindowTitle = windowTitle.c_str();
		config.pszMainIcon = icon;
		config.pszMainInstruction = mainInstruction.c_str();
		config.pszContent = wcontent.c_str();
		TheTaskDialogIndirectFunc(&config, NULL, NULL, NULL);
	}
	FreeLibrary(hInstLibrary);
}
#endif

////////////////////////////////////////////////////////////
/// Default Message Box with OK button
////////////////////////////////////////////////////////////
void MsgBox::OK(std::string const& msg, std::string const& title) {
#ifdef _MSC_VER
	if (WindowsUtils::GetWindowsVersion() >= 6) {
		ShowTaskDialog(msg, title, TD_INFORMATION_ICON);
	} else {
		CBTMessageBox(WindowsUtils::GetHwnd(), Utils::DecodeUTF(msg).c_str(), Utils::DecodeUTF(title).c_str(), MB_OK);
	}
#else
	CBTMessageBox(WindowsUtils::GetHwnd(), Utils::DecodeUTF(msg).c_str(), Utils::DecodeUTF(title).c_str(), MB_OK);
#endif
}

////////////////////////////////////////////////////////////
/// Error Message Box
////////////////////////////////////////////////////////////
void MsgBox::Error(std::string const& msg, std::string const& title) {
#ifdef _MSC_VER
	if (WindowsUtils::GetWindowsVersion() >= 6) {
		ShowTaskDialog(msg, title, TD_ERROR_ICON);
	} else {
		CBTMessageBox(WindowsUtils::GetHwnd(), Utils::DecodeUTF(msg).c_str(), Utils::DecodeUTF(title).c_str(), MB_OK | MB_ICONERROR);
	}
#else
	CBTMessageBox(WindowsUtils::GetHwnd(), Utils::DecodeUTF(msg).c_str(), Utils::DecodeUTF(title).c_str(), MB_OK | MB_ICONERROR);
#endif
}

////////////////////////////////////////////////////////////
/// Warning Message Box
////////////////////////////////////////////////////////////
void MsgBox::Warning(std::string const& msg, std::string const& title) {
#ifdef _MSC_VER
	if (WindowsUtils::GetWindowsVersion() >= 6) {
		ShowTaskDialog(msg, title, TD_WARNING_ICON);
	} else {
		CBTMessageBox(WindowsUtils::GetHwnd(), Utils::DecodeUTF(msg).c_str(), Utils::DecodeUTF(title).c_str(), MB_OK | MB_ICONEXCLAMATION);
	}
#else
	CBTMessageBox(WindowsUtils::GetHwnd(), Utils::DecodeUTF(msg).c_str(), Utils::DecodeUTF(title).c_str(), MB_OK | MB_ICONEXCLAMATION);
#endif
}

#endif
