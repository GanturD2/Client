#pragma once

#include "AbstractSingleton.h"

class IAbstractChat : public TAbstractSingleton<IAbstractChat>
{
public:
	IAbstractChat() = default;
	~IAbstractChat() = default;

	virtual void AppendChat(int iType, const char * c_szChat) = 0;
};