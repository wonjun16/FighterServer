#pragma once

#include <Windows.h>

template<typename T>
class SingletonTemplate
{
protected:
	SingletonTemplate() {};
	virtual ~SingletonTemplate() {};

public:
	static T* GetInstance() noexcept
	{
		if (_Instance == nullptr)
		{
			_Instance = new T;
			atexit(DestroyInstance);
		}

		return _Instance;
	}

	static void DestroyInstance() noexcept
	{
		if (_Instance)
		{
			delete _Instance;
			_Instance = nullptr;
		}
	}

private:
	inline static T* _Instance = nullptr;
};