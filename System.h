#pragma once
#include "SingletonTemplate.h"

class System : public SingletonTemplate<System>
{
	friend class SingletonTemplate<System>;
public:
	
	bool Initialize(void);

	bool GetShutdown(void) const;
	void ShouldShutdown(void);

	void Update(void);

	unsigned long GetLastUpdateTime(void) const;
private:
	System(void);
	~System(void);

	bool Shutdown;
	unsigned long LastUpdateTime;
};

