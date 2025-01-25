#include "StdAfx.h"
#include "PythonSystem.h"

PyObject * systemGetWidth(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetWidth());
}

PyObject * systemGetHeight(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetHeight());
}

PyObject * systemSetInterfaceHandler(PyObject * poSelf, PyObject * poArgs)
{
	PyObject * poHandler;
	if (!PyTuple_GetObject(poArgs, 0, &poHandler))
		return Py_BuildException();

	CPythonSystem::Instance().SetInterfaceHandler(poHandler);
	return Py_BuildNone();
}

PyObject * systemDestroyInterfaceHandler(PyObject * poSelf, PyObject * poArgs)
{
	CPythonSystem::Instance().DestroyInterfaceHandler();
	return Py_BuildNone();
}

PyObject * systemReserveResource(PyObject * poSelf, PyObject * poArgs)
{
	std::set<std::string> ResourceSet;
	CResourceManager::Instance().PushBackgroundLoadingSet(ResourceSet);
	return Py_BuildNone();
}

PyObject * systemisInterfaceConfig(PyObject * poSelf, PyObject * poArgs)
{
	int isInterfaceConfig = CPythonSystem::Instance().isInterfaceConfig();
	return Py_BuildValue("i", isInterfaceConfig);
}

PyObject * systemSaveWindowStatus(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	int iVisible;
	if (!PyTuple_GetInteger(poArgs, 1, &iVisible))
		return Py_BuildException();

	int iMinimized;
	if (!PyTuple_GetInteger(poArgs, 2, &iMinimized))
		return Py_BuildException();

	int ix;
	if (!PyTuple_GetInteger(poArgs, 3, &ix))
		return Py_BuildException();

	int iy;
	if (!PyTuple_GetInteger(poArgs, 4, &iy))
		return Py_BuildException();

	int iHeight;
	if (!PyTuple_GetInteger(poArgs, 5, &iHeight))
		return Py_BuildException();

	CPythonSystem::Instance().SaveWindowStatus(iIndex, iVisible, iMinimized, ix, iy, iHeight);
	return Py_BuildNone();
}

PyObject * systemGetWindowStatus(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const CPythonSystem::TWindowStatus & c_rWindowStatus = CPythonSystem::Instance().GetWindowStatusReference(iIndex);
	return Py_BuildValue("iiiii", c_rWindowStatus.isVisible, c_rWindowStatus.isMinimized, c_rWindowStatus.ixPosition,
						 c_rWindowStatus.iyPosition, c_rWindowStatus.iHeight);
}

PyObject * systemGetConfig(PyObject * poSelf, PyObject * poArgs)
{
	CPythonSystem::TConfig * tmp = CPythonSystem::Instance().GetConfig();

	int iRes = CPythonSystem::Instance().GetResolutionIndex(tmp->width, tmp->height, tmp->bpp);
	int iFrequency = CPythonSystem::Instance().GetFrequencyIndex(iRes, tmp->frequency);

	return Py_BuildValue("iiiiiiii", iRes, iFrequency, tmp->is_software_cursor, tmp->is_object_culling, tmp->music_volume,
						 tmp->voice_volume, tmp->gamma, tmp->iDistance);
}

PyObject * systemSetSaveID(PyObject * poSelf, PyObject * poArgs)
{
	int iValue;
	if (!PyTuple_GetInteger(poArgs, 0, &iValue))
		return Py_BuildException();

	char * szSaveID;
	if (!PyTuple_GetString(poArgs, 1, &szSaveID))
		return Py_BuildException();

	CPythonSystem::Instance().SetSaveID(iValue, szSaveID);
	return Py_BuildNone();
}

PyObject * systemisSaveID(PyObject * poSelf, PyObject * poArgs)
{
	int value = CPythonSystem::Instance().IsSaveID();
	return Py_BuildValue("i", value);
}

PyObject * systemGetSaveID(PyObject * poSelf, PyObject * poArgs)
{
	const char * c_szSaveID = CPythonSystem::Instance().GetSaveID();
	return Py_BuildValue("s", c_szSaveID);
}

