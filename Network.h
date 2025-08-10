#pragma once

#include <WinSock2.h>
#include <list>
#include "Session.h"
#include "SingletonTemplate.h"

#pragma comment(lib, "ws2_32.lib")

class Network : public SingletonTemplate<Network>
{
	friend class SingletonTemplate<Network>;
public:
	bool Initialize(void);
	void NetIOProcess(void);
	void AcceptProc(void);
	void RecvProc(Session* s);
	void SendProc(Session* s);
	void Disconnect(Session* s);
	Session* CreateSession(SOCKET sock, SOCKADDR_IN* addr);
	void PacketProc(Session* s, unsigned char type);

	void CreateMyCharacter(Session* s);
	void CreateOtherCharacter(Session* mine, Session* other);
	void DeleteCharacter(Session* s, Session* target);
	void MoveStart(Session* s, Session* target);
	void MoveStop(Session* s, Session* target);
	void Attack1(Session* s, Session* target);
	void Attack2(Session* s, Session* target);
	void Attack3(Session* s, Session* target);
	void GetDamage(Session* s, Session* target);

	bool CheckAttackValid(Session* s, unsigned char attackType);
	void CheckDemaged(Session* atk, unsigned char attackType);
	bool IsInRange(Session* atk, Session* def, int rangeX, int rangeY, unsigned char dir);

	std::list<Session*> SessionList;

private:
	Network(void);
	~Network(void);

	unsigned short PORT;
	SOCKET ListenSock;
	FD_SET Rset;
	FD_SET Wset;

	unsigned long SessionIdCounter;

	int CurSessionCount;
};

