#include "pch.h"

#include "KeySettings.h"
#include "TouhouKeymap.h"

char KeySettings::iniPath[MAX_PATH];
bool KeySettings::enabled;
std::vector<KeySetting> registeredKeys;

// ----------------------------------------------------------------------------
void KeySettings::registerKeys()
{
	using TouhouKeymap::game;

	GetModuleFileNameA(NULL, iniPath, sizeof(iniPath));
	char *pathEnd = strrchr(iniPath, '\\');
	if (pathEnd) *(pathEnd + 1) = '\0';
	strncat_s(iniPath, "TouhouKeymap.ini", MAX_PATH);

	// registeredKeys.clear();

	// shift and first/second buttons
	switch (game)
	{
	default:
		key("Shot", 'Z');
		key("Bomb", 'X');
		key("Focus", VK_SHIFT);
		break;
	case TouhouKeymap::TH095:
		key("Photo", 'Z');
		key("Focus", VK_SHIFT, 'X');
		break;
	case TouhouKeymap::TH125:
		key("Photo", 'Z');
		key("Focus", VK_SHIFT);
		key("Orientation", 'X');
		break;
	case TouhouKeymap::TH143:
		key("Shot", 'Z');
		key("Item", 'X');
		key("Focus", VK_SHIFT);
		break;
	case TouhouKeymap::TH165:
		key("Shot", 'Z');
		key("Photo", 'X');
		key("Focus", VK_SHIFT);
		break;
	}

	// additional buttons
	switch (game)
	{
	default:
		break;
	case TouhouKeymap::TH125:
		key("Switch Character", 'C');
		break;
	case TouhouKeymap::TH128:
	case TouhouKeymap::AlcoSTG:
		key("Rapid Fire", 'C');
		break;
	case TouhouKeymap::TH13:
		key("Trance", 'C');
		break;
	case TouhouKeymap::TH15:
		key("Release", 'C');
		break;
	case TouhouKeymap::TH18:
		key("Item", 'C');
		key("Switch Item", 'D');
		break;
	}

	// movement
	key("Up", VK_UP, VK_NUMPAD8);
	key("Down", VK_DOWN, VK_NUMPAD2);
	key("Left", VK_LEFT, VK_NUMPAD4);
	key("Right", VK_RIGHT, VK_NUMPAD6);
	key("Up + Left", VK_NUMPAD7);
	key("Up + Right", VK_NUMPAD9);
	key("Down + Left", VK_NUMPAD1);
	key("Down + Right", VK_NUMPAD3);

	// menu navigation, misc.
	key("Confirm", VK_RETURN);
	key("Menu", VK_ESCAPE);
	if (game < TouhouKeymap::TH18)
		key("Skip", VK_CONTROL);

	switch (game)
	{
	default:
		if (game != TouhouKeymap::TH18)
			key("Quit (when paused)", 'Q');
		if (game >= TouhouKeymap::TH07 && game != TouhouKeymap::TH09 && game != TouhouKeymap::TH17)
			key("Restart (when paused)", 'R');
		break;

	case TouhouKeymap::TH095:
	case TouhouKeymap::TH125:
		key("Lock Best Shot", 'L');
		key("Save Best Shot", 'S');
		break;
	}

	if (game != TouhouKeymap::TH06)
		key("Snapshot", VK_HOME, 'P');

	load();
}

// ----------------------------------------------------------------------------
const std::vector<KeySetting>& KeySettings::keys()
{
	return registeredKeys;
}

// ----------------------------------------------------------------------------
KeySetting& KeySettings::key(const std::string& name, BYTE vKey1, BYTE vKey2)
{
	KeySetting k(name, vKey1, vKey2);
	registeredKeys.push_back(k);
	return registeredKeys.back();
}

// ----------------------------------------------------------------------------
KeySetting* KeySettings::find(BYTE vKey)
{
	for (auto &key : registeredKeys)
	{
		if (key.vKey(0) == vKey || key.vKey(1) == vKey)
			return &key;
	}

	return NULL;
}

// ----------------------------------------------------------------------------
void KeySettings::load()
{
	enabled = GetPrivateProfileIntA("TouhouKeymap", "Enable Keymapper", 0, iniPath);

	for (auto &key : registeredKeys)
	{
		key.load();
	}
}

