#pragma once

#include "AbstractSingleton.h"

class CInstanceBase;

class IAbstractCharacterManager : public TAbstractSingleton<IAbstractCharacterManager>
{
public:
	IAbstractCharacterManager() = default;
	~IAbstractCharacterManager() = default;

	virtual void Destroy() = 0;
	virtual CInstanceBase * GetInstancePtr(uint32_t dwVID) = 0;
};