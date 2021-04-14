#include "pch.h"

#include "InputHooks.h"
#include "KeySettings.h"
#include "TouhouKeymap.h"

// from user32.dll
static auto chain_GetKeyboardState = GetKeyboardState;

// from dinput8.dll
static auto chain_DirectInput8Create = DirectInput8Create;
static HRESULT(WINAPI *chain_CreateDevice)(IDirectInput8A*, REFGUID, LPDIRECTINPUTDEVICEA *, LPUNKNOWN);
static HRESULT(WINAPI *chain_GetDeviceState)(IDirectInputDevice8A*, DWORD, LPVOID);
static HRESULT(WINAPI *chain_SetDataFormat)(IDirectInputDevice8A*, LPCDIDATAFORMAT);

// ----------------------------------------------------------------------------
// Keyboard input hook (non-DirectInput version)
static BOOL WINAPI GetKeyboardStateHook(PBYTE lpKeyState)
{
	BYTE unmappedKeys[256];

	if (!KeySettings::enabled)
	{
		return chain_GetKeyboardState(lpKeyState);
	}
	
	if (chain_GetKeyboardState(unmappedKeys))
	{
		memset(lpKeyState, 0, 256);

		for (auto &key : KeySettings::keys())
		{
			lpKeyState[key.vKeyUnmapped()] |= (unmappedKeys[key.vKey(0)] | unmappedKeys[key.vKey(1)]);
		}

		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------------
// Keyboard input hook (DirectInput version)
static HRESULT WINAPI GetDeviceStateHook(IDirectInputDevice8A *self, DWORD cbData, LPVOID lpvData)
{
	BYTE unmappedKeys[256];
	
	if (!KeySettings::enabled)
	{
		return chain_GetDeviceState(self, cbData, lpvData);
	}

	HRESULT result = chain_GetDeviceState(self, cbData, unmappedKeys);
	if (result == DI_OK)
	{
		auto lpKeyState = (BYTE*)lpvData;
		memset(lpKeyState, 0, 256);

		// GetDeviceState will sometimes set the 0th byte when some keys (media keys?) are pressed,
		// but since we are using 0 as the "no key assigned" dummy value, clear it to avoid false keypresses
		unmappedKeys[0] = 0;

		for (auto &key : KeySettings::keys())
		{
			lpKeyState[key.diKeyUnmapped()] |= (unmappedKeys[key.diKey(0)] | unmappedKeys[key.diKey(1)]);
		}
	}

	return result;
}

// ----------------------------------------------------------------------------
// Map DirectInput keys to Windows virtual keys after opening the keyboard
static HRESULT WINAPI SetDataFormatHook(IDirectInputDevice8A* self, LPCDIDATAFORMAT lpdf)
{
	HRESULT result = chain_SetDataFormat(self, lpdf);
	if (result == DI_OK)
	{
		BYTE scanCodeToDIKey[512] = { 0 };

		DIPROPDWORD dip;
		dip.diph.dwSize = sizeof(dip);
		dip.diph.dwHeaderSize = sizeof(dip.diph);
		dip.diph.dwHow = DIPH_BYOFFSET;
		for (int i = 0; i < 256; i++)
		{
			dip.diph.dwObj = i;
			if (self->GetProperty(DIPROP_SCANCODE, &dip.diph) != DI_OK)
				continue;

			UINT scanCode = (BYTE)dip.dwData;

			if (scanCode == 0xe0)
				scanCode = (BYTE)(dip.dwData >> 8) | KF_EXTENDED;
			else if (scanCode == 0xe1)
				scanCode = (BYTE)(dip.dwData >> 16) | KF_EXTENDED; // ?

			scanCodeToDIKey[scanCode] = i;
		}

		KeySetting::updateDIKeys(scanCodeToDIKey);
	}

	return result;
}

// ----------------------------------------------------------------------------
static HRESULT WINAPI CreateDeviceHook(IDirectInput8A* self, REFGUID rguid, LPDIRECTINPUTDEVICEA *lplpDirectInputDevice, LPUNKNOWN punkOuter)
{
	HRESULT result = chain_CreateDevice(self, rguid, lplpDirectInputDevice, punkOuter);
	if (result == DI_OK && rguid == GUID_SysKeyboard)
	{
		auto iface = (void***)*lplpDirectInputDevice;
		auto vtable = *iface;

		*(void**)&chain_GetDeviceState = vtable[9];
		vtable[9] = GetDeviceStateHook;

		*(void**)&chain_SetDataFormat = vtable[11];
		vtable[11] = SetDataFormatHook;
	}

	return result;
}

// ----------------------------------------------------------------------------
static HRESULT WINAPI DirectInput8CreateHook(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter)
{
	HRESULT result = chain_DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
	if (result == DI_OK)
	{
		auto iface = (void***)*ppvOut;
		auto vtable = *iface;

		*(void**)&chain_CreateDevice = vtable[3];
		vtable[3] = CreateDeviceHook;
	}

	return result;
}

// ----------------------------------------------------------------------------
void InputHooks::Init()
{
	detour_chain("user32.dll", 1,
		"GetKeyboardState", GetKeyboardStateHook, &chain_GetKeyboardState,
		NULL);
	detour_chain("dinput8.dll", 1,
		"DirectInput8Create", DirectInput8CreateHook, &chain_DirectInput8Create,
		NULL);
}
