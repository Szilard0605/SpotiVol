#include "VolumeSetter.h"
#include "SVServer.h"

#include "PacketIdentifiers.h"

#include "TrayApplication.h"

#include <windows.h>
#include <iostream>
#include <cstdio>

SVServer server;

void OnClientConnect(ServerClientInfo& clientInfo)
{
	printf("Client %s connected: %s:%d (ID: %d)\n", clientInfo.name.c_str(), clientInfo.ipAddress.c_str(), clientInfo.port, clientInfo.id);

	float volumeLevel = VolumeSetter::GetAppVolume(L"Spotify.exe");

	Packet packet;
	packet.header.type = PacketIdentifier::VolumeChange;
	packet.header.dataSize = sizeof(float);
	packet.data = std::vector<uint8_t>(
		reinterpret_cast<const uint8_t*>(&volumeLevel),
		reinterpret_cast<const uint8_t*>(&volumeLevel) + sizeof(float)
	);

	server.SendPacketToAllClients(packet);

	printf("Sent initial volume level %.2f to client %s\n", volumeLevel, clientInfo.name.c_str());
}

void OnClientDisconnect(ServerClientInfo& clientInfo)
{
	printf("Client %s disconnected: %s:%d (ID: %d)\n", clientInfo.name.c_str(), clientInfo.ipAddress.c_str(), clientInfo.port, clientInfo.id);
}

void OnUIVolumeChange(ServerClientInfo& clientInfo, float volumeLevel)
{
	printf("Volume change request from %s (ID %d) client: %.2f\n", clientInfo.name.c_str(), clientInfo.id, volumeLevel);
	
	Packet packet;
	packet.header.type = PacketIdentifier::VolumeChange;
	packet.header.dataSize = sizeof(float);
	packet.data = std::vector<uint8_t>(
		reinterpret_cast<const uint8_t*>(&volumeLevel),
		reinterpret_cast<const uint8_t*>(&volumeLevel) + sizeof(float)
	);
	
	server.SendPacketToAllClients(packet);

	if(VolumeSetter::SetAppVolume(L"Spotify.exe", volumeLevel))
	{
		printf("Volume for client %s set to %.2f\n", clientInfo.name.c_str(), volumeLevel);
	}
	else
	{
		printf("Failed to set volume for client %s\n", clientInfo.name.c_str());
	}
}

#ifdef _DEBUG
int main()
{
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#endif
{	
	if (AllocConsole()) 
	{
		FILE* fpOut;
		freopen_s(&fpOut, "CONOUT$", "w", stdout);

		FILE* fpIn;
		freopen_s(&fpIn, "CONIN$", "r", stdin);

		FILE* fpErr;
		freopen_s(&fpErr, "CONOUT$", "w", stderr);

		std::ios::sync_with_stdio();
	}


	int port = 8000;
	printf("Starting server on port %d...\n", port);
	if (server.Start(port, VolumeSetter::GetAppVolume(L"Spotify.exe")))
	{
		HWND hConsole = GetConsoleWindow();
		TrayApplication trayApp(L"SpotiVolServer", hConsole);

		server.SetOnClientConnectCallback(OnClientConnect);
		server.SetOnClientDisconnectCallback(OnClientDisconnect);
		server.SetOnVolumeChangeCallback(OnUIVolumeChange);

		printf("Server started successfully!\n");
		while (true)
		{
			server.Update();
		}
	}
	return 0;
}