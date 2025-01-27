#include "StdAfx.h"
#include "PythonPlayer.h"

#include "InstanceBase.h"

void CPythonPlayer::SetAttackKeyState(bool isPress)
{
	if (isPress)
	{
		CInstanceBase * pkInstMain = NEW_GetMainActorPtr();
		if (pkInstMain)
		{
			if (pkInstMain->IsFishingMode())
			{
				NEW_Fishing();
				return;
			}
		}
	}

	m_isAtkKey = isPress;
}

void CPythonPlayer::NEW_SetSingleDIKKeyState(int eDIKKey, bool isPress)
{
	if (NEW_CancelFishing())
		return;

	switch (eDIKKey)
	{
		case DIK_UP:
			NEW_SetSingleDirKeyState(DIR_UP, isPress);
			break;
		case DIK_DOWN:
			NEW_SetSingleDirKeyState(DIR_DOWN, isPress);
			break;
		case DIK_LEFT:
			NEW_SetSingleDirKeyState(DIR_LEFT, isPress);
			break;
		case DIK_RIGHT:
			NEW_SetSingleDirKeyState(DIR_RIGHT, isPress);
			break;
	}
}

void CPythonPlayer::NEW_SetSingleDirKeyState(int eDirKey, bool isPress)
{
	switch (eDirKey)
	{
		case DIR_UP:
			m_isUp = isPress;
			break;
		case DIR_DOWN:
			m_isDown = isPress;
			break;
		case DIR_LEFT:
			m_isLeft = isPress;
			break;
		case DIR_RIGHT:
			m_isRight = isPress;
			break;
	}

	m_isDirKey = (m_isUp || m_isDown || m_isLeft || m_isRight);

	NEW_SetMultiDirKeyState(m_isLeft, m_isRight, m_isUp, m_isDown);
}

void CPythonPlayer::NEW_SetMultiDirKeyState(bool isLeft, bool isRight, bool isUp, bool isDown)
{
	if (!__CanMove())
		return;

	bool isAny = (isLeft || isRight || isUp || isDown);

	if (isAny)
	{
		float fDirRot = 0.0f;
		NEW_GetMultiKeyDirRotation(isLeft, isRight, isUp, isDown, &fDirRot);

		if (!NEW_MoveToDirection(fDirRot))
		{
			Tracen("CPythonPlayer::NEW_SetMultiKeyState - NEW_Move -> ERROR");
		}
	}
	else
		NEW_Stop();
}

float CPythonPlayer::GetDegreeFromDirection(int iUD, int iLR) const
{
	switch (iUD)
	{
	case KEYBOARD_UD_UP:
	{
		if (KEYBOARD_LR_LEFT == iLR)
			return +45.0f;
		if (KEYBOARD_LR_RIGHT == iLR)
			return -45.0f;
	}

		return 0.0f;

	case KEYBOARD_UD_DOWN:
	{
		if (KEYBOARD_LR_LEFT == iLR)
			return +135.0f;
		if (KEYBOARD_LR_RIGHT == iLR)
			return -135.0f;
	}

		return +180.0f;

	case KEYBOARD_UD_NONE:
	{
		if (KEYBOARD_LR_LEFT == iLR)
			return +90.0f;
		if (KEYBOARD_LR_RIGHT == iLR)
			return -90.0f;
	}
	break;
	}

	return 0.0f;
}

#ifdef ENABLE_KEYCHANGE_SYSTEM
#include "PythonApplication.h"

static void SetEmoticon(uint8_t byIndex)
{
	if (byIndex >= 1 && byIndex <= 9)
	{
		CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
		if (rkChrMgr.IsPossibleEmoticon(-1))
		{
			rkChrMgr.SetEmoticon(-1, byIndex - 1);
			CPythonNetworkStream::Instance().SendEmoticon(byIndex - 1);
		}
	}
}

