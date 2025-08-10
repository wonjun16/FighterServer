#include "Network.h"
#include <WS2tcpip.h>
#include <iostream>
#include <random>
#include "PacketDefine.h"
#include "System.h"

Network::Network(void)
{
	PORT = 5000;
	ListenSock = INVALID_SOCKET;
	FD_ZERO(&Rset);
	FD_ZERO(&Wset);
	SessionIdCounter = 0;
	CurSessionCount = 0;
	SessionList = std::list<Session*>();
}

Network::~Network(void)
{
	if (ListenSock != INVALID_SOCKET)
	{
		closesocket(ListenSock);
		ListenSock = INVALID_SOCKET;
	}
	for (auto it = SessionList.begin(); it != SessionList.end(); ++it)
	{
		Session* session = *it;
		delete session;
	}
	SessionList.clear();
	WSACleanup();
}

bool Network::Initialize(void)
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa))
	{
		printf("WSAStartup() error : %d", WSAGetLastError());
		return false;
	}

	ListenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSock == INVALID_SOCKET)
	{
		printf("socket() error : %d", WSAGetLastError());
		return false;
	}

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(PORT);
	int retbind = bind(ListenSock, (SOCKADDR*)&addr, sizeof(addr));
	if (retbind == SOCKET_ERROR)
	{
		printf("bind() error : %d", WSAGetLastError());
		return false;
	}

	int retlisten = listen(ListenSock, SOMAXCONN);
	if (retlisten == SOCKET_ERROR)
	{
		printf("listen() error : %d", WSAGetLastError());
		return false;
	}
	return true;
}

