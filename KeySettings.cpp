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
	case TouhouKeymap::TH185:
		key("Shot", 'Z');
		key("Magic Circle", 'X');
		key("Focus", VK_SHIFT);
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
	case TouhouKeymap::TH16:
		key("Release", 'C');
		break;
	case TouhouKeymap::TH18:
	case TouhouKeymap::TH185:
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
KeySetting::KeySetting(const std::string& name, BYTE vKey1, BYTE vKey2)
{
	_name = name;

	_vKeyUnmapped = vKey1;
	// DirectInput keys are mapped after opening the keyboard device,
	// because we don't really know how DIK_* keys will map to scan codes
	// (and therefore to VK_* keys) until that happens and we can query the keyboard.
	_diKeyUnmapped = diKeys[0] = diKeys[1] = 0;

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

	return *this;
}

// ----------------------------------------------------------------------------
KeySetting& KeySetting::reset()
{
	vKeys[0] = defaults[0];
	vKeys[1] = defaults[1];

	return *this;
}

// ----------------------------------------------------------------------------
KeySetting& KeySetting::change(BYTE vKey)
{
	// don't use key codes that we can't resolve to a scan code
	if (!scanCode(vKey))
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

		UINT scanCode = KeySetting::scanCode(vKeys[i]);

		GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName));
		if (i > 0 && vKeys[i-1] > 0)
			name += " or ";
		name += keyName;
	}

	return name;
}

// ----------------------------------------------------------------------------
UINT KeySetting::scanCode(BYTE vKey)
{
	UINT scanCode = MapVirtualKeyA(vKey, MAPVK_VK_TO_VSC);
	switch (vKey)
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

	case VK_PAUSE:
		if (!scanCode)
			scanCode = 0x45;
		break;
	}

	return scanCode;
}

// ----------------------------------------------------------------------------
void KeySetting::updateDIKeys(const BYTE *scanCodeToDIKey)
{
	for (auto &key : registeredKeys)
	{
		key._diKeyUnmapped = scanCodeToDIKey[scanCode(key._vKeyUnmapped)];

		key.diKeys[0] = scanCodeToDIKey[scanCode(key.vKeys[0])];
		key.diKeys[1] = scanCodeToDIKey[scanCode(key.vKeys[1])];
	}
}