PyObject * systemGetMusicVolume(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("f", CPythonSystem::Instance().GetMusicVolume());
}

PyObject * systemGetSoundVolume(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetSoundVolume());
}

PyObject * systemSetMusicVolume(PyObject * poSelf, PyObject * poArgs)
{
	float fVolume;
	if (!PyTuple_GetFloat(poArgs, 0, &fVolume))
		return Py_BuildException();

	CPythonSystem::Instance().SetMusicVolume(fVolume);
	return Py_BuildNone();
}

PyObject * systemSetSoundVolumef(PyObject * poSelf, PyObject * poArgs)
{
	float fVolume;
	if (!PyTuple_GetFloat(poArgs, 0, &fVolume))
		return Py_BuildException();

	CPythonSystem::Instance().SetSoundVolumef(fVolume);
	return Py_BuildNone();
}

PyObject * systemIsSoftwareCursor(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsSoftwareCursor());
}

PyObject * systemSetViewChatFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetViewChatFlag(iFlag);

	return Py_BuildNone();
}

PyObject * systemIsViewChat(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsViewChat());
}

PyObject * systemSetAlwaysShowNameFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetAlwaysShowNameFlag(iFlag);

	return Py_BuildNone();
}

#ifdef ENABLE_EXTENDED_CONFIGS
PyObject * systemSetCollectEquipment(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetCollectEquipment(iFlag);

	return Py_BuildNone();
}

PyObject * systemSetShowMoneyLog(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowMoneyLog(iFlag);

	return Py_BuildNone();
}
#endif

PyObject * systemSetShowDamageFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowDamageFlag(iFlag);

	return Py_BuildNone();
}

#ifdef ENABLE_REFINE_RENEWAL
PyObject * systemSetRefineStatus(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetRefineStatus(iFlag);
	return Py_BuildNone();
}
#endif

PyObject * systemSetShowSalesTextFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowSalesTextFlag(iFlag);

	return Py_BuildNone();
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
PyObject* systemSetShopTextTailLevel(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetShopTextTailLevel(iFlag);

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_EXTENDED_CONFIGS
PyObject * systemIsCollectEquipment(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsCollectEquipment());
}

PyObject * systemIsShowMoneyLog(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowMoneyLog());
}
#endif

#ifdef WJ_SHOW_MOB_INFO
PyObject* systemSetShowMobLevel(PyObject* poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowMobLevel(iOpt);
	return Py_BuildNone();
}

PyObject* systemIsShowMobLevel(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowMobLevel());
}

PyObject* systemSetShowMobAIFlag(PyObject* poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowMobAIFlag(iOpt);
	return Py_BuildNone();
}

PyObject* systemIsShowMobAIFlag(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowMobAIFlag());
}
#endif

#ifdef ENABLE_STRUCTURE_VIEW_MODE
PyObject* systemGetStructureViewMode(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetStructureViewMode());
}

PyObject* systemSetStructureViewMode(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetStructureViewMode(iFlag);
	return Py_BuildNone();
}
#endif

PyObject * systemIsAlwaysShowName(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsAlwaysShowName());
}

PyObject * systemIsShowDamage(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowDamage());
}

#ifdef ENABLE_REFINE_RENEWAL
PyObject * systemIsRefineStatusShow(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsRefineStatusShow());
}
#endif

PyObject * systemIsShowSalesText(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowSalesText());
}

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
PyObject* systemGetShopTextTailLevel(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetShopTextTailLevel());
}
#endif

PyObject * systemIsUsingItemHighlight(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsUsingItemHighlight());
}

PyObject * systemSetUsingItemHighlight(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetUsingItemHighlight(iFlag);

	return Py_BuildNone();
}

