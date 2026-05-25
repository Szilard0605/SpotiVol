#pragma once

#include <string>

class VolumeSetter
{
public:
	static bool SetAppVolume(const std::wstring& targetProcessName, float volumeLevel);
	static float GetAppVolume(const std::wstring& targetProcessName);
};

