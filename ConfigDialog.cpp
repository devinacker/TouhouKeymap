#include "pch.h"

#include "ConfigDialog.h"
#include "KeySettings.h"
#include "TouhouKeymap.h"
#include "resource.h"

#include <cstdio>

// from user32.dll
static auto chain_CreateDialogParamA = CreateDialogParamA;
static auto chain_CreateDialogParamW = CreateDialogParamW;
static auto chain_DialogBoxParamA = DialogBoxParamA;
static DLGPROC chain_DialogProc;
static WNDPROC chain_TextBoxWindowProc;

// ----------------------------------------------------------------------------
static BOOL CALLBACK UpdateEditForKey(HWND hwnd, LPARAM lParam)
{
	if (lParam == (LPARAM)GetWindowLongA(hwnd, GWL_USERDATA))
	{
		auto *gameKey = (KeySetting*)lParam;
		SetWindowTextA(hwnd, gameKey->keyBinding().c_str());

		return FALSE;
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
// Keystroke handler for our new dialog
static LRESULT CALLBACK TextBoxWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_GETDLGCODE)
	{
		return DLGC_WANTALLKEYS;
	}
	else if (uMsg == WM_KEYDOWN 
		&& wParam != 0
		&& !(lParam & (KF_UP | KF_REPEAT)))
	{
		auto *gameKey = (KeySetting*)GetWindowLongA(hwnd, GWL_USERDATA);

		if (gameKey)
		{
			// find an existing bind using this key and remove it if needed
			auto *otherKey = KeySettings::find(wParam);
			if (otherKey)
			{
				otherKey->change(wParam);
				EnumChildWindows(GetParent(hwnd), UpdateEditForKey, (LPARAM)otherKey);
			}
			if (gameKey != otherKey)
			{
				gameKey->change(wParam);
				SetWindowTextA(hwnd, gameKey->keyBinding().c_str());
			}
		}

		return 0;
	}
	else if (uMsg == WM_KEYUP || uMsg == WM_CHAR || uMsg == WM_UNICHAR)
	{
		return 0;
	}

	return CallWindowProcA(chain_TextBoxWindowProc, hwnd, uMsg, wParam, lParam);
}

