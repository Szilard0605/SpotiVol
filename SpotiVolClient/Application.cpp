#include "Application.h"

#include "SVClient.h"
#include "UI.h"
#include "PacketIdentifiers.h"

#include <Lmcons.h>

Application::Application(ApplicationInfo appInfo)
	: m_AppInfo(appInfo)
{
}

Application::~Application() { }

void Application::Run()
{
	m_Window.Initialize(m_AppInfo.name, m_AppInfo.width, m_AppInfo.height);

	UI::SetOnVolumeChangeCallback([this](float volumeLevel) { OnUIVolumeChange(volumeLevel); });
	m_Client.SetOnVolumeChangeCallback([this](float volumeLevel) { OnServerVolumeChange(volumeLevel); });

	char username[UNLEN + 1];
	DWORD size = UNLEN + 1;
	if(GetUserNameA(username, &size))
		m_Client.Connect("192.168.0.177", 8080, username);
	else
		m_Client.Connect("192.168.0.177", 8080);

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
