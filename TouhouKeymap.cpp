#include "pch.h"

#include "TouhouKeymap.h"
#include "InputHooks.h"
#include "ConfigDialog.h"
#include "KeySettings.h"

// from thcrap.dll
int (*detour_chain)(const char*, int, ...);
void (*log_printf)(const char*, ...);
const char* (*runconfig_game_get)();

namespace TouhouKeymap
{
	HINSTANCE hInstance;
	Game game = Unknown;

	struct GameDef
	{
		const char *name;
		Game game;
	};

	GameDef gameDefs[] = 
	{
		{"th06",           TH06},
		{"th06_custom",    TH06},
		{"th07",           TH07},
		{"th07_custom",    TH07},
		{"th08",           TH08},
		{"th08_custom",    TH08},
		{"th09",           TH09},
		{"th09_custom",    TH09},
		{"th095",          TH095},
		{"th095_custom",   TH095},
		{"th10",           TH10},
		{"th10_custom",    TH10},
		{"alcostg",        AlcoSTG},
		{"alcostg_custom", AlcoSTG},
		{"th11",           TH11},
		{"th11_custom",    TH11},
		{"th12",           TH12},
		{"th12_custom",    TH12},
		{"th125",          TH125},
		{"th125_custom",   TH125},
		{"th128",          TH128},
		{"th128_custom",   TH128},
		{"th13",           TH13},
		{"th13_custom",    TH13},
		{"th14",           TH14},
		{"th14_custom",    TH14},
		{"th143",          TH143},
		{"th143_custom",   TH143},
		{"th15",           TH15},
		{"th15_custom",    TH15},
		{"th16",           TH16},
		{"th16_custom",    TH16},
		{"th165",          TH165},
		{"th165_custom",   TH165},
		{"th17",           TH17},
		{"th17_custom",    TH17},
		{"th18",           TH18},
		{"th18_custom",    TH18},
		{"th185",          TH185},
		{"th185_custom",   TH185},
		{"th19",           TH19},
		{"th19_custom",    TH19},

		{"th105",          Unsupported},
		{"th123",          Unsupported},
		{"th135",          Unsupported},
		{"th145",          Unsupported},
		{"th155",          Unsupported},
		{"th175",          Unsupported},
		{"megamari",       Unsupported},
		{"nsml",           Unsupported},
		{"marilega",       Unsupported},

		{0,                Unknown},
	};
}

// ----------------------------------------------------------------------------
TOUHOUKEYMAP_API int __stdcall thcrap_plugin_init()
{
	HMODULE thcrap = GetModuleHandleA("thcrap.dll");
	*(FARPROC*)&detour_chain       = GetProcAddress(thcrap, "detour_chain");
	*(FARPROC*)&log_printf         = GetProcAddress(thcrap, "log_printf");
	*(FARPROC*)&runconfig_game_get = GetProcAddress(thcrap, "runconfig_game_get");

	if (!detour_chain || !log_printf || !runconfig_game_get)
	{
		MessageBoxA(GetForegroundWindow(), "couldn't get thcrap functions", "TouhouKeymap", MB_OK);
		return 1;
	}

	const char *game = runconfig_game_get();
	if (!game)
	{
		// probably vpatch or something else
		return 1;
	}
	for (int i = 0; TouhouKeymap::gameDefs[i].name; i++)
	{
		if (!strcmp(game, TouhouKeymap::gameDefs[i].name))
		{
			TouhouKeymap::game = TouhouKeymap::gameDefs[i].game;
			break;
		}
	}
	if (TouhouKeymap::game == TouhouKeymap::Unsupported)
	{
		return 1;
	}

	KeySettings::registerKeys();

	return 0;
}

// ----------------------------------------------------------------------------
extern "C" TOUHOUKEYMAP_API void keymap_mod_detour(void)
{
	ConfigDialog::Init();
	InputHooks::Init();
}

// ----------------------------------------------------------------------------
BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		TouhouKeymap::hInstance = hModule;
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
