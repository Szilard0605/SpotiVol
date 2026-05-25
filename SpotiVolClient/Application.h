#pragma once

#include <string>

#include "Window.h"
#include "SVClient.h"

struct ApplicationInfo
{
	std::string name;
	int width;
	int height;
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

	void OnUIVolumeChange(float volumeLevel);
	void OnServerVolumeChange(float volumeLevel);
};

