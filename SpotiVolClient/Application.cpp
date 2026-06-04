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

Application::~Application() 
{
	m_Window.Destroy();
	m_Client.Disconnect();
}

void Application::Run()
{
	Logger::Init();
	ReadConfigFile();

	m_Window.Initialize(m_AppInfo.name, m_AppInfo.width, m_AppInfo.height);

	UI::SetOnVolumeChangeCallback([this](float volumeLevel) { OnUIVolumeChange(volumeLevel); });
	m_Client.SetOnVolumeChangeCallback([this](float volumeLevel) { OnServerVolumeChange(volumeLevel); });

	Logger::Info("Attempting to connect to server at {}:{}", m_AppInfo.serverIPAddress.c_str(), m_AppInfo.serverPort);

	char username[UNLEN + 1];
	DWORD size = UNLEN + 1;
	std::string userName = "Unknown";
	if (GetUserNameA(username, &size))
		userName = username;

	int connTry = 0;

	while (connTry < 500)
	{
		if (!m_Client.IsConnected())
		{
			m_Client.Connect(m_AppInfo.serverIPAddress, m_AppInfo.serverPort, userName);
		}
		else break;

		m_Window.Update();

		UI::BeginFrame(&m_Window);
		UI::RenderConnecting();
		UI::EndFrame();

		connTry++;
	}


	while (!m_Window.ShouldClose() && m_Client.IsConnected())
	{
		m_Client.Update();
		m_Window.Update();

		UI::BeginFrame(&m_Window);
		UI::RenderWindowOutline();
		UI::RenderConnected();
		UI::EndFrame();
	}

	m_Window.Destroy();
	m_Client.Disconnect();
}

void Application::ReadConfigFile()
{
	std::ifstream configFile(m_AppInfo.configFilePath);
	if (!configFile.is_open())
		return;

	std::string line;
	while (std::getline(configFile, line))
	{
		if (line.rfind("HostAddress=", 0) == 0)
		{
			m_AppInfo.serverIPAddress = line.substr(12);
			Logger::Info("Set server IP address to {} from config file", m_AppInfo.serverIPAddress.c_str());
			continue;
		}
		else
		{
			Logger::Error("Unknown config entry in config file: {}", line.c_str());
		}
		
		if (line.rfind("HostPort=", 0) == 0)
		{
			m_AppInfo.serverPort = std::stoi(line.substr(9));
			Logger::Info("Set server IP address to {} from config file", m_AppInfo.serverIPAddress.c_str());
			continue;
		}
		else
		{
			Logger::Error("Unknown config entry in config file: {}", line.c_str());
		
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
