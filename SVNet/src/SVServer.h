#pragma once

#include <cstdint>
#include <winsock2.h>
#include <vector>
#include <functional>
#include <string>
#include "Packet.h"

#define MAX_CLIENTS 5

struct ServerClientInfo
{
	int id = -1;
	std::string name;
	SOCKET socket;
	std::string ipAddress;
	u_short port;

	std::vector<uint8_t> dataBuffer;
};

class SVServer
{
public:
	SVServer();
	~SVServer();

	bool Start(uint16_t port, float initVolume);
	void Update();
	void Shutdown();

	bool SendPacketToClient(ServerClientInfo& client, Packet& packet);
	bool SendPacketToAllClients(Packet& packet);


	void SetOnClientConnectCallback(std::function<void(ServerClientInfo&)> callback) { m_OnClientConnectFn = callback; }
	void SetOnClientDisconnectCallback(std::function<void(ServerClientInfo&)> callback) { m_OnClientDisconnectFn = callback; }
	void SetOnVolumeChangeCallback(std::function<void(ServerClientInfo&, float)> callback) { m_OnVolumeChangeFn = callback; }

	std::vector<ServerClientInfo> GetConnectedClients() { return m_ConnectedClients; }
private:
	int m_ListenSocket;
	std::vector<ServerClientInfo> m_ConnectedClients;
	int m_ConnectedClientCount = 0;
	float m_Volume = 0.0f;
	float m_MutePrevVol = 0.0f;

	// callbacks
	std::function<void(ServerClientInfo&)> m_OnClientConnectFn;
	std::function<void(ServerClientInfo&)> m_OnClientDisconnectFn;
	std::function<void(ServerClientInfo&, float value)> m_OnVolumeChangeFn;
};

