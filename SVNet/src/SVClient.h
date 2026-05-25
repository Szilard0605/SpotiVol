#pragma once

#include <cstdint>
//#include <winsock2.h>
#include <string>

#include "Packet.h"
#include <functional>

class SVClient
{
public:
	SVClient();
	~SVClient();
	void Connect(std::string ipAddress, uint16_t port, std::string name = "Unknown");
	void Update();
	bool SendPacketToServer(Packet& packet);
	void Disconnect();
	bool IsConnected() const { return m_IsConnected; }

	void SetOnVolumeChangeCallback(std::function<void(float)> callback) { m_OnVolumeChangeCallback = callback; }
private:
	unsigned __int64 m_Socket = ~0;
	bool m_IsConnected = false;

	std::function<void(float volume)> m_OnVolumeChangeCallback;

	std::vector<uint8_t> m_DataBuffer;
};