void CPythonPlayer::OnKeyDown(int iKey)
{
	if (m_isOpenKeySettingWindow)
		return;

	if (CPythonApplication::Instance().IsPressed(DIK_LCONTROL))
		iKey += DIK_LCONTROL + KEY_ADDKEYBUFFERCONTROL;
	else if (CPythonApplication::Instance().IsPressed(DIK_RCONTROL))
		iKey += DIK_RCONTROL + KEY_ADDKEYBUFFERCONTROL;

	else if (CPythonApplication::Instance().IsPressed(DIK_LALT))
		iKey += DIK_LALT + KEY_ADDKEYBUFFERALT;
	else if (CPythonApplication::Instance().IsPressed(DIK_RALT))
		iKey += DIK_RALT + KEY_ADDKEYBUFFERALT;

	else if (CPythonApplication::Instance().IsPressed(DIK_LSHIFT))
		iKey += DIK_LSHIFT + KEY_ADDKEYBUFFERSHIFT;
	else if (CPythonApplication::Instance().IsPressed(DIK_RSHIFT))
		iKey += DIK_RSHIFT + KEY_ADDKEYBUFFERSHIFT;

	KeySettingMap::iterator it = m_keySettingMap.find(iKey);
	if (it != m_keySettingMap.end())
	{
		switch (it->second)
		{
			case KEY_MOVE_UP_1:
			case KEY_MOVE_UP_2:
				NEW_SetSingleDIKKeyState(DIK_UP, true);
				break;

			case KEY_MOVE_DOWN_1:
			case KEY_MOVE_DOWN_2:
				NEW_SetSingleDIKKeyState(DIK_DOWN, true);
				break;

			case KEY_MOVE_LEFT_1:
			case KEY_MOVE_LEFT_2:
				NEW_SetSingleDIKKeyState(DIK_LEFT, true);
				break;

			case KEY_MOVE_RIGHT_1:
			case KEY_MOVE_RIGHT_2:
				NEW_SetSingleDIKKeyState(DIK_RIGHT, true);
				break;

			case KEY_ATTACK:
				SetAttackKeyState(true);
				break;

			case KEY_AUTO_RUN:
				PyCallClassMemberFunc(m_ppyGameWindow, "MoveUp", Py_BuildValue("()"));
				m_bAutoRun = !m_bAutoRun;
				NEW_SetSingleDIKKeyState(0xC8, m_bAutoRun);
				break;

			case KEY_RIDEMYHORS:
				CPythonNetworkStream::Instance().SendChatPacket("/user_horse_ride");
				break;
			case KEY_FEEDMYHORS:
				CPythonNetworkStream::Instance().SendChatPacket("/user_horse_feed");
				break;

			case KEY_CAMERA_ROTATE_POSITIVE_1:
			case KEY_CAMERA_ROTATE_POSITIVE_2:
				CPythonApplication::Instance().RotateCamera(CPythonApplication::CAMERA_TO_POSITIVE);
				break;
			case KEY_CAMERA_ROTATE_NEGATIVE_1:
			case KEY_CAMERA_ROTATE_NEGATIVE_2:
				CPythonApplication::Instance().RotateCamera(CPythonApplication::CAMERA_TO_NEGITIVE);
				break;

			case KEY_CAMERA_ZOOM_POSITIVE_1:
			case KEY_CAMERA_ZOOM_POSITIVE_2:
				CPythonApplication::Instance().ZoomCamera(CPythonApplication::CAMERA_TO_POSITIVE);
				break;
			case KEY_CAMERA_ZOOM_NEGATIVE_1:
			case KEY_CAMERA_ZOOM_NEGATIVE_2:
				CPythonApplication::Instance().ZoomCamera(CPythonApplication::CAMERA_TO_NEGITIVE);
				break;

			case KEY_CAMERA_PITCH_POSITIVE_1:
			case KEY_CAMERA_PITCH_POSITIVE_2:
				CPythonApplication::Instance().PitchCamera(CPythonApplication::CAMERA_TO_POSITIVE);
				break;
			case KEY_CAMERA_PITCH_NEGATIVE_1:
			case KEY_CAMERA_PITCH_NEGATIVE_2:
				CPythonApplication::Instance().PitchCamera(CPythonApplication::CAMERA_TO_NEGITIVE);
				break;

			case KEY_ROOTING_1:
			case KEY_ROOTING_2:
				PickCloseItem();
				break;

			case KEY_EMOTION1: SetEmoticon(1); break;
			case KEY_EMOTION2: SetEmoticon(2); break;
			case KEY_EMOTION3: SetEmoticon(3); break;
			case KEY_EMOTION4: SetEmoticon(4); break;

			case KEY_RIDEHORS:
				CPythonNetworkStream::Instance().SendChatPacket("/ride");
				break;

			case KEY_BYEMYHORS:
				CPythonNetworkStream::Instance().SendChatPacket("/user_horse_back");
				break;

			case KEY_EMOTION5: SetEmoticon(5); break;
			case KEY_EMOTION6: SetEmoticon(6); break;
			case KEY_EMOTION7: SetEmoticon(7); break;
			case KEY_EMOTION8: SetEmoticon(8); break;
			case KEY_EMOTION9: SetEmoticon(9); break;

#ifdef ENABLE_TAB_TARGETING
			case KEY_NEXT_TARGET:
				if (GetWarState())
					PyCallClassMemberFunc(m_ppyGameWindow, "__PressTabKey", Py_BuildValue("()"));
				else
					SelectNearTarget();
				break;
#endif

			case KEY_SLOT_1: RequestUseLocalQuickSlot(0); break;
			case KEY_SLOT_2: RequestUseLocalQuickSlot(1); break;
			case KEY_SLOT_3: RequestUseLocalQuickSlot(2); break;
			case KEY_SLOT_4: RequestUseLocalQuickSlot(3); break;
			case KEY_SLOT_5: RequestUseLocalQuickSlot(4); break;
			case KEY_SLOT_6: RequestUseLocalQuickSlot(5); break;
			case KEY_SLOT_7: RequestUseLocalQuickSlot(6); break;
			case KEY_SLOT_8: RequestUseLocalQuickSlot(7); break;

			case KEY_SLOT_CHANGE_1: SetQuickPage(0); break;
			case KEY_SLOT_CHANGE_2: SetQuickPage(1); break;
			case KEY_SLOT_CHANGE_3: SetQuickPage(2); break;
			case KEY_SLOT_CHANGE_4: SetQuickPage(3); break;

			case KEY_OPEN_STATE:
				PyCallClassMemberFunc(m_ppyGameWindow, "OpenWindow", Py_BuildValue("(is)", KEY_OPEN_STATE, "STATUS"));
				break;

			case KEY_OPEN_SKILL:
				PyCallClassMemberFunc(m_ppyGameWindow, "OpenWindow", Py_BuildValue("(is)", KEY_OPEN_STATE, "SKILL"));
				break;

			case KEY_OPEN_QUEST:
				PyCallClassMemberFunc(m_ppyGameWindow, "OpenWindow", Py_BuildValue("(is)", KEY_OPEN_STATE, "QUEST"));
				break;

			case KEY_OPEN_INVENTORY:
				PyCallClassMemberFunc(m_ppyGameWindow, "OpenWindow", Py_BuildValue("(is)", KEY_OPEN_INVENTORY, ""));
				break;

			case KEY_OPEN_DDS:
				PyCallClassMemberFunc(m_ppyGameWindow, "OpenWindow", Py_BuildValue("(is)", KEY_OPEN_DDS, ""));
				break;

			case KEY_OPEN_MINIMAP:
				PyCallClassMemberFunc(m_ppyGameWindow, "OpenWindow", Py_BuildValue("(is)", KEY_OPEN_MINIMAP, ""));
				break;

			case KEY_OPEN_LOGCHAT:
				PyCallClassMemberFunc(m_ppyGameWindow, "OpenWindow", Py_BuildValue("(is)", KEY_OPEN_LOGCHAT, ""));
				break;

			case KEY_SCROLL_ONOFF:
				PyCallClassMemberFunc(m_ppyGameWindow, "ScrollOnOff", Py_BuildValue("()"));
				break;

			case KEY_OPEN_GUILD:
				PyCallClassMemberFunc(m_ppyGameWindow, "OpenWindow", Py_BuildValue("(is)", KEY_OPEN_GUILD, ""));
				break;

			case KEY_OPEN_MESSENGER:
				PyCallClassMemberFunc(m_ppyGameWindow, "OpenWindow", Py_BuildValue("(is)", KEY_OPEN_MESSENGER, ""));
				break;

			case KEY_OPEN_HELP:
				PyCallClassMemberFunc(m_ppyGameWindow, "OpenWindow", Py_BuildValue("(is)", KEY_OPEN_HELP, ""));
				break;

			case KEY_OPEN_ACTION:
				PyCallClassMemberFunc(m_ppyGameWindow, "OpenWindow", Py_BuildValue("(is)", KEY_OPEN_STATE, "EMOTICON"));
				break;

			case KEY_PLUS_MINIMAP:CPythonMiniMap::Instance().ScaleUp(); break;
			case KEY_MIN_MINIMAP: CPythonMiniMap::Instance().ScaleDown(); break;
			case KEY_SCREENSHOT: PyCallClassMemberFunc(m_ppyGameWindow, "SaveScreen", Py_BuildValue("()")); break;
			case KEY_SHOW_NAME: PyCallClassMemberFunc(m_ppyGameWindow, "ShowName", Py_BuildValue("()")); break;

#ifdef ENABLE_GROWTH_PET_SYSTEM
			case KEY_OPEN_PET:
				PyCallClassMemberFunc(m_ppyGameWindow, "OpenWindow", Py_BuildValue("(is)", KEY_OPEN_PET, ""));
				break;
#endif
//#ifdef ENABLE_AUTO_SYSTEM
			case KEY_OPEN_AUTO:
				PyCallClassMemberFunc(m_ppyGameWindow, "OpenWindow", Py_BuildValue("(is)", KEY_OPEN_AUTO, ""));
				break;
//#endif
#ifdef ENABLE_PARTY_MATCH
			case KEY_PARTY_MATCH:
				PyCallClassMemberFunc(m_ppyGameWindow, "OpenWindow", Py_BuildValue("(is)", KEY_PARTY_MATCH, ""));
				break;
#endif
#ifdef ENABLE_DSS_KEY_SELECT
			case KEY_SELECT_DSS_1:
				PyCallClassMemberFunc(m_ppyGameWindow, "OpenWindow", Py_BuildValue("(is)", KEY_SELECT_DSS_1, ""));
				break;
			case KEY_SELECT_DSS_2:
				PyCallClassMemberFunc(m_ppyGameWindow, "OpenWindow", Py_BuildValue("(is)", KEY_SELECT_DSS_2, ""));
				break;
#endif
		}
	}
}