PyObject * systemSetConfig(PyObject * poSelf, PyObject * poArgs)
{
	int res_index;
	int width;
	int height;
	int bpp;
	int frequency_index;
	int frequency;
	int software_cursor;
	int shadow;
	int object_culling;
	int music_volume;
	int voice_volume;
	int gamma;
	int distance;

	if (!PyTuple_GetInteger(poArgs, 0, &res_index))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &frequency_index))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 2, &software_cursor))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 3, &shadow))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 4, &object_culling))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 5, &music_volume))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 6, &voice_volume))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 7, &gamma))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 8, &distance))
		return Py_BuildException();

	if (!CPythonSystem::Instance().GetResolution(res_index, reinterpret_cast<uint32_t *>(&width), reinterpret_cast<uint32_t *>(&height),
												 reinterpret_cast<uint32_t *>(&bpp)))
		return Py_BuildNone();

	if (!CPythonSystem::Instance().GetFrequency(res_index, frequency_index, reinterpret_cast<uint32_t *>(&frequency)))
		return Py_BuildNone();

	CPythonSystem::TConfig tmp;

	memcpy(&tmp, CPythonSystem::Instance().GetConfig(), sizeof(tmp));

	tmp.width = width;
	tmp.height = height;
	tmp.bpp = bpp;
	tmp.frequency = frequency;
	tmp.is_software_cursor = software_cursor ? true : false;
	tmp.is_object_culling = object_culling ? true : false;
	tmp.music_volume = static_cast<char>(music_volume);
	tmp.voice_volume = static_cast<char>(voice_volume);
	tmp.gamma = gamma;
	tmp.iDistance = distance;

	CPythonSystem::Instance().SetConfig(&tmp);
	return Py_BuildNone();
}

PyObject * systemApplyConfig(PyObject * poSelf, PyObject * poArgs)
{
	CPythonSystem::Instance().ApplyConfig();
	return Py_BuildNone();
}

PyObject * systemSaveConfig(PyObject * poSelf, PyObject * poArgs)
{
	int ret = CPythonSystem::Instance().SaveConfig();
	return Py_BuildValue("i", ret);
}

PyObject * systemGetResolutionCount(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetResolutionCount());
}

PyObject * systemGetFrequencyCount(PyObject * poSelf, PyObject * poArgs)
{
	int index;

	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonSystem::Instance().GetFrequencyCount(index));
}

PyObject * systemGetResolution(PyObject * poSelf, PyObject * poArgs)
{
	int index;
	uint32_t width = 0, height = 0, bpp = 0;

	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	CPythonSystem::Instance().GetResolution(index, &width, &height, &bpp);
	return Py_BuildValue("iii", width, height, bpp);
}

PyObject * systemGetCurrentResolution(PyObject * poSelf, PyObject * poArgs)
{
	CPythonSystem::TConfig * tmp = CPythonSystem::Instance().GetConfig();
	return Py_BuildValue("iii", tmp->width, tmp->height, tmp->bpp);
}

PyObject * systemGetFrequency(PyObject * poSelf, PyObject * poArgs)
{
	int index, frequency_index;
	uint32_t frequency = 0;

	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &frequency_index))
		return Py_BuildException();

	CPythonSystem::Instance().GetFrequency(index, frequency_index, &frequency);
	return Py_BuildValue("i", frequency);
}

#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
PyObject* systemGetShadowTargetLevel(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetShadowTargetLevel());
}

PyObject* systemSetShadowTargetLevel(PyObject* poSelf, PyObject* poArgs)
{
	int level;
	if (!PyTuple_GetInteger(poArgs, 0, &level))
		return Py_BuildException();

	CPythonSystem::Instance().SetShadowTargetLevel(level);
	return Py_BuildNone();
}

PyObject* systemGetShadowQualityLevel(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetShadowQualityLevel());
}

PyObject* systemSetShadowQualityLevel(PyObject* poSelf, PyObject* poArgs)
{
	int level;
	if (!PyTuple_GetInteger(poArgs, 0, &level))
		return Py_BuildException();

	CPythonSystem::Instance().SetShadowQualityLevel(level);
	return Py_BuildNone();
}
#else
PyObject * systemGetShadowLevel(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetShadowLevel());
}

