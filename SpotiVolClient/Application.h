#pragma once

#include <string>

#include "Window.h"
#include "SVClient.h"

#include <chrono>

enum class AppState
{
	Connecting = 0,
	Connected = 1
};

struct ApplicationInfo
{
	std::string name;
	int width;
	int height;

	std::string configFilePath = "config.ini";
	std::string serverIPAddress = "127.0.0.1";
	uint16_t serverPort = 22506;

	AppState appState = AppState::Connecting;
};

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

	std::chrono::steady_clock::time_point m_lastServerPing;

	bool m_Muted = false;
	bool m_IsKeyMDown = false;

	void ReadConfigFile();

	void OnUIVolumeChange(float volumeLevel);
	void OnServerVolumeChange(float volumeLevel);
	void OnMuteRequest(bool mute);
	void OnServerPing();
};

