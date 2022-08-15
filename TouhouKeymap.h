#pragma once

#ifdef TOUHOUKEYMAP_EXPORTS
#define TOUHOUKEYMAP_API __declspec(dllexport)
#else
#define TOUHOUKEYMAP_API __declspec(dllimport)
#endif

namespace TouhouKeymap
{
	extern HINSTANCE hInstance;

	enum Game
	{
		TH06,
		TH07,
		TH08,
		TH09,
		TH095,
		TH10,
		AlcoSTG,
		TH11,
		TH12,
		TH125,
		TH128,
		TH13,
		TH14,
		TH143,
		TH15,
		TH16,
		TH165,
		TH17,
		TH18,
		TH185,

		Unknown,
		Unsupported
	};
	extern Game game;
}

// from thcrap.dll
extern int(*detour_chain)(const char*, int, ...);
extern void(*log_printf)(const char*, ...);