// ----------------------------------------------------------------------------
void KeySettings::save()
{
	WritePrivateProfileStringA("TouhouKeymap", "Enable Keymapper", enabled ? "1" : "0", iniPath);

	for (auto &key : registeredKeys)
	{
		key.save();
	}
}

// ----------------------------------------------------------------------------
void KeySettings::resetAll()
{
	for (auto &key : registeredKeys)
	{
		key.reset();
	}
}

// ----------------------------------------------------------------------------
BYTE KeySetting::vKeyToDIKey[256] = 
{
	0,0,0,0,0,0,0,0, // 0x00-07 unused
	DIK_BACK,
	DIK_TAB,
	0,0, // 0x0a-0b reserved
	0, // VK_CLEAR
	DIK_RETURN,
	0,0, // 0x0e-0f reserved
	DIK_LSHIFT,
	DIK_LCONTROL,
	DIK_LMENU,
	DIK_PAUSE,
	DIK_CAPITAL,
	0,0,0,0,0,0, // IME keys
	DIK_ESCAPE,
	0,0,0,0, // IME keys
	DIK_SPACE,
	DIK_PRIOR,
	DIK_NEXT,
	DIK_END,
	DIK_HOME,
	DIK_LEFT,
	DIK_UP,
	DIK_RIGHT,
	DIK_DOWN,
	0,0,0,0,
	DIK_INSERT,
	DIK_DELETE,
	0,
	DIK_0,
	DIK_1,
	DIK_2,
	DIK_3,
	DIK_4,
	DIK_5,
	DIK_6,
	DIK_7,
	DIK_8,
	DIK_9,
	0,0,0,0,0,0,0, // 0x3a-40 undefined
	DIK_A,
	DIK_B,
	DIK_C,
	DIK_D,
	DIK_E,
	DIK_F,
	DIK_G,
	DIK_H,
	DIK_I,
	DIK_J,
	DIK_K,
	DIK_L,
	DIK_M,
	DIK_N,
	DIK_O,
	DIK_P,
	DIK_Q,
	DIK_R,
	DIK_S,
	DIK_T,
	DIK_U,
	DIK_V,
	DIK_W,
	DIK_X,
	DIK_Y,
	DIK_Z,
	0,0,0,0,0,
	DIK_NUMPAD0,
	DIK_NUMPAD1,
	DIK_NUMPAD2,
	DIK_NUMPAD3,
	DIK_NUMPAD4,
	DIK_NUMPAD5,
	DIK_NUMPAD6,
	DIK_NUMPAD7,
	DIK_NUMPAD8,
	DIK_NUMPAD9,
	DIK_MULTIPLY,
	DIK_ADD,
	0, // VK_SEPARATOR
	DIK_SUBTRACT,
	DIK_DECIMAL,
	DIK_DIVIDE,
	DIK_F1,
	DIK_F2,
	DIK_F3,
	DIK_F4,
	DIK_F5,
	DIK_F6,
	DIK_F7,
	DIK_F8,
	DIK_F9,
	DIK_F10,
	DIK_F11,
	DIK_F12,
	DIK_F13,
	DIK_F14,
	DIK_F15,
	0,0,0,0,0,0,0,0,0, // F16 - F24
	0,0,0,0,0,0,0,0, // 0x88-8f unassigned
	DIK_NUMLOCK,
	DIK_SCROLL,
	0,0,0,0,0, // 0x92-96 OEM specific
	0,0,0,0,0,0,0,0,0, // 0x97-9f unassigned
	DIK_LSHIFT,
	DIK_RSHIFT,
	DIK_LCONTROL,
	DIK_RCONTROL,
	DIK_LMENU,
	DIK_RMENU,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0xa6-b7 browser / media keys
	0,0, // reserved
	DIK_SEMICOLON,
	DIK_NUMPADPLUS,
	DIK_COMMA,
	DIK_MINUS,
	DIK_PERIOD,
	DIK_SLASH,
	DIK_GRAVE,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 0xc1-d7 reserved
	0,0,0, // 0xd8-da unassigned
	DIK_LBRACKET,
	DIK_BACKSLASH,
	DIK_RBRACKET,
	DIK_APOSTROPHE,
	0,0,0, // 0xdf-e1 reserved/OEM
	DIK_OEM_102
};

