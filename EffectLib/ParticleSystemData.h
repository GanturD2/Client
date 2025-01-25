#pragma once

#include "../EterLib/TextFileLoader.h"

#include "EffectElementBase.h"
#include "EmitterProperty.h"
#include "ParticleProperty.h"
//#include "ParticleInstance.h"

class CParticleInstance;

class CParticleSystemData : public CEffectElementBase
{
public:
	~CParticleSystemData() override;
	CParticleSystemData();

	CEmitterProperty * GetEmitterPropertyPointer();
	CParticleProperty * GetParticlePropertyPointer();
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	CParticleProperty* GetParticlePropertyBackupPointer();
#endif


	void ChangeTexture(const char * c_szFileName);

	void BuildDecorator(CParticleInstance * pInstance);

protected:
	BOOL OnLoadScript(CTextFileLoader & rTextFileLoader) override;

	void OnClear() override;
	bool OnIsData() override;


	CEmitterProperty m_EmitterProperty;
	CParticleProperty m_ParticleProperty;
#ifdef ENABLE_SKILL_COLOR_SYSTEM
	CParticleProperty m_ParticleProperty_Backup;
#endif

public:
	static void DestroySystem();

	static CParticleSystemData * New();
	static void Delete(CParticleSystemData * pkData);

	static CDynamicPool<CParticleSystemData> ms_kPool;
};
