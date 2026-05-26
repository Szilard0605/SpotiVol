#include "Application.h"

#include "SVClient.h"
#include "UI.h"
#include "PacketIdentifiers.h"
#include "Logger.h"

#include <Lmcons.h>
#include <fstream>

Application::Application(ApplicationInfo appInfo)
	: m_AppInfo(appInfo)
{
}

Application::~Application() { }

void Application::Run()
{
	Logger::Init();
	ReadConfigFile();

	m_Window.Initialize(m_AppInfo.name, m_AppInfo.width, m_AppInfo.height);

	UI::SetOnVolumeChangeCallback([this](float volumeLevel) { OnUIVolumeChange(volumeLevel); });
	m_Client.SetOnVolumeChangeCallback([this](float volumeLevel) { OnServerVolumeChange(volumeLevel); });

	Logger::Info("Attempting to connect to server at {}:{}\n", m_AppInfo.serverIPAddress.c_str(), m_AppInfo.serverPort);

	char username[UNLEN + 1];
	DWORD size = UNLEN + 1;
	if(GetUserNameA(username, &size))
		m_Client.Connect(m_AppInfo.serverIPAddress, 22506, username);
	else
		m_Client.Connect(m_AppInfo.serverIPAddress, 22506);

	while (!m_Client.IsConnected())
	{
		m_Window.Update();

		UI::BeginFrame(&m_Window);
		UI::RenderConnecting();
		UI::EndFrame();
	}

	while (!m_Window.ShouldClose())
	{
		m_Client.Update();
		m_Window.Update();

		UI::BeginFrame(&m_Window);
		UI::RenderWindowOutline();
		UI::RenderConnected();
		UI::EndFrame();
	}
}

void Application::ReadConfigFile()
{
	std::ifstream configFile(m_AppInfo.configFilePath);
	if (!configFile.is_open())
		return;

	std::string line;
	while (std::getline(configFile, line))
	{
		if (line.rfind("HostAddres=", 0) == 0)
		{
			m_AppInfo.serverIPAddress = line.substr(11);
			Logger::Info("Set server IP address to %s from config file\n", m_AppInfo.serverIPAddress.c_str());
		}
		else
		{
			Logger::Error("Unknown config entry in config file: {}\n", line.c_str());
		}
		
		if (line.rfind("HostPort=", 0) == 0)
		{

			m_AppInfo.serverPort = std::stoi(line.substr(9));
			Logger::Info("Set server IP address to %s from config file\n", m_AppInfo.serverIPAddress.c_str());
		}
		else
		{
			Logger::Error("Unknown config entry in config file: {}\n", line.c_str());
		
		}
	}
}

void Application::OnUIVolumeChange(float volumeLevel)
{
	int volume = static_cast<int>(std::floor(volumeLevel * 100));
	
	Packet packet;
	packet.header.type = PacketIdentifier::VolumeChange;
	packet.header.dataSize = sizeof(float);
	packet.data = std::vector<uint8_t>(
		reinterpret_cast<const uint8_t*>(&volumeLevel),
		reinterpret_cast<const uint8_t*>(&volumeLevel) + sizeof(float)
	);
	m_Client.SendPacketToServer(packet);
}

void Application::OnServerVolumeChange(float volumeLevel)
{
	UI::SetVolumeLevel(volumeLevel);
}
