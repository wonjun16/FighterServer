#include "Network.h"
#include "System.h"
#include <Windows.h>
#include <mmsystem.h>
#include "PacketDefine.h"

#pragma comment(lib, "winmm.lib")

System::System(void)
{
	Shutdown = false;
	LastUpdateTime = 0;
}

System::~System(void)
{
	timeEndPeriod(1);
}

bool System::Initialize(void)
{
	timeBeginPeriod(1);
	LastUpdateTime = timeGetTime();
	return true;
}

bool System::GetShutdown(void) const
{
	return Shutdown;
}

void System::ShouldShutdown(void)
{
	Shutdown = true;
}

void System::Update(void)
{
	unsigned long currentTime = timeGetTime();
	if (currentTime - LastUpdateTime < 20) return;
	LastUpdateTime = currentTime;

	for(auto it = Network::GetInstance()->SessionList.begin(); it != Network::GetInstance()->SessionList.end(); ++it)
	{
		Session* session = *it;
		if (!session->IsAlive) continue;
		if (session->HP <= 0)
		{
			Network::GetInstance()->Disconnect(session);
			continue;
		}

		if(session->IsMoving)
		{
			switch (session->Action)
			{
			case dfPACKET_MOVE_DIR_RR:
				session->X = min(session->X + 3, dfRANGE_MOVE_RIGHT);
				break;
			case dfPACKET_MOVE_DIR_RU:
				if (session->Y - 2 > dfRANGE_MOVE_TOP && session->X + 3 < dfRANGE_MOVE_RIGHT)
				{
					session->X += 3;
					session->Y -= 2;
				}
				break;
			case dfPACKET_MOVE_DIR_RD:
				if(session->Y + 2 < dfRANGE_MOVE_BOTTOM && session->X + 3 < dfRANGE_MOVE_RIGHT)
				{
					session->X += 3;
					session->Y += 2;
				}
				break;
			case dfPACKET_MOVE_DIR_LL:
				session->X = max(session->X-3, dfRANGE_MOVE_LEFT);
				break;
			case dfPACKET_MOVE_DIR_LU:
				if( session->Y - 2 > dfRANGE_MOVE_TOP && session->X - 3 > dfRANGE_MOVE_LEFT)
				{
					session->X -= 3;
					session->Y -= 2;
				}
				break;
			case dfPACKET_MOVE_DIR_LD:
				if(session->Y + 2 < dfRANGE_MOVE_BOTTOM && session->X - 3 > dfRANGE_MOVE_LEFT)
				{
					session->X -= 3;
					session->Y += 2;
				}
				break;
			case dfPACKET_MOVE_DIR_UU:
				session->Y = max(session->Y - 2, dfRANGE_MOVE_TOP);
				break;
			case dfPACKET_MOVE_DIR_DD:
				session->Y = min(session->Y + 2, dfRANGE_MOVE_BOTTOM);
				break;
			default:
				break;
			}
			printf("# Update Position # ID : %d # X : %d # Y : %d\n", session->SessionId, session->X, session->Y);
		}
	}
}

unsigned long System::GetLastUpdateTime(void) const
{
	return LastUpdateTime;
}
