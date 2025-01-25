#ifndef __METIN_II_COMMON_BUILDING_H__
#define __METIN_II_COMMON_BUILDING_H__

namespace building
{
	enum
	{
		OBJECT_MATERIAL_MAX_NUM = 5,
	};

	typedef struct SLand
	{
		uint32_t dwID;
		long lMapIndex;
		long x, y;
		long width, height;
		uint32_t dwGuildID;
		uint8_t bGuildLevelLimit;
		uint32_t dwPrice;
	} TLand;

	typedef struct SObjectMaterial
	{
		uint32_t dwItemVnum;
		uint32_t dwCount;
	} TObjectMaterial;

	typedef struct SObjectProto
	{
		uint32_t dwVnum;
		uint32_t dwPrice;

		TObjectMaterial kMaterials[OBJECT_MATERIAL_MAX_NUM];

		uint32_t dwUpgradeVnum;
		uint32_t dwUpgradeLimitTime;
		long lLife;
		long lRegion[4];

		uint32_t dwNPCVnum;
		long lNPCX;
		long lNPCY;

		uint32_t dwGroupVnum;
		uint32_t dwDependOnGroupVnum;
	} TObjectProto;

	typedef struct SObject
	{
		uint32_t dwID;
		uint32_t dwLandID;
		uint32_t dwVnum;
		long lMapIndex;
		long x, y;

		float xRot;
		float yRot;
		float zRot;
		long lLife;
	} TObject;
};

#endif