void Network::NetIOProcess(void)
{
	FD_ZERO(&Rset);
	FD_ZERO(&Wset);
	FD_SET(ListenSock, &Rset);

	for (auto it = SessionList.begin(); it != SessionList.end(); ++it)
	{
		Session* session = *it;
		if (session->Socket != INVALID_SOCKET)
		{
			FD_SET(session->Socket, &Rset);
			if (session->SendQ.GetUseSize() > 0)
			{
				FD_SET(session->Socket, &Wset);
			}
		}
	}

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	int retval = select(0, &Rset, &Wset, NULL, &timeout);
	if (retval == SOCKET_ERROR)
	{
		printf("select() error : %d", WSAGetLastError());
		exit(1);
	}

	if (retval > 0)
	{
		if (FD_ISSET(ListenSock, &Rset))
		{
			--retval;
			AcceptProc();
		}

		for (auto it = SessionList.begin(); it != SessionList.end(); ++it)
		{
			if (retval <= 0) break;

			Session* session = *it;
			if (FD_ISSET(session->Socket, &Rset))
			{
				--retval;
				RecvProc(session);
			}
			if (FD_ISSET(session->Socket, &Wset))
			{
				--retval;
				SendProc(session);
			}
		}
	}

	for (auto it = SessionList.begin(); it != SessionList.end();)
	{
		Session* session = *it;
		if (!session->IsAlive)
		{
			delete session;
			it = SessionList.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void Network::AcceptProc(void)
{
	SOCKADDR_IN clientaddr;
	int caddrlen = sizeof(clientaddr);
	SOCKET clientsock = accept(ListenSock, (SOCKADDR*)&clientaddr, &caddrlen);
	if (clientsock == INVALID_SOCKET)
	{
		printf("accept() error : %d\n", WSAGetLastError());
		return;
	}

	if(CurSessionCount >= dfMAX_SESSION)
	{
		printf("Max session count reached. Cannot accept new connection.\n");
		closesocket(clientsock);
		return;
	}
	CurSessionCount++;

	Session* newSession = CreateSession(clientsock, &clientaddr);
	CreateMyCharacter(newSession);
	for (auto it = SessionList.begin(); it != SessionList.end(); ++it)
	{
		Session* player = *it;
		if (player->SessionId != newSession->SessionId)
		{
			CreateOtherCharacter(player, newSession);
			CreateOtherCharacter(newSession, player);
		}
	}
}

void Network::RecvProc(Session* s)
{
	int ret = recv(s->Socket, s->RecvQ.GetRearBufferPtr(), s->RecvQ.DirectEnqueueSize(), 0);

	if (ret == SOCKET_ERROR)
	{
		printf("recv() error : %d\n", WSAGetLastError());
		Disconnect(s);
		return;
	}
	else if (ret == 0)
	{
		Disconnect(s);
		return;
	}

	s->RecvQ.MoveRear(ret);

	while (1)
	{
		if (s->RecvQ.GetUseSize() < sizeof(HEADER)) break;

		HEADER* header = (HEADER*)s->RecvQ.GetFrontBufferPtr();
		if (s->RecvQ.GetUseSize() < header->_size + sizeof(HEADER)) break;

		s->RecvQ.MoveFront(sizeof(HEADER));

		PacketProc(s, header->_type);
	}
}

void Network::SendProc(Session* s)
{
	if (s->SendQ.GetUseSize() <= 0) return;
	int sendsize = send(s->Socket, s->SendQ.GetFrontBufferPtr(), s->SendQ.GetUseSize(), 0);

	if (sendsize != s->SendQ.GetUseSize()) Disconnect(s);
	else s->SendQ.MoveFront(sendsize);
}

void Network::Disconnect(Session* s)
{
	s->IsAlive = false;
	CurSessionCount--;

	for (auto it = SessionList.begin(); it != SessionList.end(); ++it)
	{
		Session* player = *it;
		if (player->SessionId != s->SessionId)
		{
			DeleteCharacter(player, s);
		}
	}
	printf("# Delete Player # ID : %d # Cur Session Num : %d\n", s->SessionId, CurSessionCount);
}

Session* Network::CreateSession(SOCKET sock, SOCKADDR_IN* addr)
{
	Session* session = new Session();
	session->Socket = sock;
	session->SessionId = SessionIdCounter++;
	session->X = rand() % 500 + 50;
	session->Y = rand() % 350 + 60;
	session->HP = 100;
	session->ValidCount = 5;

	srand(static_cast<unsigned int>(time(0)) + session->SessionId);

	SessionList.push_back(session);

	printf("# Create Player # ID : %d # X : %d # Y : %d # Cur Session Num : %d\n", session->SessionId, session->X, session->Y, CurSessionCount);

	return session;
}

void Network::PacketProc(Session* s, unsigned char type)
{
	switch (type)
	{
	case dfPACKET_CS_MOVE_START:
	{
		PACKET_CS_MOVE_START* packet = (PACKET_CS_MOVE_START*)s->RecvQ.GetFrontBufferPtr();
		s->RecvQ.MoveFront(sizeof(PACKET_CS_MOVE_START));

		if (abs(s->X - packet->_x) > dfERROR_RANGE || abs(s->Y - packet->_y) > dfERROR_RANGE
			|| packet->_x > dfRANGE_MOVE_RIGHT || packet->_x < dfRANGE_MOVE_LEFT || packet->_y > dfRANGE_MOVE_BOTTOM || packet->_y < dfRANGE_MOVE_TOP)
		{
			Disconnect(s);
			printf("Invalid move start packet from ID: %d, X: %d, Y: %d\n", s->SessionId, packet->_x, packet->_y);
			return;
		}

		printf("# Move Start # ID : %d # X : %d # Y : %d # Action : %d\n", s->SessionId, packet->_x, packet->_y, packet->_direction);

		s->Action = packet->_direction;
		s->X = packet->_x;
		s->Y = packet->_y;
		s->IsMoving = true;

		switch (packet->_direction)
		{
		case dfPACKET_MOVE_DIR_RR:
		case dfPACKET_MOVE_DIR_RU:
		case dfPACKET_MOVE_DIR_RD:
			s->Direction = dfPACKET_MOVE_DIR_RR;
			break;
		case dfPACKET_MOVE_DIR_LL:
		case dfPACKET_MOVE_DIR_LU:
		case dfPACKET_MOVE_DIR_LD:
			s->Direction = dfPACKET_MOVE_DIR_LL;
			break;
		default:
			break;
		}

		for (auto it = SessionList.begin(); it != SessionList.end(); ++it)
		{
			Session* player = *it;
			if (player->SessionId != s->SessionId)
			{
				MoveStart(player, s);
			}
		}
	}
	break;

	case dfPACKET_CS_MOVE_STOP:
	{
		PACKET_CS_MOVE_STOP* packet = (PACKET_CS_MOVE_STOP*)s->RecvQ.GetFrontBufferPtr();
		s->RecvQ.MoveFront(sizeof(PACKET_CS_MOVE_STOP));

		if (abs(s->X - packet->_x) > dfERROR_RANGE || abs(s->Y - packet->_y) > dfERROR_RANGE
			|| packet->_x > dfRANGE_MOVE_RIGHT || packet->_x < dfRANGE_MOVE_LEFT || packet->_y > dfRANGE_MOVE_BOTTOM || packet->_y < dfRANGE_MOVE_TOP)
		{
			Disconnect(s);
			printf("Invalid move stop packet from ID: %d, X: %d, Y: %d\n", s->SessionId, packet->_x, packet->_y);
			return;
		}

		printf("# Move Stop # ID : %d # X : %d # Y : %d # Action : %d\n", s->SessionId, packet->_x, packet->_y, packet->_direction);

		s->Action = packet->_direction;
		s->IsMoving = false;
		s->X = packet->_x;
		s->Y = packet->_y;

		for (auto it = SessionList.begin(); it != SessionList.end(); ++it)
		{
			Session* player = *it;
			if (player->SessionId != s->SessionId)
			{
				MoveStop(player, s);
			}
		}
	}
	break;

	case dfPACKET_CS_ATTACK1:
	{
		PACKET_CS_ATTACK1* packet = (PACKET_CS_ATTACK1*)s->RecvQ.GetFrontBufferPtr();
		s->RecvQ.MoveFront(sizeof(PACKET_CS_ATTACK1));

		if (!CheckAttackValid(s, s->LastAttackType))
		{
			printf("Attack1 packet from ID: %d is invalid due to cooldown.\n", s->SessionId);
			
			s->ValidCount--;

			if(s->ValidCount <= 0)
			{
				printf("Session %d has been disconnected due to too many invalid packets.\n", s->SessionId);
				Disconnect(s);
			}

			return;
		}
		s->LastAttackTime = System::GetInstance()->GetLastUpdateTime();
		s->LastAttackType = 1;

		printf("# Attack1 # ID : %d # X : %d # Y : %d # Action : %d\n", s->SessionId, packet->_x, packet->_y, packet->_direction);

		for(auto it = SessionList.begin(); it != SessionList.end(); ++it)
		{
			Session* player = *it;
			if (player->SessionId != s->SessionId)
			{
				Attack1(player, s);
			}
		}
		
		CheckDemaged(s, 1);
	}
	break;

	case dfPACKET_CS_ATTACK2:
	{
		PACKET_CS_ATTACK2* packet = (PACKET_CS_ATTACK2*)s->RecvQ.GetFrontBufferPtr();
		s->RecvQ.MoveFront(sizeof(PACKET_CS_ATTACK2));

		if (!CheckAttackValid(s, s->LastAttackType))
		{
			printf("Attack1 packet from ID: %d is invalid due to cooldown.\n", s->SessionId);

			s->ValidCount--;

			if (s->ValidCount <= 0)
			{
				printf("Session %d has been disconnected due to too many invalid packets.\n", s->SessionId);
				Disconnect(s);
			}

			return;
		}
		s->LastAttackTime = System::GetInstance()->GetLastUpdateTime();
		s->LastAttackType = 2;

		printf("# Attack2 # ID : %d # X : %d # Y : %d # Action : %d\n", s->SessionId, packet->_x, packet->_y, packet->_direction);

		for (auto it = SessionList.begin(); it != SessionList.end(); ++it)
		{
			Session* player = *it;
			if (player->SessionId != s->SessionId)
			{
				Attack2(player, s);
			}
		}

		CheckDemaged(s, 2);
	}
	break;

	case dfPACKET_CS_ATTACK3:
	{
		PACKET_CS_ATTACK3* packet = (PACKET_CS_ATTACK3*)s->RecvQ.GetFrontBufferPtr();
		s->RecvQ.MoveFront(sizeof(PACKET_CS_ATTACK3));

		if (!CheckAttackValid(s, s->LastAttackType))
		{
			printf("Attack1 packet from ID: %d is invalid due to cooldown.\n", s->SessionId);

			s->ValidCount--;

			if (s->ValidCount <= 0)
			{
				printf("Session %d has been disconnected due to too many invalid packets.\n", s->SessionId);
				Disconnect(s);
			}

			return;
		}
		s->LastAttackTime = System::GetInstance()->GetLastUpdateTime();
		s->LastAttackType = 3;

		printf("# Attack3 # ID : %d # X : %d # Y : %d # Action : %d\n", s->SessionId, packet->_x, packet->_y, packet->_direction);

		for (auto it = SessionList.begin(); it != SessionList.end(); ++it)
		{
			Session* player = *it;
			if (player->SessionId != s->SessionId)
			{
				Attack3(player, s);
			}
		}

		CheckDemaged(s, 3);
	}
	break;

	default:
	{
		printf("Unknown packet type: %d\n", type);
		return;
	}
	}
}

void Network::CreateMyCharacter(Session* s)
{
	HEADER header;
	header._code = 0x89;
	header._size = sizeof(PACKET_SC_CREATE_MY_CHARACTER);
	header._type = dfPACKET_SC_CREATE_MY_CHARACTER;

	PACKET_SC_CREATE_MY_CHARACTER packet;
	packet._id = s->SessionId;
	packet._direction = s->Direction;
	packet._x = s->X;
	packet._y = s->Y;
	packet._hp = s->HP;

	int ret = s->SendQ.Enqueue((char*)&header, sizeof(header));
	if (ret == 0) Disconnect(s);
	ret = s->SendQ.Enqueue((char*)&packet, sizeof(packet));
	if (ret == 0) Disconnect(s);
}

void Network::CreateOtherCharacter(Session* mine, Session* other)
{
	HEADER header;
	header._code = 0x89;
	header._size = sizeof(PACKET_SC_CREATE_OTHER_CHARACTER);
	header._type = dfPACKET_SC_CREATE_OTHER_CHARACTER;

	PACKET_SC_CREATE_OTHER_CHARACTER packet;
	packet._id = other->SessionId;
	packet._direction = other->Direction;
	packet._x = other->X;
	packet._y = other->Y;
	packet._hp = other->HP;

	int ret = mine->SendQ.Enqueue((char*)&header, sizeof(header));
	if (ret == 0) Disconnect(mine);
	ret = mine->SendQ.Enqueue((char*)&packet, sizeof(packet));
	if (ret == 0) Disconnect(mine);
}

void Network::DeleteCharacter(Session* s, Session* target)
{
	HEADER header;
	header._code = 0x89;
	header._size = sizeof(PACKET_SC_DELETE_CHARACTER);
	header._type = dfPACKET_SC_DELETE_CHARACTER;

	PACKET_SC_DELETE_CHARACTER packet;
	packet._id = target->SessionId;
	int ret = s->SendQ.Enqueue((char*)&header, sizeof(header));
	if (ret == 0) Disconnect(s);
	ret = s->SendQ.Enqueue((char*)&packet, sizeof(packet));
	if (ret == 0) Disconnect(s);
}

void Network::MoveStart(Session* s, Session* target)
{
	HEADER header;
	header._code = 0x89;
	header._size = sizeof(PACKET_SC_MOVE_START);
	header._type = dfPACKET_SC_MOVE_START;

	PACKET_SC_MOVE_START packet;
	packet._id = target->SessionId;
	packet._direction = target->Action;
	packet._x = target->X;
	packet._y = target->Y;

	int ret = s->SendQ.Enqueue((char*)&header, sizeof(header));
	if (ret == 0) Disconnect(s);
	ret = s->SendQ.Enqueue((char*)&packet, sizeof(packet));
	if (ret == 0) Disconnect(s);
}

void Network::MoveStop(Session* s, Session* target)
{
	HEADER header;
	header._code = 0x89;
	header._size = sizeof(PACKET_SC_MOVE_STOP);
	header._type = dfPACKET_SC_MOVE_STOP;

	PACKET_SC_MOVE_STOP packet;
	packet._id = target->SessionId;
	packet._direction = target->Direction;
	packet._x = target->X;
	packet._y = target->Y;

	int ret = s->SendQ.Enqueue((char*)&header, sizeof(header));
	if (ret == 0) Disconnect(s);
	ret = s->SendQ.Enqueue((char*)&packet, sizeof(packet));
	if (ret == 0) Disconnect(s);
}

void Network::Attack1(Session* s, Session* target)
{
	HEADER header;
	header._code = 0x89;
	header._size = sizeof(PACKET_SC_ATTACK1);
	header._type = dfPACKET_SC_ATTACK1;

	PACKET_SC_ATTACK1 packet;
	packet._id = target->SessionId;
	packet._direction = target->Direction;
	packet._x = target->X;
	packet._y = target->Y;

	int ret = s->SendQ.Enqueue((char*)&header, sizeof(header));
	if (ret == 0) Disconnect(s);
	ret = s->SendQ.Enqueue((char*)&packet, sizeof(packet));
	if (ret == 0) Disconnect(s);
}

void Network::Attack2(Session* s, Session* target)
{
	HEADER header;
	header._code = 0x89;
	header._size = sizeof(PACKET_SC_ATTACK2);
	header._type = dfPACKET_SC_ATTACK2;

	PACKET_SC_ATTACK2 packet;
	packet._id = target->SessionId;
	packet._direction = target->Direction;
	packet._x = target->X;
	packet._y = target->Y;

	int ret = s->SendQ.Enqueue((char*)&header, sizeof(header));
	if (ret == 0) Disconnect(s);
	ret = s->SendQ.Enqueue((char*)&packet, sizeof(packet));
	if (ret == 0) Disconnect(s);
}

void Network::Attack3(Session* s, Session* target)
{
	HEADER header;
	header._code = 0x89;
	header._size = sizeof(PACKET_SC_ATTACK3);
	header._type = dfPACKET_SC_ATTACK3;

	PACKET_SC_ATTACK3 packet;
	packet._id = target->SessionId;
	packet._direction = target->Direction;
	packet._x = target->X;
	packet._y = target->Y;

	int ret = s->SendQ.Enqueue((char*)&header, sizeof(header));
	if (ret == 0) Disconnect(s);
	ret = s->SendQ.Enqueue((char*)&packet, sizeof(packet));
	if (ret == 0) Disconnect(s);
}

void Network::GetDamage(Session* s, Session* target)
{
	HEADER header;
	header._code = 0x89;
	header._size = sizeof(PACKET_SC_DAMAGE);
	header._type = dfPACKET_SC_DAMAGE;

	PACKET_SC_DAMAGE packet;
	packet._attackerId = s->SessionId;
	packet._defenderId = target->SessionId;
	packet._defenderLeftHP = target->HP;

	for(auto it = SessionList.begin(); it != SessionList.end(); ++it)
	{
		Session* player = *it;

		int ret = player->SendQ.Enqueue((char*)&header, sizeof(header));
		if (ret == 0) Disconnect(player);
		ret = player->SendQ.Enqueue((char*)&packet, sizeof(packet));
		if (ret == 0) Disconnect(player);
	}
}

bool Network::CheckAttackValid(Session* s, unsigned char attackType)
{
	bool ret = true;

	switch (attackType)
	{
	case 1:
		if (System::GetInstance()->GetLastUpdateTime() - s->LastAttackTime < dfATTACK1_COOL_TIME)
			ret = false;
		break;
	case 2:
		if (System::GetInstance()->GetLastUpdateTime() - s->LastAttackTime < dfATTACK2_COOL_TIME)
			ret = false;
		break;
	case 3:
		if (System::GetInstance()->GetLastUpdateTime() - s->LastAttackTime < dfATTACK3_COOL_TIME)
			ret = false;
		break;
	default:
		break;
	}

	return ret;
}

void Network::CheckDemaged(Session* atk, unsigned char attackType)
{
	int attackRangeX = 0;
	int attackRangeY = 0;
	int demage = 0;
	unsigned char attackDir = atk->Direction;

	switch (attackType)
	{
	case 1:
		attackRangeX = dfATTACK1_RANGE_X;
		attackRangeY = dfATTACK1_RANGE_Y;
		demage = dfATTACK1_DAMAGE;
		break;
	case 2:
		attackRangeX = dfATTACK2_RANGE_X;
		attackRangeY = dfATTACK2_RANGE_Y;
		demage = dfATTACK2_DAMAGE;
		break;
	case 3:
		attackRangeX = dfATTACK3_RANGE_X;
		attackRangeY = dfATTACK3_RANGE_Y;
		demage = dfATTACK3_DAMAGE;
		break;
	default:
		return;
	}

	for (auto it = SessionList.begin(); it != SessionList.end(); ++it)
	{
		Session* target = *it;
		if (target->SessionId == atk->SessionId) continue;
		if (IsInRange(atk, target, attackRangeX, attackRangeY, attackDir))
		{
			target->HP -= demage;

			GetDamage(atk, target);

			if(target->HP <= 0)
			{
				printf("# Player %d is dead. HP: %d\n", target->SessionId, target->HP);
				Disconnect(target);
			}
			else
			{
				printf("# Player %d took damage from %d. Remaining HP: %d\n", target->SessionId, atk->SessionId, target->HP);
			}
		}
	}
}

bool Network::IsInRange(Session* atk, Session* def, int rangeX, int rangeY, unsigned char dir)
{
	if (dir == dfPACKET_MOVE_DIR_LL)
	{
		if(def->X >= atk->X - rangeX && def->X <= atk->X 
			&&def->Y >= atk->Y && def->Y <= atk->Y + rangeY)
		{
			return true;
		}
	}
	else
	{
		if(def->X >= atk->X && def->X <= atk->X + rangeX
			&& def->Y >= atk->Y && def->Y <= atk->Y + rangeY)
		{
			return true;
		}
	}
	return false;
}
