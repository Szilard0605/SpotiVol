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
	if (m_State != ConnectionState::Disconnected)
		return;

	m_ServerIP = ipAddress;
	m_ServerPort = port;
	m_UserName = name;

	m_State = ConnectionState::Connecting;

	if (!StartConnecting())
		return;
}

void SVClient::Disconnect()
{
	m_IsConnected = false;
	closesocket(m_Socket);
	m_Socket = INVALID_SOCKET;
	m_State = ConnectionState::Disconnected;
}

bool SVClient::StartConnecting()
{
	m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_Socket == INVALID_SOCKET)
	{
		printf("Invalid socket\n");
		return false;
	}

	u_long mode = 1;
	ioctlsocket(m_Socket, FIONBIO, &mode);

	SOCKADDR_IN addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_ServerPort);

	std::wstring wIp(m_ServerIP.begin(), m_ServerIP.end());

	InetPton(AF_INET, wIp.c_str(), &addr.sin_addr);

	int result = connect(m_Socket,
		reinterpret_cast<SOCKADDR*>(&addr),
		sizeof(addr));

	if (result == 0)
		return true;

	int err = WSAGetLastError();

	if (err == WSAEWOULDBLOCK)
		return true;

	closesocket(m_Socket);
	m_Socket = INVALID_SOCKET;
	return false;
}

void SVClient::UpdateConnecting()
{
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
		Disconnect();
		return;
	}
	
	if (selectResult == 0) // still connecting 
		return;

	if (FD_ISSET(m_Socket, &errFds))
	{
		Disconnect();
		return;
	}

	if (FD_ISSET(m_Socket, &writeFds))
	{
		int soError = 0;
		int len = sizeof(soError);

		getsockopt(m_Socket, SOL_SOCKET, SO_ERROR,
			reinterpret_cast<char*>(&soError), &len);

		if (soError != 0)
		{
			printf("Connect failed: %d\n", soError);
			closesocket(m_Socket);
			m_Socket = INVALID_SOCKET;
			return;
		}

		m_State = ConnectionState::Connected;

		Packet packet;
		packet.header.type = PacketIdentifier::ClientName;
		packet.header.dataSize = m_UserName.length() * sizeof(char);

		packet.data = std::vector<uint8_t>(
			reinterpret_cast<const uint8_t*>(m_UserName.c_str()),
			reinterpret_cast<const uint8_t*>(m_UserName.c_str()) + m_UserName.length()
		);

		SendPacketToServer(packet);

		printf("Connected to server!\n");
	}
}

void SVClient::UpdateConnected()
{

	char recvbuf[1024];
	int recvbuflen = 1024;
	int bytesReceived = recv(m_Socket, recvbuf, recvbuflen, 0);

	if (bytesReceived > 0)
	{
		m_DataBuffer.insert(m_DataBuffer.end(), recvbuf, recvbuf + bytesReceived);

		while (m_DataBuffer.size() >= sizeof(PacketHeader))
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
		Disconnect();
	}
}

void SVClient::Update()
{
	switch (m_State)
	{
	case ConnectionState::Disconnected:
	{
		break;
	}

	case ConnectionState::Connecting:
	{
		UpdateConnecting();
		break;
	}

	case ConnectionState::Connected:
	{
		UpdateConnected();
		break;
	}
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
