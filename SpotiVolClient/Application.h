#pragma once

#include <string>

#include "Window.h"
#include "SVClient.h"

struct ApplicationInfo
{
	std::string name;
	int width;
	int height;

	std::string configFilePath = "config.ini";
	std::string serverIPAddress = "127.0.0.1";
	uint16_t serverPort = 22506;
};;

class Application
{
public:
	Application(ApplicationInfo appInfo);
	~Application();
	void Run();
private:
	ApplicationInfo m_AppInfo;
	Window m_Window;
	SVClient m_Client;

	bool m_Muted = false;
	bool m_IsKeyMDown = false;

	void ReadConfigFile();

	void OnUIVolumeChange(float volumeLevel);
	void OnServerVolumeChange(float volumeLevel);
	void OnMuteRequest(bool mute);
};

