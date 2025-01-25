#pragma once

#include "../eterGrnLib/ThingInstance.h"

class IActorInstance : public CGraphicThingInstance
{
public:
	enum
	{
		ID = ACTOR_OBJECT
	};
	int GetType() const noexcept override { return ID; }

	IActorInstance() = default;
	virtual ~IActorInstance() = default;
	virtual bool TestCollisionWithDynamicSphere(const CDynamicSphereInstance & dsi) = 0;
	virtual uint32_t GetVirtualID() = 0;
#ifdef ENABLE_GRAPHIC_ON_OFF
	virtual bool IsShowEffects() = 0;
	virtual bool IsShowActor() = 0;
#endif
};