#include "Application.h"

#include "SVClient.h"
#include "UI.h"
#include "PacketIdentifiers.h"
#include "Logger.h"

#include <Lmcons.h>
#include <fstream>

static bool s_UIInit = false;

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
	m_Client.SetServerPingReceivedCallback([this]() { OnServerPing(); });

	Logger::Info("Attempting to connect to server at {}:{}", m_AppInfo.serverIPAddress.c_str(), m_AppInfo.serverPort);

	char username[UNLEN + 1];
	DWORD size = UNLEN + 1;
	std::string userName = "Unknown";
	if (GetUserNameA(username, &size))
		userName = username;

	auto lastConnectTry = std::chrono::steady_clock::now();
	const std::chrono::milliseconds connTryInterval(1000);

	auto lastCheck = std::chrono::steady_clock::now();
	const std::chrono::milliseconds checkInterval(20);


	auto lastPing = std::chrono::steady_clock::now();
	const std::chrono::milliseconds pingInterval(3000);

	while (!m_Window.ShouldClose())
	{
		m_Window.Update();

		if (m_Client.IsConnected())
		{
			auto now = std::chrono::steady_clock::now();

			if (now - m_LastServerPing >= std::chrono::milliseconds(6000))
			{
				Logger::Error("No response from server since 6 seconds, trying to reconnect...");
				m_Client.Disconnect();
				m_LastServerPing = now;
			}

			// mute shortcut
			if (now - lastCheck >= checkInterval)
			{
				lastCheck = now;

				bool isCtrlDown = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
				bool isMDown = (GetAsyncKeyState(0x4D) & 0x8000) != 0;

				if (isCtrlDown && isMDown && !m_IsKeyMDown)
				{
					m_Muted = !m_Muted;
					OnMuteRequest(m_Muted);
				}
				m_IsKeyMDown = isMDown;
			}

			// ping server
			if (now - lastPing >= pingInterval)
			{
				lastPing = now;

				Packet packet;
				packet.header.type = PacketIdentifier::Ping;
				packet.header.dataSize = 0;

				Logger::Info("Pinging server...");

				m_Client.SendPacketToServer(packet);
			}

			m_Client.Update();

			UI::BeginFrame(&m_Window);
			UI::RenderWindowOutline();
			UI::RenderConnected();
			UI::RenderClientList();
			UI::EndFrame();
		}
		else
		{
			m_Client.Update();
			auto now = std::chrono::steady_clock::now();

			if (now - lastConnectTry >= connTryInterval)
			{
				Logger::Info("Attempting connect....");
				lastConnectTry = now;
				m_Client.Connect(m_AppInfo.serverIPAddress, m_AppInfo.serverPort, userName);
			
			}

			m_LastServerPing = now;

			UI::BeginFrame(&m_Window);
			UI::RenderConnecting();
			UI::EndFrame();
		}
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

void Application::OnMuteRequest(bool mute)
{
	Packet packet;
	packet.header.dataSize = 0;
	if (mute)
	{
		packet.header.type = PacketIdentifier::Mute;
	}
	else
	{
		packet.header.type = PacketIdentifier::Unmute;
	}
	m_Client.SendPacketToServer(packet);
}

void Application::OnServerPing()
{
	Logger::Info("Received ping answer from server");
	m_LastServerPing = std::chrono::steady_clock::now();
}