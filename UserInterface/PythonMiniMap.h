#pragma once

#include "PythonBackground.h"
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
#	include "Packet.h"
#	include <unordered_map>
#endif

class CPythonMiniMap : public CScreen, public CSingleton<CPythonMiniMap>
{
public:
	enum
	{
		EMPIRE_NUM = 4,

		MINI_WAYPOINT_IMAGE_COUNT = 12,
		WAYPOINT_IMAGE_COUNT = 15,
		TARGET_MARK_IMAGE_COUNT = 2
	};
	enum
	{
		TYPE_OPC,
		TYPE_OPCPVP,
		TYPE_OPCPVPSELF,
		TYPE_NPC,
		TYPE_MONSTER,
		TYPE_WARP,
		TYPE_WAYPOINT,
		TYPE_PARTY,
		TYPE_EMPIRE,
		TYPE_EMPIRE_END = TYPE_EMPIRE + EMPIRE_NUM,
		TYPE_TARGET,
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		TYPE_SHOP,
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
		TYPE_PET,
#endif
		TYPE_COUNT
	};

public:
	CPythonMiniMap();
	~CPythonMiniMap();
	CLASS_DELETE_COPYMOVE(CPythonMiniMap);

	void Destroy();
	bool Create();

	bool IsAtlas() const;
	bool CanShow() const;
	bool CanShowAtlas() const;

	void SetMiniMapSize(float fWidth, float fHeight);
	void SetScale(float fScale);
	void ScaleUp();
	void ScaleDown();

	void SetCenterPosition(float fCenterX, float fCenterY);

	void Update(float fCenterX, float fCenterY);
	void Render(float fScreenX, float fScreenY);

	void Show();
	void Hide();

	bool GetPickedInstanceInfo(float fScreenX, float fScreenY, std::string & rReturnName, float * pReturnPosX, float * pReturnPosY,
							   uint32_t * pdwTextColor) const;

	// Atlas
	bool LoadAtlas();
	void UpdateAtlas();
	void RenderAtlas(float fScreenX, float fScreenY);
	void ShowAtlas();
	void HideAtlas();

	bool GetAtlasInfo(float fScreenX, float fScreenY, std::string & rReturnString, float * pReturnPosX, float * pReturnPosY,
					  uint32_t * pdwTextColor, uint32_t * pdwGuildID);
	bool GetAtlasSize(float * pfSizeX, float * pfSizeY) const;

#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	void RemovePartyPositionInfo(uint32_t dwPID);
	void AddPartyPositionInfo(const SPartyPosition& Info);
#endif

	// WayPoint
	void AddWayPoint(uint8_t byType, uint32_t dwID, float fX, float fY, std::string strText, uint32_t dwChrVID = 0);
	void RemoveWayPoint(uint32_t dwID);

	// SignalPoint
	void AddSignalPoint(float fX, float fY);
	void ClearAllSignalPoint();

	void RegisterAtlasWindow(PyObject * poHandler);
	void UnregisterAtlasWindow();
	void OpenAtlasWindow() const;
	void SetAtlasCenterPosition(int x, int y) const;

	// NPC List
	void ClearAtlasMarkInfo();
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	void ClearAtlasShopInfo();
#endif
	void RegisterAtlasMark(uint8_t byType, const char * c_szName, uint32_t dwMobVnum, long lx, long ly);

	// Guild
	void ClearGuildArea();
	void RegisterGuildArea(uint32_t dwID, uint32_t dwGuildID, long x, long y, long width, long height);
	uint32_t GetGuildAreaID(uint32_t x, uint32_t y);

	// Target
	void CreateTarget(int iID, const char * c_szName);
	void CreateTarget(int iID, const char * c_szName, uint32_t dwVID);
	void UpdateTarget(int iID, int ix, int iy);
	void DeleteTarget(int iID);
#ifdef ENABLE_ATLAS_SCALE
	void SetAtlasScale(float fx, float fy);
#endif

protected:
	void __Initialize();
	void __SetPosition();
	void __LoadAtlasMarkInfo();

	void __RenderWayPointMark(int ixCenter, int iyCenter);
	void __RenderMiniWayPointMark(int ixCenter, int iyCenter);
	void __RenderTargetMark(int ixCenter, int iyCenter);

	void __GlobalPositionToAtlasPosition(long lx, long ly, float * pfx, float * pfy) const;

protected:
	// Atlas
	typedef struct
	{
		uint8_t m_byType;
		uint32_t m_dwID; // For WayPoint
		float m_fX;
		float m_fY;
		float m_fScreenX;
		float m_fScreenY;
		float m_fMiniMapX;
		float m_fMiniMapY;
		uint32_t m_dwChrVID;
		std::string m_strText;
		uint32_t m_dwMobVnum;
	} TAtlasMarkInfo;

	// GuildArea
	typedef struct
	{
		uint32_t dwGuildID;
		long lx, ly;
		long lwidth, lheight;

		float fsxRender, fsyRender;
		float fexRender, feyRender;
	} TGuildAreaInfo;

	struct SObserver
	{
		float fCurX;
		float fCurY;
		float fSrcX;
		float fSrcY;
		float fDstX;
		float fDstY;

		uint32_t dwSrcTime;
		uint32_t dwDstTime;
	};

#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	struct SPartyPlayerPosition
	{
		~SPartyPlayerPosition()
		{
			grMarkImage.Destroy();
		}
		std::string sName;
		long	lX, lY;
		float	fScreenX, fScreenY;
		float	fRot;
		CGraphicExpandedImageInstance grMarkImage;
	};
#endif

