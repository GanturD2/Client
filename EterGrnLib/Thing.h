#pragma once

#include "Model.h"
#include "Motion.h"

class CGraphicThing : public CResource
{
public:
	using TRef = CRef<CGraphicThing>;

public:
	static TType Type();

public:
	CGraphicThing(const char * c_szFileName);
	~CGraphicThing() override;

	bool CreateDeviceObjects() override;
	void DestroyDeviceObjects() override;

	bool CheckModelIndex(int iModel) const;
	CGrannyModel * GetModelPointer(int iModel) const;
	int GetModelCount() const;

	bool CheckMotionIndex(int iMotion) const;
	CGrannyMotion * GetMotionPointer(int iMotion) const;
	int GetMotionCount() const;
	int						GetTextureCount() const;
	const char *			GetTexturePath(int iTexture);

protected:
	void Initialize();

	bool LoadModels();
	bool LoadMotions();

protected:
	bool OnLoad(int iSize, const void * c_pvBuf) override;
	void OnClear() override;
	bool OnIsEmpty() const override;
	bool OnIsType(TType type) override;

protected:
	granny_file * m_pgrnFile;
	granny_file_info * m_pgrnFileInfo;

	granny_animation * m_pgrnAni;

	CGrannyModel * m_models;
	CGrannyMotion * m_motions;
};
