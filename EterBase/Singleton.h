#ifndef __INC_ETERLIB_SINGLETON_H__
#define __INC_ETERLIB_SINGLETON_H__

#include <cassert>

#ifndef NDEBUG
#	define ENABLE_SINGLETON_INFORMATION // marty 1 - singleton ymir retarded version 0
#endif

#ifdef ENABLE_SINGLETON_INFORMATION
#	include <typeinfo>
#endif

template <typename T> class CSingleton
{
	static T * ms_singleton;

public:
	CSingleton()
	{
#ifdef ENABLE_SINGLETON_INFORMATION
		if (ms_singleton)
			MessageBoxA(NULL, typeid(T).name(), "CSingleton() DECLARED MORE THAN ONCE", MB_ICONEXCLAMATION | MB_OK);
#endif
		assert(!ms_singleton);
		int offset = (int) (T *) 1 - (int) (CSingleton<T> *) (T *) 1;
		ms_singleton = (T *) ((int) this + offset);
	}

	virtual ~CSingleton()
	{
#ifdef ENABLE_SINGLETON_INFORMATION
		if (!ms_singleton)
			MessageBoxA(NULL, typeid(T).name(), "~CSingleton() FREED AT RUNTIME", MB_ICONEXCLAMATION | MB_OK);
#endif
		assert(ms_singleton);
		ms_singleton = nullptr;
	}

	__forceinline static T & Instance()
	{
#ifdef ENABLE_SINGLETON_INFORMATION
		if (!ms_singleton)
			MessageBoxA(NULL, typeid(T).name(), "CSingleton::Instance() NEVER DECLARED", MB_ICONEXCLAMATION | MB_OK);
#endif
		assert(ms_singleton);
		return (*ms_singleton);
	}

	__forceinline static T * InstancePtr() { return (ms_singleton); }
};

template <typename T> T * CSingleton<T>::ms_singleton = nullptr;

#endif
