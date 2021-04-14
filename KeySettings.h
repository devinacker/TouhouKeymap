#pragma once

#include <string>
#include <vector>

class KeySetting
{
public:
	KeySetting(const std::string& name, BYTE vKey1, BYTE vKey2 = 0);
	
	KeySetting& set(BYTE vKey1, BYTE vKey2 = 0);
	KeySetting& reset();
	KeySetting& change(BYTE vKey);

	KeySetting& load();
	void save() const;

	BYTE vKeyUnmapped() const  { return _vKeyUnmapped; }
	BYTE diKeyUnmapped() const { return _diKeyUnmapped; }

	BYTE vKey(int index) const  { return vKeys[index]; }
	BYTE diKey(int index) const { return diKeys[index]; }

	const std::string& keyName() const { return _name; }
	std::string keyBinding() const;

	static UINT scanCode(BYTE vKey);
	static void updateDIKeys(const BYTE *scanCodeToDIKey);

private:
	std::string _name;

	BYTE _vKeyUnmapped, _diKeyUnmapped;
	BYTE defaults[2]; // VK_* key codes
	BYTE vKeys[2];    // VK_* key codes
	BYTE diKeys[2];   // DIK_* key codes
};

namespace KeySettings
{
	extern char iniPath[MAX_PATH];
	extern bool enabled;
	const std::vector<KeySetting>& keys();

	KeySetting& key(const std::string& name, BYTE vKey1, BYTE vKey2 = 0);
	KeySetting* find(BYTE vKey);

	void load();
	void save();
	void resetAll();
	void registerKeys();
}
