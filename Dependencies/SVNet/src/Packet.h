#pragma once

#include <cstdint>
#include <vector>

struct PacketHeader 
{
	uint16_t type;
	size_t dataSize; 
};

struct Packet
{
	PacketHeader header;
	std::vector<uint8_t> data;
};