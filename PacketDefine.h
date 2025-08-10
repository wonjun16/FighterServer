#pragma once

#define	dfPACKET_SC_CREATE_MY_CHARACTER			0
#define	dfPACKET_SC_CREATE_OTHER_CHARACTER		1
#define	dfPACKET_SC_DELETE_CHARACTER			2

#define	dfPACKET_CS_MOVE_START					10
#define	dfPACKET_SC_MOVE_START					11
#define	dfPACKET_CS_MOVE_STOP					12
#define	dfPACKET_SC_MOVE_STOP					13

#define dfPACKET_MOVE_DIR_LL					0
#define dfPACKET_MOVE_DIR_LU					1
#define dfPACKET_MOVE_DIR_UU					2
#define dfPACKET_MOVE_DIR_RU					3
#define dfPACKET_MOVE_DIR_RR					4
#define dfPACKET_MOVE_DIR_RD					5
#define dfPACKET_MOVE_DIR_DD					6
#define dfPACKET_MOVE_DIR_LD					7

#define	dfPACKET_CS_ATTACK1						20
#define	dfPACKET_SC_ATTACK1						21
#define	dfPACKET_CS_ATTACK2						22
#define	dfPACKET_SC_ATTACK2						23
#define	dfPACKET_CS_ATTACK3						24
#define	dfPACKET_SC_ATTACK3						25

#define	dfPACKET_SC_DAMAGE						30

#pragma pack(push, 1)

struct HEADER
{
	unsigned char _code;
	unsigned char _size;
	unsigned char _type;
};

struct PACKET_SC_CREATE_MY_CHARACTER
{
	unsigned int _id;
	char _direction;
	short _x;
	short _y;
	char _hp;
};

struct PACKET_SC_CREATE_OTHER_CHARACTER
{
	unsigned int _id;
	char _direction;
	short _x;
	short _y;
	char _hp;
};

struct PACKET_SC_DELETE_CHARACTER
{
	unsigned int _id;
};

struct PACKET_CS_MOVE_START
{
	char _direction;
	short _x;
	short _y;
};

struct PACKET_SC_MOVE_START
{
	unsigned int _id;
	char _direction;
	short _x;
	short _y;
};

struct PACKET_CS_MOVE_STOP
{
	char _direction;
	short _x;
	short _y;
};

struct PACKET_SC_MOVE_STOP
{
	unsigned int _id;
	char _direction;
	short _x;
	short _y;
};

struct PACKET_CS_ATTACK1
{
	char _direction;
	short _x;
	short _y;
};

struct PACKET_SC_ATTACK1
{
	unsigned int _id;
	char _direction;
	short _x;
	short _y;
};

struct PACKET_CS_ATTACK2
{
	char _direction;
	short _x;
	short _y;
};

struct PACKET_SC_ATTACK2
{
	unsigned int _id;
	char _direction;
	short _x;
	short _y;
};

struct PACKET_CS_ATTACK3
{
	char _direction;
	short _x;
	short _y;
};

struct PACKET_SC_ATTACK3
{
	unsigned int _id;
	char _direction;
	short _x;
	short _y;
};

struct PACKET_SC_DAMAGE
{
	unsigned int _attackerId;
	unsigned int _defenderId;
	char _defenderLeftHP;
};

#pragma pack(pop)

#define dfERROR_RANGE		50

#define dfRANGE_MOVE_TOP	50
#define dfRANGE_MOVE_LEFT	10
#define dfRANGE_MOVE_RIGHT	630
#define dfRANGE_MOVE_BOTTOM	470

#define dfATTACK1_RANGE_X		80
#define dfATTACK2_RANGE_X		90
#define dfATTACK3_RANGE_X		100
#define dfATTACK1_RANGE_Y		10
#define dfATTACK2_RANGE_Y		10
#define dfATTACK3_RANGE_Y		20

#define dfATTACK1_DAMAGE		5
#define dfATTACK2_DAMAGE		8
#define dfATTACK3_DAMAGE		15

#define dfATTACK1_COOL_TIME		200
#define dfATTACK2_COOL_TIME		280
#define dfATTACK3_COOL_TIME		400

#define dfMAX_SESSION			40