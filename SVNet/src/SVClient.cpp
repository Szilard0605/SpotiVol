#include "SVClient.h"

#include "PacketIdentifiers.h"

#define WIN32_LEAN_AND_MEAN
#include <print>
#include <windows.h>
#include <ws2tcpip.h>
SVClient::SVClient()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		//printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return;
	}

	m_Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_Socket == INVALID_SOCKET)
	{
		//printf("socket creation failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	u_long iMode = 1;
	if (ioctlsocket(m_Socket, FIONBIO, &iMode) == SOCKET_ERROR)
	{
		//printf("ioctlsocket failed\n");
	}

}

SVClient::~SVClient()
{
	if (m_Socket != INVALID_SOCKET)
	{
		closesocket(m_Socket);
	}
	WSACleanup();
}

void SVClient::Connect(std::string ipAddress, uint16_t port, std::string name)
{
	if (m_Socket == INVALID_SOCKET)
	{
		m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_Socket == INVALID_SOCKET) 
		{
			printf("Socket creation failed: %d\n", WSAGetLastError());
			return;
		}

		// If you are using non-blocking sockets (implied by WSAEWOULDBLOCK check):
		u_long mode = 1;
		ioctlsocket(m_Socket, FIONBIO, &mode);
	}

	SOCKADDR_IN addr;

	std::wstring wIpAddress(ipAddress.begin(), ipAddress.end());
	InetPton(AF_INET, wIpAddress.c_str(), &addr.sin_addr.s_addr);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	int connectResult = connect(m_Socket, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr));

	if (connectResult == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSAEWOULDBLOCK)
		{
			printf("Unable to connect to server: %d\n", err);
			closesocket(m_Socket);
			m_Socket = INVALID_SOCKET;
			return;
		}
	}

	fd_set writeFds, errFds;
	FD_ZERO(&writeFds);
	FD_ZERO(&errFds);
	FD_SET(m_Socket, &writeFds);
	FD_SET(m_Socket, &errFds);

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	int selectResult = select(0, nullptr, &writeFds, &errFds, &timeout);

	if (selectResult == SOCKET_ERROR)
	{
		//printf("Select error: %d\n", WSAGetLastError());
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return;
	}
	else if (selectResult == 0)
	{
		//printf("Timeout: Server is not responding.\n");
		//closesocket(m_Socket);
		//m_Socket = INVALID_SOCKET;
		return;
	}

	if (FD_ISSET(m_Socket, &errFds))
	{
		//printf("Connection error: The server may not be running.\n");
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		return;
	}

	if (FD_ISSET(m_Socket, &writeFds))
	{
		m_IsConnected = true;

		size_t len = name.length();
		Packet packet;
		packet.header.type = PacketIdentifier::ClientName;
		packet.header.dataSize = len * sizeof(char);

		packet.data = std::vector<uint8_t>(
			reinterpret_cast<const uint8_t*>(name.c_str()),
			reinterpret_cast<const uint8_t*>(name.c_str()) + len
		);

		SendPacketToServer(packet);
		return;
	}

	m_IsConnected = true;
}

void SVClient::Disconnect()
{
	m_IsConnected = false;
	closesocket(m_Socket);
	//WSACleanup();
}

void SVClient::Update()
{
	if (!m_IsConnected)
		return;

	char recvbuf[1024];
	int recvbuflen = 1024;
	int bytesReceived = recv(m_Socket, recvbuf, recvbuflen, 0);
	
	if (bytesReceived > 0)
	{
		m_DataBuffer.insert(m_DataBuffer.end(), recvbuf, recvbuf + bytesReceived);

		while(m_DataBuffer.size() >= sizeof(PacketHeader))
		{
			PacketHeader header;
			memcpy(&header, m_DataBuffer.data(), sizeof(PacketHeader));

			if (m_DataBuffer.size() < sizeof(PacketHeader) + header.dataSize)
				break;

			std::vector<uint8_t> payload(
				m_DataBuffer.begin() + sizeof(PacketHeader),
				m_DataBuffer.begin() + sizeof(PacketHeader) + header.dataSize
			);

			if (header.type == PacketIdentifier::VolumeChange)
			{
				//printf("Received volume change packet from server\n");

				if (header.dataSize == sizeof(float))
				{
					float volumeLevel;
					memcpy(&volumeLevel, payload.data(), sizeof(float));
					//printf("Volume level from server: %.2f\n", volumeLevel);
					if (m_OnVolumeChangeCallback)
						m_OnVolumeChangeCallback(volumeLevel);
				}
			}
			else if (header.type == PacketIdentifier::Ping)
			{
				if (m_ServerPingReceivedCallback)
					m_ServerPingReceivedCallback();
			}
			m_DataBuffer.erase(
				m_DataBuffer.begin(),
				m_DataBuffer.begin() + sizeof(PacketHeader) + header.dataSize
			);
		}

	}
	else if (bytesReceived == 0)
	{
		//printf("Connection closed by server.\n");
		Disconnect();
	}

}

bool SVClient::SendPacketToServer(Packet& packet)
{
	PacketHeader header;
	header.type = packet.header.type;
	header.dataSize = packet.header.dataSize;

	int sendResult = send(m_Socket, reinterpret_cast<const char*>(&header), sizeof(header), 0);
	if (sendResult == SOCKET_ERROR)
	{
		//printf("Failed to send packet header: %d\n", WSAGetLastError());
		Disconnect();
		return false;
	}

	sendResult = send(m_Socket, reinterpret_cast<const char*>(packet.data.data()), packet.data.size(), 0);
	if (sendResult == SOCKET_ERROR)
	{
		//printf("Failed to send packet data: %d\n", WSAGetLastError());
		Disconnect();
		return false;
	}
	return false;
}
