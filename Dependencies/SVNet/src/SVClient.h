#pragma once

#include <cstdint>
//#include <winsock2.h>
#include <string>

#include "Packet.h"
#include <functional>

class SVClient
{
public:
	enum class ConnectionState
	{
		Disconnected = 0,
		Connecting = 1,
		Connected = 2
	};

	SVClient();
	~SVClient();
	void Connect(std::string ipAddress, uint16_t port, std::string name = "Unknown");
	void Update();
	bool SendPacketToServer(Packet& packet);
	void Disconnect();
	bool IsConnected() const { return m_State == ConnectionState::Connected; }

	void SetOnVolumeChangeCallback(std::function<void(float)> callback) { m_OnVolumeChangeCallback = callback; }
	void SetServerPingReceivedCallback(std::function<void(void)> callback) { m_ServerPingReceivedCallback = callback; }
private:
	bool StartConnecting();
	void UpdateConnecting();
	void UpdateConnected();

	std::string m_ServerIP;
	uint16_t m_ServerPort;
	std::string m_UserName;

	ConnectionState m_State = ConnectionState::Disconnected;

	unsigned __int64 m_Socket = ~0;
	bool m_IsConnected = false;

	std::function<void(float volume)> m_OnVolumeChangeCallback;
	std::function<void(void)> m_ServerPingReceivedCallback;

	std::vector<uint8_t> m_DataBuffer;
};

