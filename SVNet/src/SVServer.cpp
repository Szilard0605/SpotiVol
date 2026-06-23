#include "SVServer.h"

#define WIN32_LEAN_AND_MEAN
#include <print>
#include <windows.h>
#include <ws2tcpip.h>
#include <algorithm>

#include "PacketIdentifiers.h"

SVServer::SVServer()
{
	m_ConnectedClients.resize(MAX_CLIENTS); // Pre-allocate client slots

    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        m_ConnectedClients[i].id = -1; // Mark all client slots as available
	}

	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(result != 0) 
	{
		std::printf("WSAStartup failed with error: %d\n", result);
		return;
	}
}

SVServer::~SVServer()
{
	closesocket(m_ListenSocket);
	WSACleanup();
}

bool SVServer::Start(uint16_t port, float initVolume)
{ 
    m_Volume = initVolume;

	struct addrinfo hints;
	struct addrinfo* result = nullptr;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	std::string portStr = std::to_string(port);
	int addrInfoResult = getaddrinfo(NULL, portStr.c_str(), &hints, &result);
	if(addrInfoResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", addrInfoResult);
		WSACleanup();
		return false;
	}

	m_ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if(m_ListenSocket == INVALID_SOCKET) 
	{
		printf("socket failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}

	int bindResult = bind(m_ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if(bindResult == SOCKET_ERROR) 
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(m_ListenSocket);
		WSACleanup();
		return false;
	}

	freeaddrinfo(result);
	int listenResult = listen(m_ListenSocket, SOMAXCONN);
	if(listenResult == SOCKET_ERROR) 
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(m_ListenSocket);
		WSACleanup();
		return false;
	}
	return true;
}

void SVServer::Update()
{

    fd_set read_fds;
    FD_ZERO(&read_fds);

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    FD_SET(m_ListenSocket, &read_fds);
    SOCKET max_fd = m_ListenSocket;

    for (ServerClientInfo client : m_ConnectedClients)
    {
        if(client.id == -1)
			continue; 

        FD_SET(client.socket, &read_fds);
        if (client.socket > max_fd) {
            max_fd = client.socket;
        }
    }

    int selectResult = select(static_cast<int>(max_fd + 1), &read_fds, NULL, NULL, &timeout);

    if (selectResult == SOCKET_ERROR)
    {
        printf("select failed with error: %d\n", WSAGetLastError());
        return;
    }

    if (FD_ISSET(m_ListenSocket, &read_fds))
    {
        sockaddr_in clientAddr;
        socklen_t addrlen = sizeof(clientAddr);
        SOCKET newClientSocket = accept(m_ListenSocket, (sockaddr*)&clientAddr, &addrlen);

        if (newClientSocket != INVALID_SOCKET)
        {
            if(m_ConnectedClientCount >= MAX_CLIENTS)
            {
                printf("Max clients reached. Rejecting new connection.\n");
                closesocket(newClientSocket);
                return;
			}

            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            u_short clientPort = ntohs(clientAddr.sin_port);

			ServerClientInfo newClientInfo;
			newClientInfo.socket = newClientSocket;
			newClientInfo.ipAddress = clientIP;
			newClientInfo.port = clientPort;

            for (int i = 0; i < m_ConnectedClients.size(); i++)
            {
                if (m_ConnectedClients[i].id == -1)
                {
                    newClientInfo.id = i;
                    m_ConnectedClients[i] = newClientInfo;
                    m_ConnectedClientCount++;      
                    break;
                }
            }
        }
    }

    for (size_t i = 0; i < m_ConnectedClients.size();)
    {
        ServerClientInfo& clientInfo = m_ConnectedClients[i];
        bool clientDisconnected = false;

        if (FD_ISSET(clientInfo.socket, &read_fds))
        {
            char readBuffer[1024];
            int bytesReceived = recv(clientInfo.socket, readBuffer, sizeof(readBuffer), 0);

            if (bytesReceived <= 0)
            {
                if (bytesReceived == 0 && m_OnClientDisconnectFn) 
                    m_OnClientDisconnectFn(clientInfo);
            
				m_ConnectedClientCount--;

                closesocket(clientInfo.socket);
                
                for(int i = 0; i < m_ConnectedClients.size(); i++)
                {
                    if(m_ConnectedClients[i].id == clientInfo.id)
                    {
                        m_ConnectedClients[i].id = -1;
                        m_ConnectedClients[i].name.clear();
                        m_ConnectedClients[i].socket = INVALID_SOCKET;
                        m_ConnectedClients[i].ipAddress.clear();
                        m_ConnectedClients[i].port = 0;
                        m_ConnectedClients[i].dataBuffer.clear();
                        break;
                    }
				}
                clientDisconnected = true;

                return;
            }

            clientInfo.dataBuffer.insert(clientInfo.dataBuffer.end(), readBuffer, readBuffer + bytesReceived);

            while (clientInfo.dataBuffer.size() >= sizeof(PacketHeader))
            {
                PacketHeader header;
                std::memcpy(&header, clientInfo.dataBuffer.data(), sizeof(PacketHeader));

                if (clientInfo.dataBuffer.size() < sizeof(PacketHeader) + header.dataSize)
                    break;

                std::vector<uint8_t> payload(
                    clientInfo.dataBuffer.begin() + sizeof(PacketHeader),
                    clientInfo.dataBuffer.begin() + sizeof(PacketHeader) + header.dataSize
                );

                if (header.type == PacketIdentifier::ClientName)
                {
                    std::string name(reinterpret_cast<const char*>(payload.data()), header.dataSize);
                    clientInfo.name = name;
                    if (m_OnClientConnectFn) 
                        m_OnClientConnectFn(clientInfo);
                }
                else if (header.type == PacketIdentifier::VolumeChange)
                {
                    if (header.dataSize == sizeof(float)) 
                    {
                        float volumeLevel;
                        memcpy(&volumeLevel, payload.data(), sizeof(float));
                        printf("Volume change: %f", m_Volume);
                        m_Volume = volumeLevel;
                        if (m_OnVolumeChangeFn) 
                            m_OnVolumeChangeFn(clientInfo, volumeLevel);
                    }
                }
                else if (header.type == PacketIdentifier::Mute)
                {
                    m_MutePrevVol = m_Volume;
                    m_Volume = 0.0f;
                    printf("Got mute request, prev vol: %f\n", m_MutePrevVol);
                    if (m_OnVolumeChangeFn)
                        m_OnVolumeChangeFn(clientInfo, m_Volume);
                }
                else if (header.type == PacketIdentifier::Unmute)
                {
                    m_Volume = m_MutePrevVol;

                    printf("Got unmute request prev vol: %f\n", m_MutePrevVol);
                    if (m_OnVolumeChangeFn)
                        m_OnVolumeChangeFn(clientInfo, m_Volume);
                }

                clientInfo.dataBuffer.erase(
                    clientInfo.dataBuffer.begin(),
                    clientInfo.dataBuffer.begin() + sizeof(PacketHeader) + header.dataSize
                );
            }
        }

        if (!clientDisconnected) {
            i++;
        }
    }
}

void SVServer::Shutdown()
{
    for (ServerClientInfo& client : m_ConnectedClients)
    {
        closesocket(client.socket);
    }
    m_ConnectedClients.clear();
	closesocket(m_ListenSocket);
    WSACleanup();
}

bool SVServer::SendPacketToClient(ServerClientInfo& client, Packet& packet)
{
    PacketHeader header;
	header.type = packet.header.type;
	header.dataSize = static_cast<uint32_t>(packet.data.size());

	int sendResult = send(client.socket, reinterpret_cast<const char*>(&header), sizeof(header), 0);
    if(sendResult == SOCKET_ERROR)
    {
        printf("Failed to send packet header to client %d. Error: %d\n", client.id, WSAGetLastError());
        return false;
	}

    if (header.dataSize <= 0)
    {
		printf("No data to send for packet type %d\n", header.type);
        return false;
    }

    sendResult = send(client.socket, reinterpret_cast<const char*>(packet.data.data()), header.dataSize, 0);
    if(sendResult == SOCKET_ERROR)
    {
        printf("Failed to send packet data to client %d. Error: %d\n", client.id, WSAGetLastError());
        return false;
    }
	
    return true;
}

bool SVServer::SendPacketToAllClients(Packet& packet)
{
    for(ServerClientInfo& client : m_ConnectedClients)
    {
        if (client.id == -1)
            continue;

        if (!SendPacketToClient(client, packet))
        {
            printf("Failed to send packet to client %d\n", client.id);
            return false;
        }
	}
}
