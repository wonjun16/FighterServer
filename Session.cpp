#include "Session.h"
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

Session::Session(void)
	: SendQ(100000), RecvQ(5000)
{
	Socket = -1;
	SessionId = 0;
	ValidCount = 5;
	IsAlive = true;
	Action = 0;
	Direction = 0;
	X = 0;
	Y = 0;
	IsMoving = false;
	LastAttackTime = 0;
	LastAttackType = 0;
	HP = 100;
}

Session::~Session(void)
{
	if (Socket != INVALID_SOCKET)
	{
		closesocket(Socket);
		Socket = INVALID_SOCKET;
	}
}