PyObject * systemSetShadowLevel(PyObject * poSelf, PyObject * poArgs)
{
	int level;

	if (!PyTuple_GetInteger(poArgs, 0, &level))
		return Py_BuildException();

	if (level > 0)
		CPythonSystem::Instance().SetShadowLevel(level);

	return Py_BuildNone();
}
#endif

PyObject* systemSetFogMode(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetFogMode(iFlag);

	return Py_BuildNone();
}

PyObject* systemIsFogMode(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsFogMode());
}

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
PyObject* systemSetNightModeOption(PyObject* poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();

	CPythonSystem::Instance().SetNightModeOption(iOpt);
	return Py_BuildNone();
}

PyObject* systemGetNightModeOption(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetNightModeOption());
}

PyObject* systemSetSnowModeOption(PyObject* poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();

	CPythonSystem::Instance().SetSnowModeOption(iOpt);
	return Py_BuildNone();
}

PyObject* systemGetSnowModeOption(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetSnowModeOption());
}

PyObject* systemSetSnowTextureModeOption(PyObject* poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();

	CPythonSystem::Instance().SetSnowTextureModeOption(iOpt);
	return Py_BuildNone();
}

PyObject* systemGetSnowTextureModeOption(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetSnowTextureModeOption());
}

# ifdef ENABLE_ENVIRONMENT_RAIN
PyObject* systemSetRainModeOption(PyObject* poSelf, PyObject* poArgs)
{
	int iOpt;
	if (!PyTuple_GetInteger(poArgs, 0, &iOpt))
		return Py_BuildException();

	CPythonSystem::Instance().SetRainModeOption(iOpt);
	return Py_BuildNone();
}

PyObject* systemGetRainModeOption(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetRainModeOption());
}
# endif
#endif

#ifdef ENABLE_DICE_SYSTEM
PyObject* systemSetDiceChatShow(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetDiceChatShow(iFlag);
	return Py_BuildNone();
}

PyObject* systemIsDiceChatShow(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsDiceChatShow());
}
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
PyObject* systemSetShowPlayerLanguage(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowPlayerLanguage(iFlag);
	return Py_BuildNone();
}

PyObject* systemIsShowPlayerLanguage(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowPlayerLanguage());
}

PyObject* systemSetShowChatLanguage(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowChatLanguage(iFlag);
	return Py_BuildNone();
}

PyObject* systemIsShowChatLanguage(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowChatLanguage());
}

PyObject* systemSetShowWhisperLanguage(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowWhisperLanguage(iFlag);
	return Py_BuildNone();
}

PyObject* systemIsShowWhisperLanguage(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowWhisperLanguage());
}
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
PyObject* systemSetEquipmentView(PyObject* poSelf, PyObject* poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetEquipmentView(iFlag);
	return Py_BuildNone();
}

PyObject* systemGetEquipmentView(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetEquipmentView());
}
#endif

