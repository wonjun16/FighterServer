#pragma once
#include <cstdint>
#include "RingBuffer.h"

class Session
{
public:
	Session(void);
	~Session(void);
	
	uintptr_t Socket;
	unsigned long SessionId;
	RingBuffer RecvQ;
	RingBuffer SendQ;

	char ValidCount;

	bool IsAlive;

	unsigned long Action;
	unsigned char Direction;

	short X;
	short Y;

	bool IsMoving;
	unsigned long LastAttackTime;
	unsigned char LastAttackType;

	char HP;
};

