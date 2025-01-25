#pragma once

#include <d3d9/d3dx9.h>

#include "../EterLib/Resource.h"
#include "../EterLib/GrpImageInstance.h"
#include "../EterLib/TextFileLoader.h"

#include "Type.h"
#include "EffectElementBase.h"

class CEffectMesh : public CResource
{
public:
	typedef struct SEffectFrameData
	{
		uint8_t byChangedFrame{};
		float fVisibility{};
		uint32_t dwVertexCount{};
		uint32_t dwTextureVertexCount{};
		uint32_t dwIndexCount{};
		std::vector<TPTVertex> PDTVertexVector;
	} TEffectFrameData;

	typedef struct SEffectMeshData
	{
		char szObjectName[32]{};
		char szDiffuseMapFileName[128]{};

		std::vector<TEffectFrameData> EffectFrameDataVector;
		std::vector<CGraphicImage *> pImageVector;

		static SEffectMeshData * New();
		static void Delete(SEffectMeshData * pkData);

		static void DestroySystem();

		static CDynamicPool<SEffectMeshData> ms_kPool;
	} TEffectMeshData;

	// About Resource Code
public:
	using TRef = CRef<CEffectMesh>;

public:
	static TType Type();

public:
	explicit CEffectMesh(const char * c_szFileName);
	~CEffectMesh() override;

	uint32_t GetFrameCount() const;
	uint32_t GetMeshCount() const;
	TEffectMeshData * GetMeshDataPointer(uint32_t dwMeshIndex);

	std::vector<CGraphicImage *> * GetTextureVectorPointer(uint32_t dwMeshIndex);
	std::vector<CGraphicImage *> & GetTextureVectorReference(uint32_t dwMeshIndex);

	// Exceptional function for tool
	BOOL GetMeshElementPointer(uint32_t dwMeshIndex, TEffectMeshData ** ppMeshData);

protected:
	bool OnLoad(int iSize, const void * c_pvBuf) override;

	void OnClear() override;
	bool OnIsEmpty() const override;
	bool OnIsType(TType type) override;

	BOOL __LoadData_Ver001(int iSize, const uint8_t * c_pbBuf);
	BOOL __LoadData_Ver002(int iSize, const uint8_t * c_pbBuf);

protected:
	int m_iGeomCount;
	int m_iFrameCount;
	std::vector<TEffectMeshData *> m_pEffectMeshDataVector;

	bool m_isData;
};

class CEffectMeshScript : public CEffectElementBase
{
public:
	typedef struct SMeshData
	{
		uint8_t byBillboardType{};

		BOOL bBlendingEnable{};
		uint8_t byBlendingSrcType{};
		uint8_t byBlendingDestType{};
		BOOL bTextureAlphaEnable{};

		uint8_t byColorOperationType{};
		D3DXCOLOR ColorFactor;

		BOOL bTextureAnimationLoopEnable{};
		float fTextureAnimationFrameDelay{};

		uint32_t dwTextureAnimationStartFrame{};

		TTimeEventTableFloat TimeEventAlpha;

		SMeshData() { TimeEventAlpha.clear(); }
	} TMeshData;

	using TMeshDataVector = std::vector<TMeshData>;

public:
	CEffectMeshScript();
	~CEffectMeshScript() override;

	const char * GetMeshFileName() const;

	void ReserveMeshData(uint32_t dwMeshCount);
	bool CheckMeshIndex(uint32_t dwMeshIndex) const;
	bool GetMeshDataPointer(uint32_t dwMeshIndex, TMeshData ** ppMeshData);
	int GetMeshDataCount() const;

	int GetBillboardType(uint32_t dwMeshIndex);
	BOOL isBlendingEnable(uint32_t dwMeshIndex);
	uint8_t GetBlendingSrcType(uint32_t dwMeshIndex);
	uint8_t GetBlendingDestType(uint32_t dwMeshIndex);
	BOOL isTextureAlphaEnable(uint32_t dwMeshIndex);
	BOOL GetColorOperationType(uint32_t dwMeshIndex, uint8_t * pbyType);
	BOOL GetColorFactor(uint32_t dwMeshIndex, D3DXCOLOR * pColor);
	BOOL GetTimeTableAlphaPointer(uint32_t dwMeshIndex, TTimeEventTableFloat ** pTimeEventAlpha);

	BOOL isMeshAnimationLoop() const;
	BOOL GetMeshAnimationLoopCount() const;
	float GetMeshAnimationFrameDelay() const;
	BOOL isTextureAnimationLoop(uint32_t dwMeshIndex);
	float GetTextureAnimationFrameDelay(uint32_t dwMeshIndex);
	uint32_t GetTextureAnimationStartFrame(uint32_t dwMeshIndex);

protected:
	void OnClear() override;
	bool OnIsData() override;
	BOOL OnLoadScript(CTextFileLoader & rTextFileLoader) override;

protected:
	BOOL m_isMeshAnimationLoop{};
	int m_iMeshAnimationLoopCount{};
	float m_fMeshAnimationFrameDelay{};
	TMeshDataVector m_MeshDataVector;

	std::string m_strMeshFileName;

public:
	static void DestroySystem();

	static CEffectMeshScript * New();
	static void Delete(CEffectMeshScript * pkData);

	static CDynamicPool<CEffectMeshScript> ms_kPool;
};
