#pragma once

enum class EPacketType : uint8
{
	SCNewEntity = 1,
	SCPosition,
	SCRemoveEntity,
	SCResult,
	SCCheckConnection,
	SCModifyHp,

	CSJoinParty = 128,
	CSPosition,
	CSLeaveParty,
};

#pragma pack(push, 1)
struct PacketBase
{
	uint8 length;
	EPacketType packetType;
};

struct SCNewEntity : PacketBase
{
	uint16 id;
	float x, y, z;
	uint8 entity_type;
	char flag;
};

struct SCPosition : PacketBase
{
	uint16 id;
	float x, y, z;
	float v;
	float r;
	char flag;
};

struct SCRemoveEntity : PacketBase
{
	uint16 id;
	char flag;
};

struct SCResult : PacketBase
{
	char data;
};

struct SCCheckConnection : PacketBase
{
	char data;
};

struct CSJoinParty : PacketBase
{
	uint16 id;
};

struct CSPosition : PacketBase
{
	float x, y, z;
	float v;
	float r;
	char flag;
};

struct CSLeaveParty : PacketBase
{
	
};
#pragma pack(pop)