// ----------------------------------------------------------------------------
KeySetting::KeySetting(const std::string& name, BYTE vKey1, BYTE vKey2)
{
	_name = name;

	_vKeyUnmapped = vKey1;
	_diKeyUnmapped = vKeyToDIKey[_vKeyUnmapped];

	defaults[0] = vKey1;
	defaults[1] = vKey2;

	set(vKey1, vKey2);
}

// ----------------------------------------------------------------------------
KeySetting& KeySetting::set(BYTE vKey1, BYTE vKey2)
{
	if (vKey1 == 0)
	{
		vKeys[0] = vKey2;
		vKeys[1] = 0;
	}
	else
	{
		vKeys[0] = vKey1;
		vKeys[1] = vKey2;
	}

	updateDIKeys();
	return *this;
}

// ----------------------------------------------------------------------------
KeySetting& KeySetting::reset()
{
	vKeys[0] = defaults[0];
	vKeys[1] = defaults[1];

	updateDIKeys();
	return *this;
}

// ----------------------------------------------------------------------------
KeySetting& KeySetting::change(BYTE vKey)
{
	// don't use key codes that we don't also have DirectInput equivalents for
	if (vKeyToDIKey[vKey] == 0)
		return *this;

	// passing an already bound key unbinds it
	if (vKey == vKeys[0])
	{
		vKeys[0] = vKeys[1];
		vKeys[1] = 0;
	}
	else if (vKey == vKeys[1])
	{
		vKeys[1] = 0;
	}
	// otherwise, bind it now
	else if (vKeys[0] == 0)
	{
		vKeys[0] = vKey;
	}
	else if (vKeys[1] == 0)
	{
		vKeys[1] = vKey;
	}
	// 2 keys already bound, get rid of the first one
	else
	{
		vKeys[0] = vKeys[1];
		vKeys[1] = vKey;
	}

	updateDIKeys();
	return *this;
}

// ----------------------------------------------------------------------------
KeySetting& KeySetting::load()
{
	char text[256];

	reset();
	if (GetPrivateProfileStringA("KeySettings", _name.c_str(), "", text, sizeof(text), KeySettings::iniPath) > 0)
	{
		unsigned newKey[2] = { 0 };
		sscanf_s(text, "%u, %u", &newKey[0], &newKey[1]);
		set(newKey[0], newKey[1]);
	}

	return *this;
}

// ----------------------------------------------------------------------------
void KeySetting::save() const
{
	char text[256];

	snprintf(text, sizeof(text), "%u, %u", vKeys[0], vKeys[1]);
	WritePrivateProfileStringA("KeySettings", _name.c_str(), text, KeySettings::iniPath);
}

// ----------------------------------------------------------------------------
std::string KeySetting::keyBinding() const
{
	std::string name;
	char keyName[256];

	for (int i = 0; i < 2; i++)
	{
		if (vKeys[i] == 0)
			continue;

		UINT scanCode = MapVirtualKeyA(vKeys[i], MAPVK_VK_TO_VSC);

		switch (vKeys[i])
		{
		case VK_UP: case VK_DOWN: case VK_LEFT: case VK_RIGHT:
		case VK_RCONTROL: case VK_RMENU:
		case VK_LWIN: case VK_RWIN:
		case VK_PRIOR: case VK_NEXT:
		case VK_HOME: case VK_END:
		case VK_INSERT: case VK_DELETE:
		case VK_NUMLOCK: case VK_DIVIDE:
			scanCode |= KF_EXTENDED;
			break;
		}

		GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName));
		if (i > 0 && vKeys[i-1] > 0)
			name += " or ";
		name += keyName;
	}

	return name;
}

// ----------------------------------------------------------------------------
void KeySetting::updateDIKeys()
{
	diKeys[0] = vKeyToDIKey[vKeys[0]];
	diKeys[1] = vKeyToDIKey[vKeys[1]];
}