void initsystemSetting()
{
	static PyMethodDef s_methods[] = {{"GetWidth", systemGetWidth, METH_VARARGS},
									  {"GetHeight", systemGetHeight, METH_VARARGS},

									  {"SetInterfaceHandler", systemSetInterfaceHandler, METH_VARARGS},
									  {"DestroyInterfaceHandler", systemDestroyInterfaceHandler, METH_VARARGS},
									  {"ReserveResource", systemReserveResource, METH_VARARGS},

									  {"isInterfaceConfig", systemisInterfaceConfig, METH_VARARGS},
									  {"SaveWindowStatus", systemSaveWindowStatus, METH_VARARGS},
									  {"GetWindowStatus", systemGetWindowStatus, METH_VARARGS},

									  {"GetResolutionCount", systemGetResolutionCount, METH_VARARGS},
									  {"GetFrequencyCount", systemGetFrequencyCount, METH_VARARGS},

									  {"GetCurrentResolution", systemGetCurrentResolution, METH_VARARGS},

									  {"GetResolution", systemGetResolution, METH_VARARGS},
									  {"GetFrequency", systemGetFrequency, METH_VARARGS},

									  {"ApplyConfig", systemApplyConfig, METH_VARARGS},
									  {"SetConfig", systemSetConfig, METH_VARARGS},
									  {"SaveConfig", systemSaveConfig, METH_VARARGS},
									  {"GetConfig", systemGetConfig, METH_VARARGS},

									  {"SetSaveID", systemSetSaveID, METH_VARARGS},
									  {"isSaveID", systemisSaveID, METH_VARARGS},
									  {"GetSaveID", systemGetSaveID, METH_VARARGS},

									  {"GetMusicVolume", systemGetMusicVolume, METH_VARARGS},
									  {"GetSoundVolume", systemGetSoundVolume, METH_VARARGS},

									  {"SetMusicVolume", systemSetMusicVolume, METH_VARARGS},
									  {"SetSoundVolumef", systemSetSoundVolumef, METH_VARARGS},
									  {"IsSoftwareCursor", systemIsSoftwareCursor, METH_VARARGS},

									  {"SetViewChatFlag", systemSetViewChatFlag, METH_VARARGS},
									  {"IsViewChat", systemIsViewChat, METH_VARARGS},

									  {"SetAlwaysShowNameFlag", systemSetAlwaysShowNameFlag, METH_VARARGS},
									  {"IsAlwaysShowName", systemIsAlwaysShowName, METH_VARARGS},

									  {"SetShowDamageFlag", systemSetShowDamageFlag, METH_VARARGS},
									  {"IsShowDamage", systemIsShowDamage, METH_VARARGS},

#ifdef ENABLE_REFINE_RENEWAL
		{ "SetRefineStatus",			systemSetRefineStatus,			METH_VARARGS },
		{ "IsRefineShow",				systemIsRefineStatusShow,		METH_VARARGS },
#endif

									  {"SetShowSalesTextFlag", systemSetShowSalesTextFlag, METH_VARARGS},
									  {"IsShowSalesText", systemIsShowSalesText, METH_VARARGS},
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_TEXTTAIL
		{ "GetShopTextTailLevel",		systemGetShopTextTailLevel,		METH_VARARGS },
		{ "SetShopTextTailLevel",		systemSetShopTextTailLevel,		METH_VARARGS },
#endif

#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
		{ "GetShadowTargetLevel",		systemGetShadowTargetLevel,		METH_VARARGS },
		{ "SetShadowTargetLevel",		systemSetShadowTargetLevel,		METH_VARARGS },
		{ "GetShadowQualityLevel",		systemGetShadowQualityLevel,	METH_VARARGS },
		{ "SetShadowQualityLevel",		systemSetShadowQualityLevel,	METH_VARARGS },
#else
		{ "GetShadowLevel",				systemGetShadowLevel,			METH_VARARGS },
		{ "SetShadowLevel",				systemSetShadowLevel,			METH_VARARGS },
#endif

		{ "SetFogMode",					systemSetFogMode,				METH_VARARGS },
		{ "IsFogMode",					systemIsFogMode,				METH_VARARGS },

#ifdef WJ_SHOW_MOB_INFO
									  {"IsShowMobAIFlag", systemIsShowMobAIFlag, METH_VARARGS},
									  {"SetShowMobAIFlag", systemSetShowMobAIFlag, METH_VARARGS},

									  {"IsShowMobLevel", systemIsShowMobLevel, METH_VARARGS},
									  {"SetShowMobLevel", systemSetShowMobLevel, METH_VARARGS},
#endif

#ifdef ENABLE_STRUCTURE_VIEW_MODE
		{ "GetStructureViewMode",		systemGetStructureViewMode,		METH_VARARGS },
		{ "SetStructureViewMode",		systemSetStructureViewMode,		METH_VARARGS },
#endif

		{ "IsUsingItemHighlight",		systemIsUsingItemHighlight,		METH_VARARGS },
		{ "SetUsingItemHighlight",		systemSetUsingItemHighlight,	METH_VARARGS },

#ifdef ENABLE_EXTENDED_CONFIGS
		{ "IsShowMoneyLog",				systemIsShowMoneyLog,			METH_VARARGS },
		{ "SetShowMoneyLog",			systemSetShowMoneyLog,			METH_VARARGS },

		{ "SetCollectEquipment",		systemSetCollectEquipment,		METH_VARARGS },
		{ "IsCollectEquipment",			systemIsCollectEquipment,		METH_VARARGS },
#endif

#ifdef ENABLE_ENVIRONMENT_EFFECT_OPTION
		{ "SetNightModeOption",			systemSetNightModeOption,		METH_VARARGS },
		{ "GetNightModeOption",			systemGetNightModeOption,		METH_VARARGS },

		{ "SetSnowModeOption",			systemSetSnowModeOption,		METH_VARARGS },
		{ "GetSnowModeOption",			systemGetSnowModeOption,		METH_VARARGS },

		{ "SetSnowTextureModeOption",	systemSetSnowTextureModeOption,	METH_VARARGS },
		{ "GetSnowTextureModeOption",	systemGetSnowTextureModeOption,	METH_VARARGS },

# ifdef ENABLE_ENVIRONMENT_RAIN
		{ "SetRainModeOption",			systemSetRainModeOption,		METH_VARARGS },
		{ "GetRainModeOption",			systemGetRainModeOption,		METH_VARARGS },
# endif
#endif

#ifdef ENABLE_DICE_SYSTEM
		{ "SetDiceChatShow",			systemSetDiceChatShow,			METH_VARARGS },
		{ "IsDiceChatShow",				systemIsDiceChatShow,			METH_VARARGS },
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
									  { "SetShowPlayerLanguage",			systemSetShowPlayerLanguage,		METH_VARARGS },
									  { "IsShowPlayerLanguage",			systemIsShowPlayerLanguage,			METH_VARARGS },
									  { "SetShowChatLanguage",			systemSetShowChatLanguage,			METH_VARARGS },
									  { "IsShowChatLanguage",				systemIsShowChatLanguage,			METH_VARARGS },
									  { "SetShowWhisperLanguage",			systemSetShowWhisperLanguage,		METH_VARARGS },
									  { "IsShowWhisperLanguage",			systemIsShowWhisperLanguage,		METH_VARARGS },
#endif

#ifdef ENABLE_ADDITIONAL_EQUIPMENT_PAGE
									  { "GetEquipmentView",				systemGetEquipmentView,			METH_VARARGS },
									  { "SetEquipmentView",				systemSetEquipmentView,			METH_VARARGS },
#endif

									  {nullptr, nullptr, 0}};

	PyObject * poModule = Py_InitModule("systemSetting", s_methods);

	PyModule_AddIntConstant(poModule, "WINDOW_STATUS", CPythonSystem::WINDOW_STATUS);
	PyModule_AddIntConstant(poModule, "WINDOW_INVENTORY", CPythonSystem::WINDOW_INVENTORY);
	PyModule_AddIntConstant(poModule, "WINDOW_ABILITY", CPythonSystem::WINDOW_ABILITY);
	PyModule_AddIntConstant(poModule, "WINDOW_SOCIETY", CPythonSystem::WINDOW_SOCIETY);
	PyModule_AddIntConstant(poModule, "WINDOW_JOURNAL", CPythonSystem::WINDOW_JOURNAL);
	PyModule_AddIntConstant(poModule, "WINDOW_COMMAND", CPythonSystem::WINDOW_COMMAND);

	PyModule_AddIntConstant(poModule, "WINDOW_QUICK", CPythonSystem::WINDOW_QUICK);
	PyModule_AddIntConstant(poModule, "WINDOW_GAUGE", CPythonSystem::WINDOW_GAUGE);
	PyModule_AddIntConstant(poModule, "WINDOW_MINIMAP", CPythonSystem::WINDOW_MINIMAP);
	PyModule_AddIntConstant(poModule, "WINDOW_CHAT", CPythonSystem::WINDOW_CHAT);
}
