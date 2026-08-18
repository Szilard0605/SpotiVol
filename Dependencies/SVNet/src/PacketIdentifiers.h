#pragma once

#include <cstdint>

enum PacketIdentifier : uint8_t
{
	Undefined = 0,
	ClientName = 1,
	VolumeChange = 2,
	Mute = 3,
	Unmute = 4,
	IntroduceClient = 5,
	Ping = 6
};