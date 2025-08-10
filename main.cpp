#include "Network.h"
#include "System.h"

int main()
{
	System* sys = System::GetInstance();
	sys->Initialize();

	Network* net = Network::GetInstance();
	if (!net->Initialize())
	{
		return -1;
	}

	while (!sys->GetShutdown())
	{
		net->NetIOProcess();
		sys->Update();
		//Frame -> time 
	}
	return 0;
}