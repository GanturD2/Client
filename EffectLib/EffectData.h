#pragma once

#include "../MilesLib/Type.h"

#include "ParticleSystemData.h"
#include "EffectMesh.h"
#include "SimpleLightData.h"

class CEffectData
{
public:
	using TParticleVector = std::vector<CParticleSystemData *>;
	using TMeshVector = std::vector<CEffectMeshScript *>;
	using TLightVector = std::vector<CLightData *>;

public:
	CEffectData();
	virtual ~CEffectData() = default;

	void Clear();
	bool LoadScript(const char * c_szFileName);
	bool LoadSoundScriptData(const char * c_szFileName);

	uint32_t GetParticleCount() const;
	CParticleSystemData * GetParticlePointer(uint32_t dwPosition);

	uint32_t GetMeshCount() const;
	CEffectMeshScript * GetMeshPointer(uint32_t dwPosition);

	uint32_t GetLightCount() const;
	CLightData * GetLightPointer(uint32_t dwPosition);

	NSound::TSoundInstanceVector * GetSoundInstanceVector();

	float GetBoundingSphereRadius() const;
	D3DXVECTOR3 GetBoundingSpherePosition() const;

	const char * GetFileName() const;

protected:
	void __ClearParticleDataVector();
	void __ClearLightDataVector();
	void __ClearMeshDataVector();

	// FIXME : 이 부분은 그다지 맘에 들지 않는다. 좋은 아이디어를 찾아내어 고치자.
	//         상위가 (특화된) 상위의 인터페이스 때문에 모양이 바뀌어야 한다는 것은 옳지 못하다. - [levites]
	virtual CParticleSystemData * AllocParticle();
	virtual CEffectMeshScript * AllocMesh();
	virtual CLightData * AllocLight();

protected:
	TParticleVector m_ParticleVector;
	TMeshVector m_MeshVector;
	TLightVector m_LightVector;
	NSound::TSoundInstanceVector m_SoundInstanceVector;

	float m_fBoundingSphereRadius;
	D3DXVECTOR3 m_v3BoundingSpherePosition;

	std::string m_strFileName;

public:
	static void DestroySystem();

	static CEffectData * New();
	static void Delete(CEffectData * pkData);

	static CDynamicPool<CEffectData> ms_kPool;
};