// ----------------------------------------------------------------------------
// Enable/disable textboxes
static BOOL CALLBACK UpdateKeysEnabled(HWND hwnd, LPARAM lParam)
{
	if (GetWindowLongA(hwnd, GWL_WNDPROC) == (LONG)TextBoxWindowProc)
	{
		EnableWindow(hwnd, KeySettings::enabled);
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
// Restore default key bindings
static BOOL CALLBACK ResetDefaultKeys(HWND hwnd, LPARAM lParam)
{
	if (GetWindowLongA(hwnd, GWL_WNDPROC) == (LONG)TextBoxWindowProc)
	{
		auto *gameKey = (KeySetting*)GetWindowLongA(hwnd, GWL_USERDATA);
		gameKey->reset();
		SetWindowTextA(hwnd, gameKey->keyBinding().c_str());
	}

	return TRUE;
}

// ----------------------------------------------------------------------------
// Message handler for our new dialog
static INT_PTR CALLBACK ConfigDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		const unsigned marginDLU = 7;
		const unsigned rowGapDLU = 2; // 4;

		const unsigned labelHeightDLU = 8;
		const unsigned labelWidthDLU = 80; // test
		const unsigned textOffsetDLU = 3;

		const unsigned editHeightDLU = 14;
		const unsigned editWidthDLU = 100; // test

		const unsigned buttonHeightDLU = 14;
		const unsigned buttonWidthDLU = 50;

		const unsigned checkHeightDLU = 10;

		RECT clientRect, controlRect;

		unsigned topDLU = marginDLU;
		const unsigned rightDLU = 2 * marginDLU + labelWidthDLU + rowGapDLU + editWidthDLU;

		const auto dlgFont = (HFONT)SendMessageA(hwnd, WM_GETFONT, 0, 0);

		// create one label/edit pair for each registered ingame key
		HWND control;

		control = GetDlgItem(hwnd, IDC_CHECK1);
		controlRect.top = topDLU;
		controlRect.bottom = controlRect.top + checkHeightDLU;
		controlRect.left = marginDLU;
		controlRect.right = rightDLU - marginDLU;
		MapDialogRect(hwnd, &controlRect);
		MoveWindow(control, controlRect.left, controlRect.top, controlRect.right - controlRect.left, controlRect.bottom - controlRect.top, FALSE);
		SendMessageA(control, BM_SETCHECK, KeySettings::enabled ? BST_CHECKED : BST_UNCHECKED, 0);
		topDLU += checkHeightDLU + rowGapDLU;

		for (auto & gameKey : KeySettings::keys())
		{
			controlRect.top    = topDLU + textOffsetDLU;
			controlRect.bottom = controlRect.top + labelHeightDLU;
			controlRect.left   = marginDLU;
			controlRect.right  = controlRect.left + labelWidthDLU;
			MapDialogRect(hwnd, &controlRect);

			control = CreateWindowExA(0, "STATIC", gameKey.keyName().c_str(), WS_VISIBLE | WS_CHILD,
				controlRect.left, controlRect.top, controlRect.right - controlRect.left, controlRect.bottom - controlRect.top,
				hwnd, NULL, TouhouKeymap::hInstance, NULL);
			SendMessageA(control, WM_SETFONT, (WPARAM)dlgFont, 0);

			controlRect.top    = topDLU;
			controlRect.bottom = controlRect.top + editHeightDLU;
			controlRect.left   = marginDLU + labelWidthDLU + rowGapDLU;
			controlRect.right  = controlRect.left + editWidthDLU;
			MapDialogRect(hwnd, &controlRect);

			control = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", gameKey.keyBinding().c_str(), WS_VISIBLE | WS_CHILD | ES_CENTER,
				controlRect.left, controlRect.top, controlRect.right - controlRect.left, controlRect.bottom - controlRect.top,
				hwnd, NULL, TouhouKeymap::hInstance, NULL);
			SendMessageA(control, WM_SETFONT, (WPARAM)dlgFont, 0);

			chain_TextBoxWindowProc = (WNDPROC)SetWindowLongA(control, GWL_WNDPROC, (LONG)TextBoxWindowProc);
			SetWindowLongA(control, GWL_USERDATA, (LONG)&gameKey);

			topDLU += editHeightDLU + rowGapDLU;
		}

		topDLU += rowGapDLU;

		// move the buttons
		const int buttons[] = { IDOK, IDCANCEL, IDC_DEFAULTS, 0 };

		for (int i = 0; buttons[i]; i++)
		{
			control = GetDlgItem(hwnd, buttons[i]);
			controlRect.top = topDLU;
			controlRect.bottom = controlRect.top + buttonHeightDLU;
			controlRect.right = rightDLU - marginDLU - (i * (buttonWidthDLU + rowGapDLU));
			controlRect.left = controlRect.right - buttonWidthDLU;
			MapDialogRect(hwnd, &controlRect);
			MoveWindow(control, controlRect.left, controlRect.top, controlRect.right - controlRect.left, controlRect.bottom - controlRect.top, FALSE);
		}

		topDLU += buttonHeightDLU + marginDLU;

		// resize the dialog client area to hold the stuff
		controlRect.top = 0;
		controlRect.bottom = topDLU;
		controlRect.left = 0;
		controlRect.right = rightDLU;
		MapDialogRect(hwnd, &controlRect);

		WINDOWINFO wi;
		wi.cbSize = sizeof(wi);
		GetWindowInfo(hwnd, &wi);

		clientRect = wi.rcClient;
		clientRect.right = clientRect.left + controlRect.right;
		clientRect.bottom = clientRect.top + controlRect.bottom;

		AdjustWindowRect(&clientRect, wi.dwStyle, FALSE);
		MoveWindow(hwnd, clientRect.left, clientRect.top, 
			clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, TRUE);

		EnumChildWindows(hwnd, UpdateKeysEnabled, 0);
	}
	else if (uMsg == WM_COMMAND)
	{
		if (wParam == IDOK)
		{
			KeySettings::save();
			MessageBoxA(hwnd, "Key configuration saved.", "TouhouKeymap", MB_OK);

			EndDialog(hwnd, 1);
			return TRUE;
		}
		else if (wParam == IDCANCEL)
		{
			// reload previous settings and exit
			KeySettings::load();

			EndDialog(hwnd, 0);
			return TRUE;
		}
		else if (wParam == IDC_DEFAULTS)
		{
			EnumChildWindows(hwnd, ResetDefaultKeys, 0);
			return TRUE;
		}
		else if (wParam == IDC_CHECK1)
		{
			KeySettings::enabled = (SendMessageA((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
			EnumChildWindows(hwnd, UpdateKeysEnabled, 0);
			return TRUE;
		}
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
// Message handler hook for the existing dialog (and our new menu item)
static INT_PTR CALLBACK DialogProcHook(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HMENU addedMenuItem;

	if (uMsg == WM_INITDIALOG)
	{
		// get (or create) menu
		bool resizeForMenu = false;
		HMENU menu = GetMenu(hwnd);
		if (!menu)
		{
			menu = CreateMenu();
			SetMenu(hwnd, menu);
			resizeForMenu = true;
		}

		// add new item to menu
		addedMenuItem = CreateMenu();
		AppendMenuA(menu, MF_STRING, (UINT_PTR)addedMenuItem, "&Key Config...");

		if (resizeForMenu)
		{
			// resize the window to account for the menu
			WINDOWINFO wi;
			wi.cbSize = sizeof(wi);
			GetWindowInfo(hwnd, &wi);

			RECT area = wi.rcClient;
			AdjustWindowRect(&area, wi.dwStyle, TRUE);
			MoveWindow(hwnd, area.left, area.top,
				area.right - area.left, area.bottom - area.top,
				TRUE);
		}
	}
	else if (uMsg == WM_COMMAND && (HMENU)wParam == addedMenuItem)
	{
		chain_DialogBoxParamA(TouhouKeymap::hInstance, MAKEINTRESOURCEA(IDD_DIALOG1), hwnd, ConfigDialogProc, 0);

		return TRUE;
	}

	return chain_DialogProc(hwnd, uMsg, wParam, lParam);
}

// ----------------------------------------------------------------------------
static HWND WINAPI CreateDialogParamHookA(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
	chain_DialogProc = lpDialogFunc;
	return chain_CreateDialogParamA(hInstance, lpTemplateName, hWndParent, DialogProcHook, dwInitParam);
}

// ----------------------------------------------------------------------------
static HWND WINAPI CreateDialogParamHookW(HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
	chain_DialogProc = lpDialogFunc;
	return chain_CreateDialogParamW(hInstance, lpTemplateName, hWndParent, DialogProcHook, dwInitParam);
}

// ----------------------------------------------------------------------------
static INT_PTR WINAPI DialogBoxParamHook(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
	chain_DialogProc = lpDialogFunc;
	return chain_DialogBoxParamA(hInstance, lpTemplateName, hWndParent, DialogProcHook, dwInitParam);
}

// ----------------------------------------------------------------------------
void ConfigDialog::Init()
{
	detour_chain("user32.dll", 1,
		"CreateDialogParamA", CreateDialogParamHookA, &chain_CreateDialogParamA,
		"CreateDialogParamW", CreateDialogParamHookW, &chain_CreateDialogParamW,
		"DialogBoxParamA", DialogBoxParamHook, &chain_DialogBoxParamA,
		NULL);
}
