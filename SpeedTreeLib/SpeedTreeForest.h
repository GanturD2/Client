#pragma once


///////////////////////////////////////////////////////////////////////
//	Include Files
#include <SpeedTreeRT.h>
#include "SpeedTreeWrapper.h"

#include <vector>
#include <map>

///////////////////////////////////////////////////////////////////////
//	Render bit vector

#define Forest_RenderBranches		(1 << 0)
#define Forest_RenderLeaves			(1 << 1)
#define Forest_RenderFronds			(1 << 2)
#define Forest_RenderBillboards		(1 << 3)
#define Forest_RenderAll			((1 << 4) - 1)
#define Forest_RenderToShadow		(1 << 5)
#define Forest_RenderToMiniMap		(1 << 6)

///////////////////////////////////////////////////////////////////////
//	class CSpeedTreeForest declaration

class CSpeedTreeForest : public CSingleton<CSpeedTreeForest>
{
public:
	typedef std::map<uint32_t, CSpeedTreeWrapper*> TTreeMap;

public:
	CSpeedTreeForest();
	virtual ~CSpeedTreeForest();

	void ClearMainTree();

	BOOL GetMainTree(uint32_t dwCRC, CSpeedTreeWrapper** ppMainTree, const char * c_pszFileName);
	CSpeedTreeWrapper* GetMainTree(uint32_t dwCRC);
	void DeleteMainTree(uint32_t dwCRC);

	CSpeedTreeWrapper* CreateInstance(float x, float y, float z, uint32_t dwTreeCRC, const char * c_szTreeName);
	void DeleteInstance(CSpeedTreeWrapper* pInstance);

	void UpdateSystem(float fCurrentTime);

	void Clear();

	void SetLight(const float * afDirection, const float * afAmbient, const float * afDiffuse);
	void SetFog(float fFogNear, float fFogFar);
	//////////////////////////////////////////////////////////////////////////

	const float * GetExtents(void) const { return m_afForestExtents; }

	// wind management
	float GetWindStrength(void) const { return m_fWindStrength; }
	void SetWindStrength(float fStrength);
	void SetupWindMatrices(float fTimeInSecs);

	// overridden by specific graphics API
	virtual void UploadWindMatrix(unsigned int uiLocation, const float * pMatrix) const = 0;
	virtual void Render(unsigned long ulRenderBitVector = Forest_RenderAll) = 0;
	virtual bool SetRenderingDevice() = 0;
	virtual	void UpdateCompundMatrix(const D3DXVECTOR3& c_rEyeVec, const D3DXMATRIX& c_rmatView, const D3DXMATRIX& c_rmatProj) = 0;

protected:
	TTreeMap m_pMainTreeMap;

	float m_afLighting[12];
	float m_afFog[4];

private:
	void AdjustExtents(float x, float y, float z);

	float m_afForestExtents[6]; // [0] = min x, [1] = min y..., [3] = max x, [4] = max y...
	float m_fWindStrength; // 0.0 = no wind, 1.0 = full strength

	float m_fAccumTime;
};