	// 캐릭터 리스트
	typedef struct
	{
		float m_fX;
		float m_fY;
		uint32_t m_eNameColor;
	} TMarkPosition;

	using TInstanceMarkPositionVector = std::vector<TMarkPosition>;
	using TInstancePositionVectorIterator = TInstanceMarkPositionVector::iterator;

protected:
	bool __GetWayPoint(uint32_t dwID, TAtlasMarkInfo ** ppkInfo);
	void __UpdateWayPoint(TAtlasMarkInfo * pkInfo, int ix, int iy) const;

protected:
	float m_fWidth;
	float m_fHeight;

	float m_fScale;

	float m_fCenterX;
	float m_fCenterY;

	float m_fCenterCellX;
	float m_fCenterCellY;

	float m_fScreenX;
	float m_fScreenY;

	float m_fMiniMapRadius;

	// 맵 그림...
	LPDIRECT3DTEXTURE9 m_lpMiniMapTexture[AROUND_AREA_NUM];

	// 미니맵 커버
	CGraphicImageInstance m_MiniMapFilterGraphicImageInstance;
	CGraphicExpandedImageInstance m_MiniMapCameraraphicImageInstance;

	// 캐릭터 마크
	CGraphicExpandedImageInstance m_PlayerMark;
	CGraphicImageInstance m_WhiteMark;
	CGraphicImageInstance			m_BossMark;	//@custom013

	TInstanceMarkPositionVector m_PartyPCPositionVector;
	TInstanceMarkPositionVector m_OtherPCPositionVector;
	TInstanceMarkPositionVector m_NPCPositionVector;
	TInstanceMarkPositionVector		m_MetinPositionVector;
	TInstanceMarkPositionVector		m_BossPositionVector;
	TInstanceMarkPositionVector m_MonsterPositionVector;
	TInstanceMarkPositionVector m_WarpPositionVector;
#ifdef ENABLE_BATTLE_FIELD
	TInstanceMarkPositionVector		m_TargetPCVictimPositionVector;
#endif
#ifdef ENABLE_PROTO_RENEWAL
	TInstanceMarkPositionVector		m_PetPayPositionVector;
	TInstanceMarkPositionVector		m_HorsePositionVector;
#endif
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	TInstanceMarkPositionVector		m_ShopPositionVector;
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	TInstanceMarkPositionVector m_GrowthPetPositionVector;
#endif
	std::map<uint32_t, SObserver> m_kMap_dwVID_kObserver;

	bool m_bAtlas;
	bool m_bShow;

	CGraphicVertexBuffer m_VertexBuffer;
	CGraphicIndexBuffer m_IndexBuffer;

	D3DXMATRIX m_matIdentity;
	D3DXMATRIX m_matWorld;
	D3DXMATRIX m_matMiniMapCover;

	bool m_bShowAtlas;
	CGraphicImageInstance m_AtlasImageInstance;
	D3DXMATRIX m_matWorldAtlas;
	CGraphicExpandedImageInstance m_AtlasPlayerMark;
#ifdef WJ_SHOW_PARTY_ON_MINIMAP
	std::unordered_map<uint32_t, std::shared_ptr<SPartyPlayerPosition>> m_AtlasPartyPlayerMark;
	uint32_t m_dwLastPartyMarkUpdate;
#endif

	float m_fAtlasScreenX;
	float m_fAtlasScreenY;

	uint32_t m_dwAtlasBaseX;
	uint32_t m_dwAtlasBaseY;

	float m_fAtlasMaxX;
	float m_fAtlasMaxY;

	float m_fAtlasImageSizeX;
	float m_fAtlasImageSizeY;

	using TAtlasMarkInfoVector = std::vector<TAtlasMarkInfo>;
	using TAtlasMarkInfoVectorIterator = TAtlasMarkInfoVector::iterator;
	using TGuildAreaInfoVector = std::vector<TGuildAreaInfo>;
	using TGuildAreaInfoVectorIterator = TGuildAreaInfoVector::iterator;
	TAtlasMarkInfoVectorIterator m_AtlasMarkInfoVectorIterator;
	TAtlasMarkInfoVector m_AtlasNPCInfoVector;
	TAtlasMarkInfoVector m_AtlasWarpInfoVector;
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
	TAtlasMarkInfoVector					m_AtlasShopInfoVector;
#endif

	// WayPoint
	CGraphicExpandedImageInstance m_MiniWayPointGraphicImageInstances[MINI_WAYPOINT_IMAGE_COUNT];
	CGraphicExpandedImageInstance m_WayPointGraphicImageInstances[WAYPOINT_IMAGE_COUNT];
	CGraphicExpandedImageInstance m_TargetMarkGraphicImageInstances[TARGET_MARK_IMAGE_COUNT];
	CGraphicImageInstance m_GuildAreaFlagImageInstance;
	TAtlasMarkInfoVector m_AtlasWayPointInfoVector;
	TGuildAreaInfoVector m_GuildAreaInfoVector;

	// SignalPoint
	struct TSignalPoint
	{
		D3DXVECTOR2 v2Pos;
		unsigned int id;
	};
	std::vector<TSignalPoint> m_SignalPointVector;

	PyObject * m_poHandler;

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
public:
	bool ReloadAtlas(bool isSnow = false);
#endif
};