void CPythonPlayer::OnKeyUp(int iKey)
{
	if (m_isOpenKeySettingWindow)
		return;

	KeySettingMap::iterator it = m_keySettingMap.find(iKey);
	if (it != m_keySettingMap.end())
	{
		switch (it->second)
		{
			case KEY_MOVE_UP_1:
			case KEY_MOVE_UP_2:
				m_bAutoRun = false;
				NEW_SetSingleDIKKeyState(DIK_UP, false);
				break;

			case KEY_MOVE_DOWN_1:
			case KEY_MOVE_DOWN_2:
				NEW_SetSingleDIKKeyState(DIK_DOWN, false);
				break;

			case KEY_MOVE_LEFT_1:
			case KEY_MOVE_LEFT_2:
				NEW_SetSingleDIKKeyState(DIK_LEFT, false);
				break;

			case KEY_MOVE_RIGHT_1:
			case KEY_MOVE_RIGHT_2:
				NEW_SetSingleDIKKeyState(DIK_RIGHT, false);
				break;

			case KEY_ATTACK:
				SetAttackKeyState(false);
				break;

			case KEY_CAMERA_ROTATE_NEGATIVE_1:
			case KEY_CAMERA_ROTATE_NEGATIVE_2:
			case KEY_CAMERA_ROTATE_POSITIVE_1:
			case KEY_CAMERA_ROTATE_POSITIVE_2:
				CPythonApplication::Instance().RotateCamera(CPythonApplication::CAMERA_STOP);
				break;

			case KEY_CAMERA_ZOOM_POSITIVE_1:
			case KEY_CAMERA_ZOOM_POSITIVE_2:
			case KEY_CAMERA_ZOOM_NEGATIVE_1:
			case KEY_CAMERA_ZOOM_NEGATIVE_2:
				CPythonApplication::Instance().ZoomCamera(CPythonApplication::CAMERA_STOP);
				break;

			case KEY_CAMERA_PITCH_POSITIVE_1:
			case KEY_CAMERA_PITCH_POSITIVE_2:
			case KEY_CAMERA_PITCH_NEGATIVE_1:
			case KEY_CAMERA_PITCH_NEGATIVE_2:
				CPythonApplication::Instance().PitchCamera(CPythonApplication::CAMERA_STOP);
				break;

			case KEY_SHOW_NAME:
				PyCallClassMemberFunc(m_ppyGameWindow, "HideName", Py_BuildValue("()"));
				break;
		}
	}
}
#endif
