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
static HRESULT WINAPI CreateDeviceHook(IDirectInput8A* self, REFGUID rguid, LPDIRECTINPUTDEVICEA *lplpDirectInputDevice, LPUNKNOWN punkOuter)
{
	HRESULT result = chain_CreateDevice(self, rguid, lplpDirectInputDevice, punkOuter);
	if (result == DI_OK && rguid == GUID_SysKeyboard)
	{
		auto iface = (void***)*lplpDirectInputDevice;
		auto vtable = *iface;

		*(void**)&chain_GetDeviceState = vtable[9];
		vtable[9] = GetDeviceStateHook;
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
