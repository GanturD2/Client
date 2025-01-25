#include "StdAfx.h"
#include "PythonNetworkStream.h"
//#include "PythonNetworkDatagram.h"
#include "AccountConnector.h"
#include "PythonGuild.h"

#include "AbstractPlayer.h"

#ifdef ENABLE_GROWTH_PET_SYSTEM
#	include "PythonExchange.h"
#endif
#if defined(ENABLE_GROWTH_PET_SYSTEM) || defined(ENABLE_PRIVATESHOP_SEARCH_SYSTEM)
#	include "PythonPlayer.h"
#endif
#ifdef ENABLE_EVENT_MANAGER
#	include "PythonGameEventManager.h"
#endif

static std::string gs_stServerInfo;
extern BOOL gs_bEmpireLanuageEnable;
std::list<std::string> g_kList_strCommand;
#ifdef ENABLE_MOVE_CHANNEL
static std::map<int, std::string> g_ChannelNameMap;
static std::string gs_ServerName{};
static int gi_ChannelIndex{};
static long gl_MapIndex{};
#endif

PyObject * netGetBettingGuildWarValue(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();


	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.EXPORT_GetBettingGuildWarValue(szName));
}

PyObject * netSetServerInfo(PyObject * poSelf, PyObject * poArgs)
{
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	gs_stServerInfo = szFileName;
	return Py_BuildNone();
}

PyObject * netGetServerInfo(PyObject * poSelf, PyObject * poArgs)
{
#ifdef ENABLE_MOVE_CHANNEL
	auto it = g_ChannelNameMap.find(gi_ChannelIndex);
	if (it != g_ChannelNameMap.end())
		gs_stServerInfo = gs_ServerName + ", " + it->second;
#endif

	return Py_BuildValue("s", gs_stServerInfo.c_str());
}

PyObject * netPreserveServerCommand(PyObject * poSelf, PyObject * poArgs)
{
	char * szLine;
	if (!PyTuple_GetString(poArgs, 0, &szLine))
		return Py_BuildException();

	g_kList_strCommand.emplace_back(szLine);

	return Py_BuildNone();
}

PyObject * netGetPreservedServerCommand(PyObject * poSelf, PyObject * poArgs)
{
	if (g_kList_strCommand.empty())
		return Py_BuildValue("s", "");

	std::string strCommand = g_kList_strCommand.front();
	g_kList_strCommand.pop_front();

	return Py_BuildValue("s", strCommand.c_str());
}

PyObject * netStartGame(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.StartGame();

	return Py_BuildNone();
}

PyObject * netIsTest(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", false); // backward-compatible
}

PyObject * netWarp(PyObject * poSelf, PyObject * poArgs)
{
	int nX;
	if (!PyTuple_GetInteger(poArgs, 0, &nX))
		return Py_BuildException();

	int nY;
	if (!PyTuple_GetInteger(poArgs, 1, &nY))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.Warp(nX, nY);

	return Py_BuildNone();
}

PyObject * netLoadInsultList(PyObject * poSelf, PyObject * poArgs)
{
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.LoadInsultList(szFileName));
}

PyObject * netUploadMark(PyObject * poSelf, PyObject * poArgs)
{
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.UploadMark(szFileName));
}

PyObject * netUploadSymbol(PyObject * poSelf, PyObject * poArgs)
{
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.UploadSymbol(szFileName));
}

PyObject * netGetGuildID(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.GetGuildID());
}

PyObject * netGetEmpireID(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.GetEmpireID());
}

PyObject * netGetMainActorVID(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.GetMainActorVID());
}

PyObject * netGetMainActorRace(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.GetMainActorRace());
}

PyObject * netGetMainActorEmpire(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.GetMainActorEmpire());
}

PyObject * netGetMainActorSkillGroup(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.GetMainActorSkillGroup());
}

PyObject * netIsSelectedEmpire(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.IsSelectedEmpire());
}


PyObject * netGetAccountCharacterSlotDataInteger(PyObject * poSelf, PyObject * poArgs)
{
	int nIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &nIndex))
		return Py_BuildException();

	int nType;
	if (!PyTuple_GetInteger(poArgs, 1, &nType))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	uint32_t uResult = rkNetStream.GetAccountCharacterSlotDatau(nIndex, nType);
	return Py_BuildValue("i", uResult);
}

PyObject * netGetAccountCharacterSlotDataString(PyObject * poSelf, PyObject * poArgs)
{
	int nIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &nIndex))
		return Py_BuildException();

	int nType;
	if (!PyTuple_GetInteger(poArgs, 1, &nType))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("s", rkNetStream.GetAccountCharacterSlotDataz(nIndex, nType));
}

// SUPPORT_BGM
PyObject * netGetFieldMusicFileName(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("s", rkNetStream.GetFieldMusicFileName());
}

PyObject * netGetFieldMusicVolume(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("f", rkNetStream.GetFieldMusicVolume());
}
// END_OF_SUPPORT_BGM

PyObject * netSetPhaseWindow(PyObject * poSelf, PyObject * poArgs)
{
	int ePhaseWnd;
	if (!PyTuple_GetInteger(poArgs, 0, &ePhaseWnd))
		return Py_BuildException();

	PyObject * poPhaseWnd;
	if (!PyTuple_GetObject(poArgs, 1, &poPhaseWnd))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SetPhaseWindow(ePhaseWnd, poPhaseWnd);
	return Py_BuildNone();
}

PyObject * netClearPhaseWindow(PyObject * poSelf, PyObject * poArgs)
{
	int ePhaseWnd;
	if (!PyTuple_GetInteger(poArgs, 0, &ePhaseWnd))
		return Py_BuildException();

	PyObject * poPhaseWnd;
	if (!PyTuple_GetObject(poArgs, 1, &poPhaseWnd))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.ClearPhaseWindow(ePhaseWnd, poPhaseWnd);
	return Py_BuildNone();
}

PyObject * netSetServerCommandParserWindow(PyObject * poSelf, PyObject * poArgs)
{
	PyObject * poPhaseWnd;
	if (!PyTuple_GetObject(poArgs, 0, &poPhaseWnd))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SetServerCommandParserWindow(poPhaseWnd);
	return Py_BuildNone();
}

PyObject * netSetAccountConnectorHandler(PyObject * poSelf, PyObject * poArgs)
{
	PyObject * poPhaseWnd;
	if (!PyTuple_GetObject(poArgs, 0, &poPhaseWnd))
		return Py_BuildException();

	CAccountConnector & rkAccountConnector = CAccountConnector::Instance();
	rkAccountConnector.SetHandler(poPhaseWnd);
	return Py_BuildNone();
}

PyObject * netSetHandler(PyObject * poSelf, PyObject * poArgs)
{
	PyObject * poHandler;

	if (!PyTuple_GetObject(poArgs, 0, &poHandler))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SetHandler(poHandler);
	return Py_BuildNone();
}

PyObject * netSetTCPRecvBufferSize(PyObject * poSelf, PyObject * poArgs)
{
	int bufSize;
	if (!PyTuple_GetInteger(poArgs, 0, &bufSize))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SetRecvBufferSize(bufSize);
	return Py_BuildNone();
}

PyObject * netSetTCPSendBufferSize(PyObject * poSelf, PyObject * poArgs)
{
	int bufSize;
	if (!PyTuple_GetInteger(poArgs, 0, &bufSize))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SetSendBufferSize(bufSize);
	return Py_BuildNone();
}

PyObject * netSetUDPRecvBufferSize(PyObject * poSelf, PyObject * poArgs)
{
	int bufSize;
	if (!PyTuple_GetInteger(poArgs, 0, &bufSize))
		return Py_BuildException();

	//CPythonNetworkDatagram::Instance().SetRecvBufferSize(bufSize);
	return Py_BuildNone();
}

PyObject * netSetMarkServer(PyObject * poSelf, PyObject * poArgs)
{
	char * szAddr;
	if (!PyTuple_GetString(poArgs, 0, &szAddr))
		return Py_BuildException();

	int port;
	if (!PyTuple_GetInteger(poArgs, 1, &port))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SetMarkServer(szAddr, port);
	return Py_BuildNone();
}

PyObject * netConnectTCP(PyObject * poSelf, PyObject * poArgs)
{
	char * szAddr;
	if (!PyTuple_GetString(poArgs, 0, &szAddr))
		return Py_BuildException();

	int port;
	if (!PyTuple_GetInteger(poArgs, 1, &port))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.ConnectLoginServer(szAddr, port);
	return Py_BuildNone();
}

PyObject * netConnectUDP(PyObject * poSelf, PyObject * poArgs)
{
	char * c_szIP;
	if (!PyTuple_GetString(poArgs, 0, &c_szIP))
		return Py_BuildException();
	int iPort;
	if (!PyTuple_GetInteger(poArgs, 1, &iPort))
		return Py_BuildException();

	//CPythonNetworkDatagram::Instance().SetConnection(c_szIP, iPort);
	return Py_BuildNone();
}

PyObject * netConnectToAccountServer(PyObject * poSelf, PyObject * poArgs)
{
	char * addr;
	if (!PyTuple_GetString(poArgs, 0, &addr))
		return Py_BuildException();

	int port;
	if (!PyTuple_GetInteger(poArgs, 1, &port))
		return Py_BuildException();

	char * account_addr;
	if (!PyTuple_GetString(poArgs, 2, &account_addr))
		return Py_BuildException();

	int account_port;
	if (!PyTuple_GetInteger(poArgs, 3, &account_port))
		return Py_BuildException();

	CAccountConnector & rkAccountConnector = CAccountConnector::Instance();
	rkAccountConnector.Connect(addr, port, account_addr, account_port);
	return Py_BuildNone();
}

#ifdef ENABLE_SEND_TARGET_INFO
PyObject* netTargetInfoLoad(PyObject* poSelf, PyObject* poArgs)
{
	uint32_t dwVID;

	if (!PyArg_ParseTuple(poArgs, "i", &dwVID))
	{
		return Py_BuildException();
	}
	if (dwVID < 0)
	{
		return Py_BuildNone();
	}

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendTargetInfoLoadPacket(dwVID);

	return Py_BuildNone();
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

PyObject * netSetLoginInfo(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	char * szPwd;
	if (!PyTuple_GetString(poArgs, 1, &szPwd))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	CAccountConnector & rkAccountConnector = CAccountConnector::Instance();
	rkNetStream.SetLoginInfo(szName, szPwd);
	rkAccountConnector.SetLoginInfo(szName, szPwd);
	return Py_BuildNone();
}

PyObject * netSetOfflinePhase(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SetOffLinePhase();
	return Py_BuildNone();
}

PyObject * netSendSelectEmpirePacket(PyObject * poSelf, PyObject * poArgs)
{
	int iEmpireIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iEmpireIndex))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendSelectEmpirePacket(iEmpireIndex);
	return Py_BuildNone();
}

PyObject * netSendLoginPacket(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	char * szPwd;
	if (!PyTuple_GetString(poArgs, 1, &szPwd))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendLoginPacket(szName, szPwd);
	return Py_BuildNone();
}

PyObject * netSendChinaMatrixCardPacket(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildNone();
}

PyObject * netSendRunupMatrixCardPacket(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildNone();
}

PyObject * netSendNEWCIBNPasspodAnswerPacket(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildNone();
}


PyObject * netDirectEnter(PyObject * poSelf, PyObject * poArgs)
{
	int nChrSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &nChrSlot))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.ConnectGameServer(nChrSlot);
	return Py_BuildNone();
}

PyObject * netSendSelectCharacterPacket(PyObject * poSelf, PyObject * poArgs)
{
	int Index;
	if (!PyTuple_GetInteger(poArgs, 0, &Index))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendSelectCharacterPacket(static_cast<uint8_t>(Index));
	return Py_BuildNone();
}

PyObject * netSendChangeNamePacket(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();
	char * szName;
	if (!PyTuple_GetString(poArgs, 1, &szName))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendChangeNamePacket(static_cast<uint8_t>(iIndex), szName);
	return Py_BuildNone();
}

PyObject * netEnableChatInsultFilter(PyObject * poSelf, PyObject * poArgs)
{
	int nEnable;
	if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.EnableChatInsultFilter(nEnable ? true : false);
	return Py_BuildNone();
}

PyObject * netIsChatInsultIn(PyObject * poSelf, PyObject * poArgs)
{
	char * szMsg;
	if (!PyTuple_GetString(poArgs, 0, &szMsg))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.IsChatInsultIn(szMsg));
}


PyObject * netIsInsultIn(PyObject * poSelf, PyObject * poArgs)
{
	char * szMsg;
	if (!PyTuple_GetString(poArgs, 0, &szMsg))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.IsInsultIn(szMsg));
}

PyObject * netSendWhisperPacket(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	char * szLine;

	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	if (!PyTuple_GetString(poArgs, 1, &szLine))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendWhisperPacket(szName, szLine);
	return Py_BuildNone();
}

PyObject * netSendMobileMessagePacket(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildNone();
}

PyObject * netSendCharacterPositionPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iPosition;
	if (!PyTuple_GetInteger(poArgs, 0, &iPosition))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendCharacterPositionPacket(static_cast<uint8_t>(iPosition));
	return Py_BuildNone();
}

PyObject * netSendChatPacket(PyObject * poSelf, PyObject * poArgs)
{
	char * szLine;
	if (!PyTuple_GetString(poArgs, 0, &szLine))
		return Py_BuildException();
	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		iType = CHAT_TYPE_TALKING;

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendChatPacket(szLine, iType);
	return Py_BuildNone();
}

PyObject * netSendEmoticon(PyObject * poSelf, PyObject * poArgs)
{
	int eEmoticon;
	if (!PyTuple_GetInteger(poArgs, 0, &eEmoticon))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendEmoticon(eEmoticon);
	return Py_BuildNone();
}

PyObject * netSendCreateCharacterPacket(PyObject * poSelf, PyObject * poArgs)
{
	int index;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	char * name;
	if (!PyTuple_GetString(poArgs, 1, &name))
		return Py_BuildException();

	int job;
	if (!PyTuple_GetInteger(poArgs, 2, &job))
		return Py_BuildException();

	int shape;
	if (!PyTuple_GetInteger(poArgs, 3, &shape))
		return Py_BuildException();

	int stat1;
	if (!PyTuple_GetInteger(poArgs, 4, &stat1))
		return Py_BuildException();
	int stat2;
	if (!PyTuple_GetInteger(poArgs, 5, &stat2))
		return Py_BuildException();
	int stat3;
	if (!PyTuple_GetInteger(poArgs, 6, &stat3))
		return Py_BuildException();
	int stat4;
	if (!PyTuple_GetInteger(poArgs, 7, &stat4))
		return Py_BuildException();

	if (index < 0 && index > 3)
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendCreateCharacterPacket(static_cast<uint8_t>(index), name, static_cast<uint8_t>(job), static_cast<uint8_t>(shape), stat1,
										  stat2, stat3, stat4);
	return Py_BuildNone();
}

PyObject * netSendDestroyCharacterPacket(PyObject * poSelf, PyObject * poArgs)
{
	int index;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	char * szPrivateCode;
	if (!PyTuple_GetString(poArgs, 1, &szPrivateCode))
		return Py_BuildException();

	if (index < 0 && index > 3)
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendDestroyCharacterPacket(static_cast<uint8_t>(index), szPrivateCode);
	return Py_BuildNone();
}

PyObject * netSendEnterGamePacket(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendEnterGame();
	return Py_BuildNone();
}

PyObject * netOnClickPacket(PyObject * poSelf, PyObject * poArgs)
{
	int index;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendOnClickPacket(index);

	return Py_BuildNone();
}

PyObject * netSendItemUsePacket(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos Cell;
	switch (PyTuple_Size(poArgs))
	{
	case 1:
		if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
			return Py_BuildException();
		break;
	case 2:
		if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendItemUsePacket(Cell);
	return Py_BuildNone();
}

PyObject * netSendItemUseToItemPacket(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos SourceCell;
	TItemPos TargetCell;
	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &SourceCell.cell))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 1, &TargetCell.cell))
			return Py_BuildException();
		break;
	case 4:
		if (!PyTuple_GetByte(poArgs, 0, &SourceCell.window_type))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 1, &SourceCell.cell))
			return Py_BuildException();

		if (!PyTuple_GetByte(poArgs, 2, &TargetCell.window_type))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 3, &TargetCell.cell))
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendItemUseToItemPacket(SourceCell, TargetCell);
	return Py_BuildNone();
}

PyObject * netSendItemDropPacket(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos Cell;
	switch (PyTuple_Size(poArgs))
	{
	case 1:
		if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
			return Py_BuildException();
		break;
	case 2:
		if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}


	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendItemDropPacket(Cell, 0);
	return Py_BuildNone();
}

#ifdef ENABLE_DESTROY_SYSTEM
PyObject* netSendItemDestroyPacket(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos Cell;
	int count;
	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &count))
			return Py_BuildException();

		break;
	case 3:
		if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &count))
			return Py_BuildException();

		break;
	default:
		return Py_BuildException();
	}
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendItemDestroyPacket(Cell, 0, count);
	return Py_BuildNone();
}
#endif

PyObject * netSendItemDropPacketNew(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos Cell;
	int count;
	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &count))
			return Py_BuildException();

		break;
	case 3:
		if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &count))
			return Py_BuildException();

		break;
	default:
		return Py_BuildException();
	}
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendItemDropPacketNew(Cell, 0, count);
	return Py_BuildNone();
}

PyObject * netSendElkDropPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iElk;
	if (!PyTuple_GetInteger(poArgs, 0, &iElk))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendItemDropPacket(TItemPos(RESERVED_WINDOW, 0), static_cast<uint32_t>(iElk));
	return Py_BuildNone();
}

PyObject * netSendGoldDropPacketNew(PyObject * poSelf, PyObject * poArgs)
{
	int iElk;
	if (!PyTuple_GetInteger(poArgs, 0, &iElk))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendItemDropPacketNew(TItemPos(RESERVED_WINDOW, 0), static_cast<uint32_t>(iElk), 0);
	return Py_BuildNone();
}

#if defined(ENABLE_CHEQUE_SYSTEM) && defined(ENABLE_CHEQUE_EXCHANGE_WINDOW)
PyObject* netSendWonExchangeSellPacket(PyObject* poSelf, PyObject* poArgs)
{
	uint16_t wValue = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &wValue))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendWonExchangeSellPacket(wValue);
	return Py_BuildNone();
}

PyObject* netSendWonExchangeBuyPacket(PyObject* poSelf, PyObject* poArgs)
{
	uint16_t wValue = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &wValue))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendWonExchangeBuyPacket(wValue);
	return Py_BuildNone();
}
#endif

PyObject * netSendItemMovePacket(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos Cell;
	TItemPos ChangeCell;
	int num;

	switch (PyTuple_Size(poArgs))
	{
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &ChangeCell.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &num))
			return Py_BuildException();
		break;
	case 5:
	{
		if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BuildException();
		if (!PyTuple_GetByte(poArgs, 2, &ChangeCell.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 3, &ChangeCell.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 4, &num))
			return Py_BuildException();
	}
	break;
	default:
		return Py_BuildException();
	}

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendItemMovePacket(Cell, ChangeCell, static_cast<uint8_t>(num));
	return Py_BuildNone();
}

PyObject * netSendItemPickUpPacket(PyObject * poSelf, PyObject * poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendItemPickUpPacket(vid);
	return Py_BuildNone();
}

#ifdef ENABLE_SUMMER_EVENT_ROULETTE
PyObject* netSoulRoulettePacket(PyObject* poSelf, PyObject* poArgs)
{
	int option;
	if (!PyTuple_GetInteger(poArgs, 0, &option))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SoulRoulette(option);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_MINI_GAME_BNW
PyObject* netSendMiniGameBNW(PyObject* poSelf, PyObject* poArgs)
{
	int iSubHeader;
	if (!PyTuple_GetInteger(poArgs, 0, &iSubHeader))
		return Py_BuildException();

	int iSubArgument;
	if (!PyTuple_GetInteger(poArgs, 1, &iSubArgument))
		CPythonNetworkStream::Instance().SendMiniGameBNW(iSubHeader, 0);
	else
		CPythonNetworkStream::Instance().SendMiniGameBNW(iSubHeader, iSubArgument);

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_MINI_GAME_FINDM
PyObject* netSendMiniGameFindM(PyObject* poSelf, PyObject* poArgs)
{
	int iSubHeader;
	if (!PyTuple_GetInteger(poArgs, 0, &iSubHeader))
		return Py_BuildException();

	int iSubArgument;
	if (!PyTuple_GetInteger(poArgs, 1, &iSubArgument))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendMiniGameFindM(iSubHeader, iSubArgument);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_MINI_GAME_YUTNORI
PyObject* netSendMiniGameYutNori(PyObject* poSelf, PyObject* poArgs)
{
	int iSubHeader;
	if (!PyTuple_GetInteger(poArgs, 0, &iSubHeader))
		return Py_BuildException();

	int iSubArgument;
	if (!PyTuple_GetInteger(poArgs, 1, &iSubArgument))
		CPythonNetworkStream::Instance().SendMiniGameYutNori(iSubHeader, 0);
	else
		CPythonNetworkStream::Instance().SendMiniGameYutNori(iSubHeader, iSubArgument);

	return Py_BuildNone();
}
#endif

PyObject * netSendGiveItemPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iTargetVID;
	TItemPos Cell;
	int iItemCount;
	switch (PyTuple_Size(poArgs))
	{
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &iTargetVID))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &iItemCount))
			return Py_BuildException();
		break;
	case 4:
		if (!PyTuple_GetInteger(poArgs, 0, &iTargetVID))
			return Py_BuildException();
		if (!PyTuple_GetByte(poArgs, 1, &Cell.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &Cell.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 3, &iItemCount))
			return Py_BuildException();
		break;
	default:
		break;
	}

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendGiveItemPacket(iTargetVID, Cell, iItemCount);
	return Py_BuildNone();
}

PyObject * netSendShopEndPacket(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendShopEndPacket();
	return Py_BuildNone();
}

PyObject * netSendShopBuyPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iCount;
	if (!PyTuple_GetInteger(poArgs, 0, &iCount))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendShopBuyPacket(iCount);
	return Py_BuildNone();
}

PyObject * netSendShopSellPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotNumber))
		return Py_BuildException();
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendShopSellPacket(iSlotNumber);
	return Py_BuildNone();
}

PyObject * netSendShopSellPacketNew(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotNumber))
		return Py_BuildException();
	int iCount;
	if (!PyTuple_GetInteger(poArgs, 1, &iCount))
		return Py_BuildException();
#ifdef ENABLE_REVERSED_FUNCTIONS
	int iType;
	if (!PyTuple_GetInteger(poArgs, 2, &iType))
		return Py_BuildException();
#endif
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendShopSellPacketNew(iSlotNumber, iCount);
	return Py_BuildNone();
}

PyObject * netSendExchangeStartPacket(PyObject * poSelf, PyObject * poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendExchangeStartPacket(vid);
	return Py_BuildNone();
}

PyObject * netSendExchangeElkAddPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iElk;
	if (!PyTuple_GetInteger(poArgs, 0, &iElk))
		return Py_BuildException();

#ifdef ENABLE_CHEQUE_SYSTEM
	int iCheque;
	if (!PyTuple_GetInteger(poArgs, 1, &iCheque))
		return Py_BuildException();
#endif

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
#ifdef ENABLE_CHEQUE_SYSTEM
	rkNetStream.SendExchangeElkAddPacket(iElk, iCheque);
#else
	rkNetStream.SendExchangeElkAddPacket(iElk);
#endif
	return Py_BuildNone();
}

PyObject * netSendExchangeItemAddPacket(PyObject * poSelf, PyObject * poArgs)
{
	uint8_t bWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bWindowType))
		return Py_BuildException();
	uint16_t wSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &wSlotIndex))
		return Py_BuildException();
	int iDisplaySlotIndex;
	if (!PyTuple_GetInteger(poArgs, 2, &iDisplaySlotIndex))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendExchangeItemAddPacket(TItemPos(bWindowType, wSlotIndex), iDisplaySlotIndex);
	return Py_BuildNone();
}

PyObject * netSendExchangeItemDelPacket(PyObject * poSelf, PyObject * poArgs)
{
	int pos;
	if (!PyTuple_GetInteger(poArgs, 0, &pos))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendExchangeItemDelPacket(static_cast<uint8_t>(pos));
	return Py_BuildNone();
}

PyObject * netSendExchangeAcceptPacket(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendExchangeAcceptPacket();
	return Py_BuildNone();
}

PyObject * netSendExchangeExitPacket(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendExchangeExitPacket();
	return Py_BuildNone();
}

PyObject * netExitApplication(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.ExitApplication();
	return Py_BuildNone();
}

PyObject * netExitGame(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.ExitGame();
	return Py_BuildNone();
}

PyObject * netLogOutGame(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.LogOutGame();
	return Py_BuildNone();
}

PyObject * netDisconnect(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SetOffLinePhase();
	rkNetStream.Disconnect();

	return Py_BuildNone();
}

PyObject * netIsConnect(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.IsOnline());
}

PyObject * netToggleGameDebugInfo(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.ToggleGameDebugInfo();
	return Py_BuildNone();
}

PyObject * netRegisterEmoticonString(PyObject * poSelf, PyObject * poArgs)
{
	char * pcEmoticonString;
	if (!PyTuple_GetString(poArgs, 0, &pcEmoticonString))
		return Py_BuildException();

	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.RegisterEmoticonString(pcEmoticonString);
	return Py_BuildNone();
}

#ifdef ENABLE_MESSENGER_BLOCK
PyObject* netSendMessengerBlockAddByVIDPacket(PyObject* poSelf, PyObject* poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendMessengerBlockAddByVIDPacket(vid);

	return Py_BuildNone();
}

PyObject* netSendMessengerBlockAddByNamePacket(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendMessengerBlockAddByNamePacket(szName);

	return Py_BuildNone();
}

PyObject* netSendMessengerBlockRemovePacket(PyObject* poSelf, PyObject* poArgs)
{
	char* szKey;
	if (!PyTuple_GetString(poArgs, 0, &szKey))
		return Py_BuildException();

	char* szName;
	if (!PyTuple_GetString(poArgs, 1, &szName))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendMessengerBlockRemovePacket(szKey, szName);

	return Py_BuildNone();
}

PyObject* netSendMessengerBlockRemoveByVIDPacket(PyObject* poSelf, PyObject* poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendMessengerBlockRemoveByVIDPacket(vid);

	return Py_BuildNone();
}
#endif

PyObject * netSendMessengerAddByVIDPacket(PyObject * poSelf, PyObject * poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendMessengerAddByVIDPacket(vid);

	return Py_BuildNone();
}

PyObject * netSendMessengerAddByNamePacket(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendMessengerAddByNamePacket(szName);

	return Py_BuildNone();
}

PyObject * netSendMessengerRemovePacket(PyObject * poSelf, PyObject * poArgs)
{
	char * szKey;
	if (!PyTuple_GetString(poArgs, 0, &szKey))
		return Py_BuildException();
	char * szName;
	if (!PyTuple_GetString(poArgs, 1, &szName))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendMessengerRemovePacket(szKey, szName);

	return Py_BuildNone();
}

PyObject * netSendPartyInvitePacket(PyObject * poSelf, PyObject * poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendPartyInvitePacket(vid);

	return Py_BuildNone();
}

PyObject * netSendPartyInviteAnswerPacket(PyObject * poSelf, PyObject * poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();
	int answer;
	if (!PyTuple_GetInteger(poArgs, 1, &answer))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendPartyInviteAnswerPacket(vid, answer);

	return Py_BuildNone();
}

PyObject * netSendPartyExitPacket(PyObject * poSelf, PyObject * poArgs)
{
	IAbstractPlayer & rPlayer = IAbstractPlayer::GetSingleton();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();

	uint32_t dwVID = rPlayer.GetMainCharacterIndex();
	uint32_t dwPID;
	if (rPlayer.PartyMemberVIDToPID(dwVID, &dwPID))
		rns.SendPartyRemovePacket(dwPID);

	return Py_BuildNone();
}

PyObject* netSendPartyRemovePacket(PyObject* poSelf, PyObject* poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendPartyRemovePacket(vid);

	return Py_BuildNone();
}

PyObject * netSendPartyRemovePacketPID(PyObject * poSelf, PyObject * poArgs)
{
	int pid;
	if (!PyTuple_GetInteger(poArgs, 0, &pid))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendPartyRemovePacket(pid);

	return Py_BuildNone();
}

PyObject* netSendPartyRemovePacketVID(PyObject* poSelf, PyObject* poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	IAbstractPlayer & rPlayer = IAbstractPlayer::GetSingleton();
	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	uint32_t dwPID;
	if (rPlayer.PartyMemberVIDToPID(vid, &dwPID))
		rns.SendPartyRemovePacket(dwPID);

	return Py_BuildNone();
}

PyObject * netSendPartySetStatePacket(PyObject * poSelf, PyObject * poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();
	int iState;
	if (!PyTuple_GetInteger(poArgs, 1, &iState))
		return Py_BuildException();
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 2, &iFlag))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendPartySetStatePacket(iVID, iState, iFlag);

	return Py_BuildNone();
}

PyObject * netSendPartyUseSkillPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BuildException();
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 1, &iVID))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendPartyUseSkillPacket(iSkillIndex, iVID);

	return Py_BuildNone();
}

PyObject * netSendPartyParameterPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iMode;
	if (!PyTuple_GetInteger(poArgs, 0, &iMode))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendPartyParameterPacket(iMode);

	return Py_BuildNone();
}

#ifdef ENABLE_SAFEBOX_MONEY
PyObject * netSendSafeboxSaveMoneyPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iMoney;
	if (!PyTuple_GetInteger(poArgs, 0, &iMoney))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendSafeBoxMoneyPacket(SAFEBOX_MONEY_STATE_SAVE, iMoney);

	return Py_BuildNone();
}

PyObject * netSendSafeboxWithdrawMoneyPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iMoney;
	if (!PyTuple_GetInteger(poArgs, 0, &iMoney))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendSafeBoxMoneyPacket(SAFEBOX_MONEY_STATE_WITHDRAW, iMoney);

	return Py_BuildNone();
}
#endif

PyObject * netSendSafeboxCheckinPacket(PyObject * poSelf, PyObject * poArgs)
{
	TItemPos InventoryPos;
	int iSafeBoxPos;

	switch (PyTuple_Size(poArgs))
	{
	case 2:
		InventoryPos.window_type = INVENTORY;
		if (!PyTuple_GetInteger(poArgs, 0, &InventoryPos.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &iSafeBoxPos))
			return Py_BuildException();
		break;
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &InventoryPos.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &InventoryPos.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &iSafeBoxPos))
			return Py_BuildException();
		break;
	}

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendSafeBoxCheckinPacket(InventoryPos, iSafeBoxPos);

	return Py_BuildNone();
}

PyObject * netSendSafeboxCheckoutPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iSafeBoxPos;
	TItemPos InventoryPos;

	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &iSafeBoxPos))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &InventoryPos.cell))
			return Py_BuildException();
		break;
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &iSafeBoxPos))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &InventoryPos.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &InventoryPos.cell))
			return Py_BuildException();
		break;
	default:
		break;
	}

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendSafeBoxCheckoutPacket(iSafeBoxPos, InventoryPos);

	return Py_BuildNone();
}

PyObject * netSendSafeboxItemMovePacket(PyObject * poSelf, PyObject * poArgs)
{
	int iSourcePos;
	if (!PyTuple_GetInteger(poArgs, 0, &iSourcePos))
		return Py_BuildException();
	int iTargetPos;
	if (!PyTuple_GetInteger(poArgs, 1, &iTargetPos))
		return Py_BuildException();
	int iCount;
	if (!PyTuple_GetInteger(poArgs, 2, &iCount))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendSafeBoxItemMovePacket(iSourcePos, iTargetPos, iCount);

	return Py_BuildNone();
}

PyObject * netSendMallCheckoutPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iMallPos;
	TItemPos InventoryPos;

	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &iMallPos))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &InventoryPos.cell))
			return Py_BuildException();
		break;
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &iMallPos))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &InventoryPos.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &InventoryPos.cell))
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}
	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendMallCheckoutPacket(iMallPos, InventoryPos);

	return Py_BuildNone();
}

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
PyObject* netSendGuildstorageCheckinPacket(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos InventoryPos;
	int iGuildstoragePos;

	switch (PyTuple_Size(poArgs))
	{
		case 2:
			InventoryPos.window_type = INVENTORY;
			if (!PyTuple_GetInteger(poArgs, 0, &InventoryPos.cell))
				return Py_BuildException();
			if (!PyTuple_GetInteger(poArgs, 1, &iGuildstoragePos))
				return Py_BuildException();
			break;

		case 3:
			if (!PyTuple_GetInteger(poArgs, 0, &InventoryPos.window_type))
				return Py_BuildException();
			if (!PyTuple_GetInteger(poArgs, 1, &InventoryPos.cell))
				return Py_BuildException();
			if (!PyTuple_GetInteger(poArgs, 2, &iGuildstoragePos))
				return Py_BuildException();
			break;
	}

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendGuildstorageCheckinPacket(InventoryPos, iGuildstoragePos);

	return Py_BuildNone();
}

PyObject* netSendGuildstorageCheckoutPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iGuildstoragePos = 0;
	TItemPos InventoryPos;

	switch (PyTuple_Size(poArgs))
	{
		case 2:
			if (!PyTuple_GetInteger(poArgs, 0, &iGuildstoragePos))
				return Py_BuildException();
			if (!PyTuple_GetInteger(poArgs, 1, &InventoryPos.cell))
				return Py_BuildException();
			break;

		case 3:
			if (!PyTuple_GetInteger(poArgs, 0, &iGuildstoragePos))
				return Py_BuildException();
			if (!PyTuple_GetInteger(poArgs, 1, &InventoryPos.window_type))
				return Py_BuildException();
			if (!PyTuple_GetInteger(poArgs, 2, &InventoryPos.cell))
				return Py_BuildException();
			break;

		default:
			return Py_BuildException();
	}
	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendGuildstorageCheckoutPacket(iGuildstoragePos, InventoryPos);

	return Py_BuildNone();
}

PyObject* netSendGuildBankMove(PyObject* poSelf, PyObject* poArgs)
{
	int iSourcePos;
	if (!PyTuple_GetInteger(poArgs, 0, &iSourcePos))
		return Py_BuildException();
	int iTargetPos;
	if (!PyTuple_GetInteger(poArgs, 1, &iTargetPos))
		return Py_BuildException();
	int iCount;
	if (!PyTuple_GetInteger(poArgs, 2, &iCount))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendSafeBoxItemMovePacket(iSourcePos, iTargetPos, iCount);

	return Py_BuildNone();
}
#endif

PyObject * netSendAnswerMakeGuildPacket(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendAnswerMakeGuildPacket(szName);

	return Py_BuildNone();
}

PyObject * netSendQuestInputStringPacket(PyObject * poSelf, PyObject * poArgs)
{
	char * szString;
	if (!PyTuple_GetString(poArgs, 0, &szString))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendQuestInputStringPacket(szString);

	return Py_BuildNone();
}

#ifdef ENABLE_OX_RENEWAL
PyObject* netSendQuestInputStringLongPacket(PyObject* poSelf, PyObject* poArgs)
{
	char * szString;
	if (!PyTuple_GetString(poArgs, 0, &szString))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendQuestInputStringLongPacket(szString);

	return Py_BuildNone();
}
#endif

PyObject * netSendQuestConfirmPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iAnswer;
	if (!PyTuple_GetInteger(poArgs, 0, &iAnswer))
		return Py_BuildException();
	int iPID;
	if (!PyTuple_GetInteger(poArgs, 1, &iPID))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendQuestConfirmPacket(iAnswer, iPID);

	return Py_BuildNone();
}

PyObject * netSendGuildAddMemberPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendGuildAddMemberPacket(iVID);

	return Py_BuildNone();
}

PyObject * netSendGuildRemoveMemberPacket(PyObject * poSelf, PyObject * poArgs)
{
	char * szKey;
	if (!PyTuple_GetString(poArgs, 0, &szKey))
		return Py_BuildException();

	CPythonGuild::TGuildMemberData * pGuildMemberData;
	if (!CPythonGuild::Instance().GetMemberDataPtrByName(szKey, &pGuildMemberData))
	{
		TraceError("netSendGuildRemoveMemberPacket(szKey=%s) - Can't Find Guild Member\n", szKey);
		return Py_BuildNone();
	}

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendGuildRemoveMemberPacket(pGuildMemberData->dwPID);

	return Py_BuildNone();
}

#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_EXTENDED_RENEWAL_FEATURES)
PyObject* netSendGuildVoteMemberOut(PyObject* poSelf, PyObject* poArgs)
{
	char* szKey;
	if (!PyTuple_GetString(poArgs, 0, &szKey))
		return Py_BuildException();

	CPythonGuild::TGuildMemberData* pGuildMemberData;
	if (!CPythonGuild::Instance().GetMemberDataPtrByName(szKey, &pGuildMemberData))
	{
		TraceError("netSendGuildRemoveMemberPacket(szKey=%s) - Can't Find Guild Member\n", szKey);
		return Py_BuildNone();
	}

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendGuildRemoveMemberPacket(pGuildMemberData->dwPID);

	return Py_BuildNone();
}

PyObject* netSendGuildVoteChangeMaster(PyObject* poSelf, PyObject* poArgs)
{
	char* szKey;
	if (!PyTuple_GetString(poArgs, 0, &szKey))
		return Py_BuildException();

	CPythonGuild::TGuildMemberData* pGuildMemberData;
	if (!CPythonGuild::Instance().GetMemberDataPtrByName(szKey, &pGuildMemberData))
	{
		TraceError("netSendGuildVoteChangeMaster(szKey=%s) - Can't Find Guild Member\n", szKey);
		return Py_BuildNone();
	}

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendGuildVoteChangeMaster(pGuildMemberData->dwPID);

	return Py_BuildNone();
}

PyObject* netSendGuildVoteLandAbndon(PyObject* poSelf, PyObject* poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendGuildVoteLandAbndon(iVID);

	return Py_BuildNone();
}
#endif

PyObject * netSendGuildChangeGradeNamePacket(PyObject * poSelf, PyObject * poArgs)
{
	int iGradeNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &iGradeNumber))
		return Py_BuildException();
	char * szGradeName;
	if (!PyTuple_GetString(poArgs, 1, &szGradeName))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendGuildChangeGradeNamePacket(iGradeNumber, szGradeName);

	return Py_BuildNone();
}

PyObject * netSendGuildChangeGradeAuthorityPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iGradeNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &iGradeNumber))
		return Py_BuildException();
	int iAuthority;
	if (!PyTuple_GetInteger(poArgs, 1, &iAuthority))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendGuildChangeGradeAuthorityPacket(iGradeNumber, iAuthority);

	return Py_BuildNone();
}

PyObject * netSendGuildOfferPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iExperience;
	if (!PyTuple_GetInteger(poArgs, 0, &iExperience))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendGuildOfferPacket(iExperience);

	return Py_BuildNone();
}

PyObject * netSnedGuildPostCommentPacket(PyObject * poSelf, PyObject * poArgs)
{
	char * szComment;
	if (!PyTuple_GetString(poArgs, 0, &szComment))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendGuildPostCommentPacket(szComment);

	return Py_BuildNone();
}

PyObject * netSnedGuildDeleteCommentPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendGuildDeleteCommentPacket(iIndex);

	return Py_BuildNone();
}

PyObject * netSendGuildRefreshCommentsPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iHightestIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iHightestIndex))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendGuildRefreshCommentsPacket(iHightestIndex);

	return Py_BuildNone();
}

PyObject * netSendGuildChangeMemberGradePacket(PyObject * poSelf, PyObject * poArgs)
{
	int iPID;
	if (!PyTuple_GetInteger(poArgs, 0, &iPID))
		return Py_BuildException();
	int iGradeNumber;
	if (!PyTuple_GetInteger(poArgs, 1, &iGradeNumber))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendGuildChangeMemberGradePacket(iPID, iGradeNumber);

	return Py_BuildNone();
}

PyObject * netSendGuildUseSkillPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iSkillID;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillID))
		return Py_BuildException();
	int iTargetVID;
	if (!PyTuple_GetInteger(poArgs, 1, &iTargetVID))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendGuildUseSkillPacket(iSkillID, iTargetVID);

	return Py_BuildNone();
}

PyObject * netSendGuildChangeMemberGeneralPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iPID;
	if (!PyTuple_GetInteger(poArgs, 0, &iPID))
		return Py_BuildException();
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 1, &iFlag))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendGuildChangeMemberGeneralPacket(iPID, iFlag);

	return Py_BuildNone();
}

PyObject * netSendGuildInviteAnswerPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iGuildID;
	if (!PyTuple_GetInteger(poArgs, 0, &iGuildID))
		return Py_BuildException();
	int iAnswer;
	if (!PyTuple_GetInteger(poArgs, 1, &iAnswer))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendGuildInviteAnswerPacket(iGuildID, iAnswer);

	return Py_BuildNone();
}

PyObject * netSendGuildChargeGSPPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iGSP;
	if (!PyTuple_GetInteger(poArgs, 0, &iGSP))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendGuildChargeGSPPacket(iGSP);

	return Py_BuildNone();
}

PyObject * netSendGuildDepositMoneyPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iGSP;
	if (!PyTuple_GetInteger(poArgs, 0, &iGSP))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendGuildDepositMoneyPacket(iGSP);

	return Py_BuildNone();
}

PyObject * netSendGuildWithdrawMoneyPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iGSP;
	if (!PyTuple_GetInteger(poArgs, 0, &iGSP))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendGuildWithdrawMoneyPacket(iGSP);

	return Py_BuildNone();
}

PyObject * netSendRequestRefineInfoPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	//	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	//	rns.SendRequestRefineInfoPacket(iSlotIndex);
	assert(!"netSendRequestRefineInfoPacket - This function is not used anymore");

	return Py_BuildNone();
}

PyObject * netSendRefinePacket(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();
	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendRefinePacket(iSlotIndex, iType);

	return Py_BuildNone();
}

PyObject * netSendSelectItemPacket(PyObject * poSelf, PyObject * poArgs)
{
	int iItemPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemPos))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.SendSelectItemPacket(iItemPos);

	return Py_BuildNone();
}

PyObject * netSetPacketSequenceMode(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	CAccountConnector & rkAccountConnector = CAccountConnector::Instance();
	rns.SetPacketSequenceMode(true);
	rkAccountConnector.SetPacketSequenceMode(true);

	return Py_BuildNone();
}

PyObject * netSetEmpireLanguageMode(PyObject * poSelf, PyObject * poArgs)
{
	int iMode;
	if (!PyTuple_GetInteger(poArgs, 0, &iMode))
		return Py_BuildException();

	//CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	gs_bEmpireLanuageEnable = iMode;

	return Py_BuildNone();
}

PyObject * netSetSkillGroupFake(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	CPythonNetworkStream & rns = CPythonNetworkStream::Instance();
	rns.__TEST_SetSkillGroupFake(iIndex);

	return Py_BuildNone();
}

#include "GuildMarkUploader.h"
#include "GuildMarkDownloader.h"

PyObject * netSendGuildSymbol(PyObject * poSelf, PyObject * poArgs)
{
	char * szIP;
	if (!PyTuple_GetString(poArgs, 0, &szIP))
		return Py_BuildException();
	int iPort;
	if (!PyTuple_GetInteger(poArgs, 1, &iPort))
		return Py_BuildException();
	char * szFileName;
	if (!PyTuple_GetString(poArgs, 2, &szFileName))
		return Py_BuildException();
	int iGuildID;
	if (!PyTuple_GetInteger(poArgs, 3, &iGuildID))
		return Py_BuildException();

	CNetworkAddress kAddress;
	kAddress.Set(szIP, iPort);

	uint32_t uiError;

	CGuildMarkUploader & rkGuildMarkUploader = CGuildMarkUploader::Instance();
	if (!rkGuildMarkUploader.ConnectToSendSymbol(kAddress, 0, 0, iGuildID, szFileName, &uiError))
		assert(!"Failed connecting to send symbol");

	return Py_BuildNone();
}

PyObject * netDisconnectUploader(PyObject * poSelf, PyObject * poArgs)
{
	CGuildMarkUploader & rkGuildMarkUploader = CGuildMarkUploader::Instance();
	rkGuildMarkUploader.Disconnect();
	return Py_BuildNone();
}

PyObject * netRecvGuildSymbol(PyObject * poSelf, PyObject * poArgs)
{
	char * szIP;
	if (!PyTuple_GetString(poArgs, 0, &szIP))
		return Py_BuildException();
	int iPort;
	if (!PyTuple_GetInteger(poArgs, 1, &iPort))
		return Py_BuildException();
	int iGuildID;
	if (!PyTuple_GetInteger(poArgs, 2, &iGuildID))
		return Py_BuildException();

	CNetworkAddress kAddress;
	kAddress.Set(szIP, iPort);

	std::vector<uint32_t> kVec_dwGuildID;
	kVec_dwGuildID.clear();
	kVec_dwGuildID.emplace_back(iGuildID);

	// @fixme006
	if (!kVec_dwGuildID.empty())
	{
		CGuildMarkDownloader & rkGuildMarkDownloader = CGuildMarkDownloader::Instance();
		if (!rkGuildMarkDownloader.ConnectToRecvSymbol(kAddress, 0, 0, kVec_dwGuildID))
			assert(!"Failed connecting to recv symbol");
	}

	return Py_BuildNone();
}

PyObject * netRegisterErrorLog(PyObject * poSelf, PyObject * poArgs)
{
	char * szLog;
	if (!PyTuple_GetString(poArgs, 0, &szLog))
		return Py_BuildException();

	return Py_BuildNone();
}

#ifdef ENABLE_GROWTH_PET_SYSTEM
PyObject* netCheckUsePetItem(PyObject* poSelf, PyObject* poArgs)
{
	// isTrading
	if (CPythonExchange::Instance().isTrading())
		return Py_BuildValue("i", CItemData::PET_EGG_USE_FAILED_BECAUSE_TRADING);

	// IsOpenPrivateShop
	if (CPythonPlayer::Instance().IsOpenPrivateShop())
		return Py_BuildValue("i", CItemData::PET_EGG_USE_FAILED_BECAUSE_SHOP_OPEN);

	// IsOpenMall
	if (CPythonPlayer::Instance().IsOpenMall())
		return Py_BuildValue("i", CItemData::PET_EGG_USE_FAILED_BECAUSE_MALL_OPEN);

	// IsOpenSafeBox
	if (CPythonPlayer::Instance().IsOpenSafeBox())
		return Py_BuildValue("i", CItemData::PET_EGG_USE_FAILED_BECAUSE_SAFEBOX_OPEN);

	return Py_BuildValue("i", 0);
}

PyObject* netSendPetWindowType(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t pet_window_type;
	if (!PyTuple_GetInteger(poArgs, 0, &pet_window_type))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendPetWindowType(pet_window_type);

	return Py_BuildNone();
}

PyObject* netSendPetHatchingWindowPacket(PyObject* poSelf, PyObject* poArgs)
{
	bool bisOpenPetHatchingWindow = false;
	if (!PyTuple_GetBoolean(poArgs, 0, &bisOpenPetHatchingWindow))
		return Py_BadArgument();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendPetHatchingWindowPacket(bisOpenPetHatchingWindow);

	return Py_BuildNone();
}

PyObject* netSendPetHatchingPacket(PyObject* poSelf, PyObject* poArgs)
{
	char* sGrowthPetName;
	if (!PyTuple_GetString(poArgs, 0, &sGrowthPetName))
		return Py_BuildException();

	TItemPos Cell;
	if (!PyTuple_GetByte(poArgs, 1, &Cell.window_type))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 2, &Cell.cell))
		return Py_BuildException();

	if (strlen(sGrowthPetName) < CItemData::PET_NAME_MIN_SIZE || strlen(sGrowthPetName) > CItemData::PET_NAME_MAX_SIZE)
	{
		TraceError("PetHatchingPacket: Name lenght not allowed!");
		return Py_BuildNone();
	}

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendPetHatchingPacket(sGrowthPetName, Cell.cell);

	return Py_BuildNone();
}

PyObject* netSendPetLearnSkill(PyObject* poSelf, PyObject* poArgs)
{
	int SkillBookSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &SkillBookSlotIndex))
		return Py_BuildException();

	TItemPos Cell;
	if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendPetLearnSkillPacket(SkillBookSlotIndex, Cell.cell);

	return Py_BuildNone();
}

PyObject* netSendPetSkillUpgrade(PyObject* poSelf, PyObject* poArgs)
{
	int slotPos;
	if (!PyTuple_GetInteger(poArgs, 0, &slotPos))
		return Py_BuildException();

	int slotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &slotIndex))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendPetSkillUpgradePacket(slotPos, slotIndex);

	return Py_BuildNone();
}

PyObject* netSendPetSkillUpgradeRequest(PyObject* poSelf, PyObject* poArgs)
{
	int slotPos;
	if (!PyTuple_GetInteger(poArgs, 0, &slotPos))
		return Py_BuildException();

	int slotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &slotIndex))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendPetSkillUpgradeRequestPacket(slotPos, slotIndex);

	return Py_BuildNone();
}

PyObject* netSendPetFeedPacket(PyObject* poSelf, PyObject* poArgs)
{
	int feedIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &feedIndex))
		return Py_BuildException();

	PyObject* resultFeedItems;
	if (!PyTuple_GetObject(poArgs, 1, &resultFeedItems))
		return Py_BuildException();

	PyObject* resultFeedItemCounts;
	if (!PyTuple_GetObject(poArgs, 2, &resultFeedItemCounts))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendPetFeedPacket(feedIndex, resultFeedItems, resultFeedItemCounts);

	return Py_BuildNone();
}

PyObject* netSendPetDeleteSkill(PyObject* poSelf, PyObject* poArgs)
{
	int SkillBookDelSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &SkillBookDelSlotIndex))
		return Py_BuildException();

	int SkillBookDelInvenIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &SkillBookDelInvenIndex))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendPetDeleteSkill(SkillBookDelSlotIndex, SkillBookDelInvenIndex);

	return Py_BuildNone();
}

PyObject* netSendPetDeleteAllSkill(PyObject* poSelf, PyObject* poArgs)
{
	int PetSkillAllDelBookIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &PetSkillAllDelBookIndex))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendPetDeleteAllSkill(PetSkillAllDelBookIndex);

	return Py_BuildNone();
}

PyObject* netSendPetNameChangePacket(PyObject* poSelf, PyObject* poArgs)
{
	/*
		petNameEdit.GetText(),
		nameChangeItemSlotWindow, // playerm2g2.INVENTORY
		nameChangeItemSlotIndex,
		petItemSlotWindow,
		petItemSlotIndex
	*/
	char* sPetName;
	if (!PyTuple_GetString(poArgs, 0, &sPetName))
		return Py_BuildException();

	TItemPos itemCell;
	if (!PyTuple_GetByte(poArgs, 1, &itemCell.window_type))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 2, &itemCell.cell))
		return Py_BuildException();

	TItemPos petCell;
	if (!PyTuple_GetByte(poArgs, 3, &petCell.window_type))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 4, &petCell.cell))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendPetNameChange(sPetName, itemCell.window_type, itemCell.cell, petCell.window_type, petCell.cell);

	return Py_BuildNone();
}

PyObject* netSendPetNameChangeWindowPacket(PyObject* poSelf, PyObject* poArgs)
{
	// SendPetNameChangeWindowPacket
	return Py_BuildNone();
}

#ifdef ENABLE_PET_ATTR_DETERMINE
PyObject* netSendPetAttrDetermine(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendPetAttrDetermine();

	return Py_BuildNone();
}

PyObject* netSendChangePetPacket(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendChangePetPacket(CPythonPlayer::Instance().GetAttrChangeWindowSlotByAttachedInvenSlot(0),
		CPythonPlayer::Instance().GetAttrChangeWindowSlotByAttachedInvenSlot(1));

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
PyObject* netSendPetRevive(PyObject* poSelf, PyObject* poArgs)
{
	int iUpBringingPos;
	PyObject* pyPosList;
	PyObject* pyCountList;
	std::vector<std::pair<uint16_t, uint16_t>> itemVec;

	if (!PyArg_ParseTuple(poArgs, "iOO", &iUpBringingPos, &pyPosList, &pyCountList))
		return Py_BuildException();

	for (int i = 0; i < PyList_Size(pyPosList); ++i)
	{
		PyObject* pyItemPos = PyList_GetItem(pyPosList, i);
		uint16_t itemPos = static_cast<uint16_t>(_PyInt_AsInt(pyItemPos));

		PyObject* pyItemCount = PyList_GetItem(pyCountList, i);
		uint16_t itemCount = static_cast<uint16_t>(_PyInt_AsInt(pyItemCount));

		itemVec.push_back(std::make_pair(itemPos, itemCount));
	}

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendPetRevive(TItemPos(INVENTORY, iUpBringingPos), itemVec);

	return Py_BuildNone();
}
#endif
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
PyObject* netSendLanguageChangePacket(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bLanguage;
	if (!PyTuple_GetInteger(poArgs, 0, &bLanguage))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendLanguageChangePacket(bLanguage);

	return Py_BuildNone();
}

PyObject* netSendTargetLanguageRequestPacket(PyObject* poSelf, PyObject* poArgs)
{
	char* c_szName;
	if (!PyTuple_GetString(poArgs, 0, &c_szName))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendTargetLanguageRequestPacket(c_szName);

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
PyObject* netSendPrivateShopSearchInfo(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bRace;
	if (!PyTuple_GetInteger(poArgs, 0, &bRace))
		return Py_BuildException();
	uint8_t bMaskType;
	if (!PyTuple_GetInteger(poArgs, 1, &bMaskType))
		return Py_BuildException();
	uint8_t bMaskSubType;
	if (!PyTuple_GetInteger(poArgs, 2, &bMaskSubType))
		return Py_BuildException();
	uint8_t bMinRefine;
	if (!PyTuple_GetInteger(poArgs, 3, &bMinRefine))
		return Py_BuildException();
	uint8_t bMaxRefine;
	if (!PyTuple_GetInteger(poArgs, 4, &bMaxRefine))
		return Py_BuildException();
	uint8_t bMinLevel;
	if (!PyTuple_GetInteger(poArgs, 5, &bMinLevel))
		return Py_BuildException();
	uint8_t bMaxLevel;
	if (!PyTuple_GetInteger(poArgs, 6, &bMaxLevel))
		return Py_BuildException();
	int iMinGold;
	if (!PyTuple_GetInteger(poArgs, 7, &iMinGold))
		return Py_BuildException();
	int iMaxGold;
	if (!PyTuple_GetInteger(poArgs, 8, &iMaxGold))
		return Py_BuildException();
	char* itemName;
	if (!PyTuple_GetString(poArgs, 9, &itemName))
		return Py_BuildException();
#	ifdef ENABLE_CHEQUE_SYSTEM
	int dwMinCheque;
	if (!PyTuple_GetInteger(poArgs, 10, &dwMinCheque))
		return Py_BuildException();
	int dwMaxCheque;
	if (!PyTuple_GetInteger(poArgs, 11, &dwMaxCheque))
		return Py_BuildException();
#	endif

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendPrivateShopSearchInfo(bRace, bMaskType, bMaskSubType, bMinRefine, bMaxRefine, bMinLevel, bMaxLevel, iMinGold, iMaxGold, itemName
#	ifdef ENABLE_CHEQUE_SYSTEM
		, dwMinCheque, dwMaxCheque
#	endif
	);

	return Py_BuildNone();
}

PyObject* netSendPrivateShopSearchInfoSub(PyObject* poSelf, PyObject* poArgs)
{
	int iPage;
	if (!PyTuple_GetInteger(poArgs, 0, &iPage))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendPrivateShopSearchInfoSub(iPage);
	return Py_BuildNone();
}

PyObject* netSendPrivateShopSerchBuyItem(PyObject* poSelf, PyObject* poArgs)
{
	int selectitemRealindex;
	if (!PyTuple_GetInteger(poArgs, 0, &selectitemRealindex))
		return Py_BuildException();

	const TSearchItemData pItemData = CPythonPlayer::Instance().GetShopSearchItemData(selectitemRealindex);
	if (!pItemData.vid)
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendPrivateShopSearchBuyItem(pItemData.vid, pItemData.Cell.cell);

	return Py_BuildNone();
}

PyObject* netClosePrivateShopSearchWindow(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendPrivateShopSearchClose();
	return Py_BuildNone();
}
#endif

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
PyObject* netSendGemShopBuy(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendGemShopBuy(iSlotIndex);

	return Py_BuildNone();
}
PyObject* netSendSlotAdd(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendSlotAdd();
	return Py_BuildNone();
}
PyObject* netSendRequestRefresh(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendRequestRefresh();
	return Py_BuildNone();
}

PyObject* netSendSelectmetinstone(PyObject* poSelf, PyObject* poArgs)
{
	int iItemPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemPos))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendSelectmetinstone(iItemPos);

	return Py_BuildNone();
}

// Missing
PyObject* netSendGemShopClose(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_LOADING_TIP
PyObject* netSetMapTarget(PyObject* poSelf, PyObject* poArgs)
{
	long lMapTarget;
	if (!PyTuple_GetLong(poArgs, 0, &lMapTarget))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SetMapTarget(lMapTarget);
	return Py_BuildNone();
}

PyObject* netGetMapTarget(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.GetMapTarget());
}
#endif

#ifdef ENABLE_MOVE_CHANNEL
PyObject* netSetChannelName(PyObject* poSelf, PyObject* poArgs)
{
	switch (PyTuple_Size(poArgs))
	{
		case 1:
		{
			int iChannelIndex;
			if (!PyTuple_GetInteger(poArgs, 0, &iChannelIndex))
				return Py_BuildException();

			gi_ChannelIndex = iChannelIndex;
		}
		break;

		case 2:
		{
			int iChannelIndex;
			if (!PyTuple_GetInteger(poArgs, 0, &iChannelIndex))
				return Py_BuildException();

			char* szChannelName;
			if (!PyTuple_GetString(poArgs, 1, &szChannelName))
				return Py_BuildException();

			g_ChannelNameMap[iChannelIndex] = szChannelName;
		}
		break;

		default:
			return Py_BuildException();
	}

	return Py_BuildNone();
}

PyObject* netGetChannelName(PyObject* poSelf, PyObject* poArgs)
{
	int iChannelIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iChannelIndex))
		return Py_BuildException();

	auto it = g_ChannelNameMap.find(iChannelIndex);
	if (it != g_ChannelNameMap.end())
		return Py_BuildValue("s", it->second.c_str());

	return Py_BuildValue("s", "");
}

PyObject* netGetChannelNumber(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", gi_ChannelIndex);
}

PyObject* netGetChannelCount(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", g_ChannelNameMap.size());
}

PyObject* netSetMapIndex(PyObject* poSelf, PyObject* poArgs)
{
	long lMapIndex;
	if (!PyTuple_GetLong(poArgs, 0, &lMapIndex))
		return Py_BuildException();

	gl_MapIndex = lMapIndex;

	return Py_BuildNone();
}

PyObject* netGetMapIndex(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", gl_MapIndex);
}

PyObject* netSetServerName(PyObject* poSelf, PyObject* poArgs)
{
	char* szServerName;
	if (!PyTuple_GetString(poArgs, 0, &szServerName))
		return Py_BuildException();

	gs_ServerName = szServerName;
	return Py_BuildNone();
}

PyObject* netClearServerInfo(PyObject* poSelf, PyObject* poArgs)
{
	g_ChannelNameMap.clear();

	return Py_BuildNone();
}

PyObject* netMoveChannelGame(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bChannelIndex;
	if (!PyTuple_GetByte(poArgs, 0, &bChannelIndex))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendMoveChannelPacket(bChannelIndex);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
PyObject* netSendItemCombinationPacket(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bSlotMedium;
	if (!PyTuple_GetInteger(poArgs, 0, &bSlotMedium))
		return Py_BuildException();

	uint8_t bSlotBase;
	if (!PyTuple_GetInteger(poArgs, 1, &bSlotBase))
		return Py_BuildException();

	uint8_t bSlotMaterial;
	if (!PyTuple_GetInteger(poArgs, 2, &bSlotMaterial))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendItemCombinationPacket(bSlotMedium, bSlotBase, bSlotMaterial);

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_GUILDBANK_LOG
PyObject* netGuildBankInfoOpen(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.GuildBankInfoOpen();

	return Py_BuildNone();
}

PyObject* netSendGuildBankInfoOpen(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendGuildBankInfoOpen();

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
PyObject* netGuildDonateInfoOpen(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.GuildDonateInfoOpen();

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_ATTR_6TH_7TH
PyObject* netSendSkillBookCombinationPacket(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* cell;
	if (!PyTuple_GetObject(poArgs, 0, &cell))
		return Py_BuildException();

	uint8_t bIndexComb;
	if (!PyTuple_GetInteger(poArgs, 1, &bIndexComb))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendSkillBookCombinationPacket(cell, bIndexComb);
	return Py_BuildNone();
}

PyObject* netSendRegistFragmentPacket(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bRegistSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &bRegistSlot))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendRegistFragmentPacket(bRegistSlot);
	return Py_BuildNone();
}

PyObject* netSendAttr67ClosePacket(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bIndexComb;
	if (!PyTuple_GetInteger(poArgs, 0, &bIndexComb))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendAttr67ClosePacket(bIndexComb);
	return Py_BuildNone();
}

PyObject* netSendAttr67AddPacket(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bRegistSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &bRegistSlot))
		return Py_BuildException();

	uint8_t bFragmentCount;
	if (!PyTuple_GetInteger(poArgs, 1, &bFragmentCount))
		return Py_BuildException();

	uint16_t wCellAdditive;
	if (!PyTuple_GetInteger(poArgs, 2, &wCellAdditive))
		return Py_BuildException();

	uint8_t bCountAdditive;
	if (!PyTuple_GetInteger(poArgs, 3, &bCountAdditive))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendAttr67AddPacket(bRegistSlot, bFragmentCount, wCellAdditive, bCountAdditive);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
PyObject* netSendRemoveItemFromMyShop(PyObject* poSelf, PyObject* poArgs)
{
	int slot;
	if (!PyTuple_GetInteger(poArgs, 0, &slot))
		return Py_BuildException();

	uint8_t window;
	if (!PyTuple_GetByte(poArgs, 1, &window))
		return Py_BuildException();

	int target;
	if (!PyTuple_GetInteger(poArgs, 2, &target))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendRemoveFromMyShop(slot, TItemPos(window, target));

	return Py_BuildNone();
}

PyObject* netSendAddItemToMyShop(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t window;
	if (!PyTuple_GetByte(poArgs, 0, &window))
		return Py_BuildException();

	int slot;
	if (!PyTuple_GetInteger(poArgs, 1, &slot))
		return Py_BuildException();

	int targetPos;
	if (!PyTuple_GetInteger(poArgs, 2, &targetPos))
		return Py_BuildException();

	int price;
	if (!PyTuple_GetInteger(poArgs, 3, &price))
		return Py_BuildException();

#	ifdef ENABLE_CHEQUE_SYSTEM
	int cheque;
	if (!PyTuple_GetInteger(poArgs, 4, &cheque))
		return Py_BuildException();
#	endif

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
#	ifdef ENABLE_CHEQUE_SYSTEM
	rns.SendAddToMyShop(TItemPos(window, slot), targetPos, price, cheque);
#	else
	rns.SendAddToMyShop(TItemPos(window, slot), targetPos, price);
#	endif

	return Py_BuildNone();
}

PyObject* netSendCloseMyShop(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.CloseMyShop();

	return Py_BuildNone();
}

PyObject* netSendOpenMyShop(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.OpenMyShop();

	return Py_BuildNone();
}

#	ifdef ENABLE_PREMIUM_PRIVATE_SHOP_EXPIRE
PyObject* netSendReopenMyShop(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.ReopenMyShop();

	return Py_BuildNone();
}
#	endif

PyObject* netSendWithdrawMyShopMoney(PyObject* poSelf, PyObject* poArgs)
{

	int goldAmount;
	if (!PyTuple_GetInteger(poArgs, 0, &goldAmount))
		return Py_BuildException();

#	ifdef ENABLE_CHEQUE_SYSTEM
	int chequeAmount;
	if (!PyTuple_GetInteger(poArgs, 1, &chequeAmount))
		return Py_BuildException();
#	endif

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
#	ifdef ENABLE_CHEQUE_SYSTEM
	rns.WithdrawMyShopMoney(goldAmount, chequeAmount);
#	else
	rns.WithdrawMyShopMoney(goldAmount);
#	endif

	return Py_BuildNone();
}

PyObject* netSendRenameMyShop(PyObject* poSelf, PyObject* poArgs)
{
	char* newName;
	if (!PyTuple_GetString(poArgs, 0, &newName))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.RenameMyShop(newName);

	return Py_BuildNone();
}

#ifdef ENABLE_MYSHOP_DECO
PyObject* netSendMyShopDecoState(PyObject* poSelf, PyObject* poArgs)
{
	int decostate;
	if (!PyTuple_GetInteger(poArgs, 0, &decostate))
		return Py_BadArgument();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendMyShopDecoState(decostate);
	return Py_BuildNone();
}

PyObject* netSendMyShopDecoSet(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bTitleType;
	if (!PyTuple_GetByte(poArgs, 0, &bTitleType))
		return Py_BuildException();

	int PolyVnum;
	if (!PyTuple_GetInteger(poArgs, 1, &PolyVnum))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendMyShopDecoSet(bTitleType, PolyVnum);
	return Py_BuildNone();
}
#endif
#endif

#ifdef ENABLE_REFINE_ELEMENT
PyObject* netSendElementsSpellClose(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendElementsSpellClose();
	return Py_BuildNone();
}

PyObject* netSendElementsSpellAdd(PyObject* poSelf, PyObject* poArgs)
{
	int pos;
	if (!PyTuple_GetInteger(poArgs, 0, &pos))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendElementsSpellAdd(pos);
	return Py_BuildNone();
}

PyObject* netSendElementsSpellChange(PyObject* poSelf, PyObject* poArgs)
{
	int pos;
	if (!PyTuple_GetInteger(poArgs, 0, &pos))
		return Py_BuildException();

	int type_select;
	if (!PyTuple_GetInteger(poArgs, 1, &type_select))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendElementsSpellChange(pos, type_select);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
PyObject* netSendExtendInvenButtonClick(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bStepIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &bStepIndex))
		return Py_BuildException();

# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	uint8_t bWindow;
	if (!PyTuple_GetInteger(poArgs, 1, &bWindow))
		return Py_BuildException();
# endif

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	rkNetStream.SendExtendInvenRequest(bStepIndex, bWindow);
# else
	rkNetStream.SendExtendInvenRequest(bStepIndex);
# endif

	return Py_BuildNone();
}

PyObject* netSendExtendInvenUpgrade(PyObject* poSelf, PyObject* poArgs)
{
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	uint8_t bWindow;
	if (!PyTuple_GetInteger(poArgs, 0, &bWindow))
		return Py_BuildException();
# endif

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
# ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE_SPECIAL_INV
	rkNetStream.SendExtendInvenUpgrade(bWindow);
# else
	rkNetStream.SendExtendInvenUpgrade();
# endif

	return Py_BuildNone();
}
#endif

#ifdef NEW_SELECT_CHARACTER
PyObject* netGetLoginID(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	return Py_BuildValue("s", rkNetStream.GetLoginID());
}
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
#include "PythonPlayer.h"
PyObject* netSendAcceRefineCheckIn(PyObject* poSelf, PyObject* poArgs)
{
	int attachedInvenType;
	if (!PyTuple_GetInteger(poArgs, 0, &attachedInvenType))
		return Py_BuildException();
	int attachedSlotPos;
	if (!PyTuple_GetInteger(poArgs, 1, &attachedSlotPos))
		return Py_BuildException();
	int selectedSlotPos;
	if (!PyTuple_GetInteger(poArgs, 2, &selectedSlotPos))
		return Py_BuildException();
	int windowType;
	if (!PyTuple_GetInteger(poArgs, 3, &windowType))
		return Py_BuildException();

	// Packet
	//CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	//rkNetStream.SendAcceAddPacket(TItemPos(windowType, attachedSlotPos), selectedSlotPos);

	return Py_BuildNone();
}

PyObject* netSendAcceRefineCheckOut(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bAcceSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &bAcceSlot))
		return Py_BuildException();

	if (bAcceSlot < 0 || bAcceSlot > 1)
		return Py_BuildNone();

	if (bAcceSlot == 0)
	{
		for (unsigned int i = 0; i < 3; ++i)
		{
			CPythonPlayer::Instance().ClearAcceActivedItemSlot(i);
		}
		return Py_BuildNone();
	}

	CPythonPlayer::Instance().ClearAcceActivedItemSlot(bAcceSlot);
	return Py_BuildNone();
}

PyObject* netSendAcceRefineCanCle(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendAcceClosePacket();
	return Py_BuildNone();
}

PyObject* netSendAcceRefineAccept(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bWindowType))
		return Py_BuildException();

	if (bWindowType < 0 || bWindowType > 1)
		return Py_BuildNone();

	uint8_t bAcceSlot = CPythonPlayer::Instance().GetAcceActivedItemSlot(0);
	uint8_t bAcceMaterial = CPythonPlayer::Instance().GetAcceActivedItemSlot(1);

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendAcceRefinePacket(bWindowType, bAcceSlot, bAcceMaterial);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_AURA_SYSTEM
PyObject* netSendAuraRefineCheckIn(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos InventoryPos;
	TItemPos AuraPos;
	uint8_t bAuraRefineWindowType;
	switch (PyTuple_Size(poArgs))
	{
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &InventoryPos.cell))
			return Py_BuildException();
		AuraPos.window_type = AURA_REFINE;
		if (!PyTuple_GetInteger(poArgs, 1, &AuraPos.cell))
			return Py_BuildException();
		if (!PyTuple_GetByte(poArgs, 2, &bAuraRefineWindowType))
			return Py_BuildException();
		break;
	case 5:
		if (!PyTuple_GetByte(poArgs, 0, &InventoryPos.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &InventoryPos.cell))
			return Py_BuildException();
		if (!PyTuple_GetByte(poArgs, 2, &AuraPos.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 3, &AuraPos.cell))
			return Py_BuildException();
		if (!PyTuple_GetByte(poArgs, 4, &bAuraRefineWindowType))
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendAuraRefineCheckIn(InventoryPos, AuraPos, bAuraRefineWindowType);
	return Py_BuildNone();
}

PyObject* netSendAuraRefineCheckOut(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos AuraPos;
	uint8_t byAuraRefineWindowType;
	switch (PyTuple_Size(poArgs))
	{
	case 2:
		AuraPos.window_type = AURA_REFINE;
		if (!PyTuple_GetInteger(poArgs, 0, &AuraPos.cell))
			return Py_BuildException();
		if (!PyTuple_GetByte(poArgs, 1, &byAuraRefineWindowType))
			return Py_BuildException();
		break;
	case 3:
		if (!PyTuple_GetByte(poArgs, 0, &AuraPos.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &AuraPos.cell))
			return Py_BuildException();
		if (!PyTuple_GetByte(poArgs, 2, &byAuraRefineWindowType))
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendAuraRefineCheckOut(AuraPos, byAuraRefineWindowType);
	return Py_BuildNone();
}

PyObject* netSendAuraRefineAccept(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t byAuraRefineWindowType;
	if (!PyTuple_GetByte(poArgs, 0, &byAuraRefineWindowType))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendAuraRefineAccept(byAuraRefineWindowType);
	return Py_BuildNone();
}

PyObject* netSendAuraRefineCancel(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendAuraRefineCancel();
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_BINARY_SERVERINFO
static const char* ServerIPVName[1][2] = {
	{"127.0.0.1", 	"Localhost"},
};

static unsigned int ServerPORT[] = {
	{30002},	//ch1
	{30012},	//ch2
	{30022},	//ch3
	{30032},	//ch4
	{30001}		//auth
};

PyObject* netGetServer_IPNAME(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("ss", ServerIPVName[0][0], ServerIPVName[0][1]);
}

PyObject* netGetServer_PORT(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("iiiii", ServerPORT[0], ServerPORT[1], ServerPORT[2], ServerPORT[3], ServerPORT[4]);
}
#endif

#ifdef ENABLE_HWID_BAN
PyObject* netSendHwidBanPacket(PyObject* poSelf, PyObject* poArgs)
{
	int mode;
	if (!PyTuple_GetInteger(poArgs, 0, &mode))
		return Py_BuildException();

	char* szPlayer;
	if (!PyTuple_GetString(poArgs, 1, &szPlayer))
		return Py_BuildException();

	char* szReason;
	if (!PyTuple_GetString(poArgs, 2, &szReason))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendHwidBanPacket(mode, szPlayer, szReason);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_CHANGED_ATTR
PyObject* netSendChagedItemAttributePacket(PyObject* poSelf, PyObject* poArgs)
{
	bool bNew;
	if (!PyTuple_GetBoolean(poArgs, 0, &bNew))
		return Py_BuildException();

	TItemPos Cell;
	if (!PyTuple_GetByte(poArgs, 1, &Cell.window_type))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 2, &Cell.cell))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendChagedItemAttributePacket(bNew, Cell);

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_EVENTS
#	ifdef ENABLE_MINI_GAME_OKEY_NORMAL
PyObject* netSendMiniGameRumiStart(PyObject* poSelf, PyObject* poArgs)
{
	int safemode;
	if (!PyTuple_GetInteger(poArgs, 0, &safemode))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendMiniGameRumiStart(safemode);
	return Py_BuildNone();
}

PyObject* netSendMiniGameRumiExit(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendMiniGameRumiExit();
	return Py_BuildNone();
}

PyObject* netSendMiniGameRumiDeckCardClick(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendMiniGameRumiDeckCardClick();
	return Py_BuildNone();
}

PyObject* netSendMiniGameRumiHandCardClick(PyObject* poSelf, PyObject* poArgs)
{
	int index;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendMiniGameRumiHandCardClick(index);
	return Py_BuildNone();
}

PyObject* netSendMiniGameRumiFieldCardClick(PyObject* poSelf, PyObject* poArgs)
{
	int index;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendMiniGameRumiFieldCardClick(index);
	return Py_BuildNone();
}

PyObject* netSendMiniGameRumiDestroy(PyObject* poSelf, PyObject* poArgs)
{
	int index;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendMiniGameRumiDestroy(index);
	return Py_BuildNone();
}
#	endif	// ENABLE_MINI_GAME_OKEY_NORMAL

#	ifdef ENABLE_MINI_GAME_CATCH_KING
PyObject* netSendMiniGameCatchKingStart(PyObject* poSelf, PyObject* poArgs)
{
	int ibetNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &ibetNumber))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendMiniGameCatchKingStart(ibetNumber);
	return Py_BuildNone();
}

PyObject* netSendMiniGameCatchKingDeckCardClick(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendMiniGameCatchKingDeckCardClick();
	return Py_BuildNone();
}

PyObject* netSendMiniGameCatchKingFieldCardClick(PyObject* poSelf, PyObject* poArgs)
{
	int icardNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &icardNumber))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendMiniGameCatchKingFieldCardClick(icardNumber);
	return Py_BuildNone();
}

PyObject* netSendMiniGameCatchKingReward(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendMiniGameCatchKingReward();
	return Py_BuildNone();
}
#	endif	// ENABLE_MINI_GAME_CATCH_KING

#	ifdef ENABLE_MONSTER_BACK
PyObject* netSendAttendanceGetReward(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendAttendanceGetReward();
	return Py_BuildNone();
}
#	endif	// ENABLE_MONSTER_BACK

#	ifdef ENABLE_FISH_EVENT
PyObject* netSendFishBoxUse(PyObject* poSelf, PyObject* poArgs)
{
	int iWindow;
	if (!PyTuple_GetInteger(poArgs, 0, &iWindow))
		return Py_BuildException();

	int iCell;
	if (!PyTuple_GetInteger(poArgs, 1, &iCell))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendFishBoxUse(iWindow, iCell);
	return Py_BuildNone();
}

PyObject* netSendFishShapeAdd(PyObject* poSelf, PyObject* poArgs)
{
	int iShapePos;
	if (!PyTuple_GetInteger(poArgs, 0, &iShapePos))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendFishShapeAdd(iShapePos);

	return Py_BuildNone();
}

PyObject* netSendRequestFishEventBlock(PyObject* poSelf, PyObject* poArgs)
{
	// CPythonNetworkStream::Instance().SendFishingPacketNew(0, 3);
	return Py_BuildNone();
}
#	endif	// ENABLE_FISH_EVENT
#endif	// ENABLE_EVENTS

#ifdef ENABLE_MAILBOX
PyObject* netSendPostWriteConfirm(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendPostWriteConfirm(szName);
	return Py_BuildNone();
}

PyObject* netSendMailBoxClose(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendMailBoxClose();
	return Py_BuildNone();
}

PyObject* netSendPostAllDelete(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendPostAllDelete();
	return Py_BuildNone();
}

PyObject* netSendPostAllGetItems(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendPostAllGetItems();
	return Py_BuildNone();
}

PyObject* netSendPostDelete(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t index;
	if (!PyTuple_GetByte(poArgs, 0, &index))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendPostDelete(index);
	return Py_BuildNone();
}

PyObject* netSendPostGetItems(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t index;
	if (!PyTuple_GetByte(poArgs, 0, &index))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendPostGetItems(index);
	return Py_BuildNone();
}

PyObject* netSendPostWrite(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t Idx = 0;

	char* szName;
	if (!PyTuple_GetString(poArgs, Idx++, &szName))
		return Py_BuildException();

	char* szTitle;
	if (!PyTuple_GetString(poArgs, Idx++, &szTitle))
		return Py_BuildException();

	char* szMessage;
	if (!PyTuple_GetString(poArgs, Idx++, &szMessage))
		return Py_BuildException();

	TItemPos Cell;
	if (!PyTuple_GetByte(poArgs, Idx++, &Cell.window_type))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, Idx++, &Cell.cell))
		return Py_BuildException();

	int iYang;
	if (!PyTuple_GetInteger(poArgs, Idx++, &iYang))
		return Py_BuildException();

	int iWon;
	if (!PyTuple_GetInteger(poArgs, Idx++, &iWon))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendPostWrite(szName, szTitle, szMessage, Cell, iYang, iWon);
	return Py_BuildNone();
}

PyObject* netRequestPostAddData(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t ButtonIndex;
	if (!PyTuple_GetByte(poArgs, 0, &ButtonIndex))
		return Py_BuildException();

	uint8_t DataIndex;
	if (!PyTuple_GetByte(poArgs, 1, &DataIndex))
		return Py_BuildException();

	CPythonNetworkStream::Instance().RequestPostAddData(ButtonIndex, DataIndex);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_KEYCHANGE_SYSTEM
PyObject* netOpenKeyChangeWindow(PyObject* poSelf, PyObject* poArgs)
{
	CPythonPlayer::Instance().OpenKeyChangeWindow();
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
PyObject* netSendChangeLookCheckIn(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos Cell;
	if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
		return Py_BuildException();

	uint8_t bChangeLookSlotType;
	if (!PyTuple_GetByte(poArgs, 2, &bChangeLookSlotType))
		return Py_BuildException();

	if (bChangeLookSlotType >= static_cast<uint8_t>(ETRANSMUTATIONSLOTTYPE::TRANSMUTATION_SLOT_MAX))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendChangeLookCheckInPacket(Cell, bChangeLookSlotType);

	return Py_BuildNone();
}

PyObject* netSendChangeLookCheckOut(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t bChangeLookSlotType;
	if (!PyTuple_GetByte(poArgs, 0, &bChangeLookSlotType))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendChangeLookCheckOutPacket(bChangeLookSlotType);

	return Py_BuildNone();
}

PyObject* netSendChangeLookCheckInFreeYangItem(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos Cell;
	if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendChangeLookCheckInFreeYangItemPacket(Cell);

	return Py_BuildNone();
}

PyObject* netSendChangeLookCheckOutFreeYangItem(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendChangeLookCheckOutFreeYangItemPacket();

	return Py_BuildNone();
}

PyObject* netSendChangeLookAccept(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendChangeLookAcceptPacket();

	return Py_BuildNone();
}

PyObject* netSendChangeLookCancel(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendChangeLookCancelPacket();

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_PARTY_MATCH
PyObject* netSendPartyMatchSearch(PyObject* poSelf, PyObject* poArgs)
{
	int index;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	CPythonNetworkStream::Instance().PartyMatch(index, CPythonPlayer::PARTY_MATCH_SEARCH);
	return Py_BuildNone();
}
PyObject* netSendPartyMatchCancel(PyObject* poSelf, PyObject* poArgs)
{
	int index;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	CPythonNetworkStream::Instance().PartyMatch(index, CPythonPlayer::PARTY_MATCH_CANCEL);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
PyObject* netSendExtBattlePassAction(PyObject* poSelf, PyObject* poArgs)
{
	int iAction = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &iAction))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendExtBattlePassAction(iAction);
	return Py_BuildNone();
}

PyObject* netSendExtBattlePassPremium(PyObject* poSelf, PyObject* poArgs)
{
	bool premium = 0;
	if (!PyTuple_GetBoolean(poArgs, 0, &premium))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendExtBattlePassPremium(premium);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_EVENT_MANAGER
PyObject* netSendRequestEventQuest(PyObject* poSelf, PyObject* poArgs)
{
	char* szString;
	if (!PyTuple_GetString(poArgs, 0, &szString))
		return Py_BuildException();

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendRequestEventQuest(szString);

	return Py_BuildNone();
}

PyObject* netSendRequestEventData(PyObject* poSelf, PyObject* poArgs)
{
	int iMonth;
	if (!PyTuple_GetInteger(poArgs, 0, &iMonth))
		return Py_BuildException();

	if ((iMonth >= 0) && (iMonth <= InGameEventManager::MONTH_MAX_NUM))
	{
		CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
		rns.SendRequestEventData(iMonth);
	}

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_FLOWER_EVENT
PyObject* netSendFlowerEventExchange(PyObject* poSelf, PyObject* poArgs)
{
	uint8_t id;
	if (!PyTuple_GetByte(poArgs, 0, &id))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendFlowerEventExchange(id);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_HUNTING_SYSTEM
PyObject* netSendHuntingAction(PyObject* poSelf, PyObject* poArgs)
{
	int bAction;
	if (!PyTuple_GetInteger(poArgs, 0, &bAction))
		return Py_BuildException();

	int dValue;
	if (!PyTuple_GetInteger(poArgs, 1, &dValue))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendHuntingAction(bAction, dValue);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
PyObject* netSendLottoOpenWindow(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& net = CPythonNetworkStream::Instance();
	net.SendOpenLottoWindow();
	return Py_BuildNone();
}

PyObject* netSendLottoOpenRanking(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& net = CPythonNetworkStream::Instance();
	net.SendOpenRankingWindow();
	return Py_BuildNone();
}

PyObject* netSendLottoBuyTicket(PyObject* poSelf, PyObject* poArgs)
{
	int slot;
	if (!PyTuple_GetInteger(poArgs, 0, &slot))
		return Py_BuildException();

	int num1;
	if (!PyTuple_GetInteger(poArgs, 1, &num1))
		return Py_BuildException();

	int num2;
	if (!PyTuple_GetInteger(poArgs, 2, &num2))
		return Py_BuildException();

	int num3;
	if (!PyTuple_GetInteger(poArgs, 3, &num3))
		return Py_BuildException();

	int num4;
	if (!PyTuple_GetInteger(poArgs, 4, &num4))
		return Py_BuildException();

	CPythonNetworkStream& net = CPythonNetworkStream::Instance();
	net.SendLottoBuyTicket(slot, num1, num2, num3, num4);
	return Py_BuildNone();
}

PyObject* netSendLottoDeleteTicket(PyObject* poSelf, PyObject* poArgs)
{
	int ticketSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &ticketSlot))
		return Py_BuildException();

	CPythonNetworkStream& net = CPythonNetworkStream::Instance();
	net.SendLottoDeleteTicket(ticketSlot);
	return Py_BuildNone();
}

PyObject* netSendLottoReciveMoney(PyObject* poSelf, PyObject* poArgs)
{
	int ticketSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &ticketSlot))
		return Py_BuildException();

	CPythonNetworkStream& net = CPythonNetworkStream::Instance();
	net.SendLottoReciveMoney(ticketSlot);
	return Py_BuildNone();
}

PyObject* netSendLottoPickMoney(PyObject* poSelf, PyObject* poArgs)
{
	long long amount;
	if (!PyTuple_GetLongLong(poArgs, 0, &amount))
		return Py_BuildException();

	CPythonNetworkStream& net = CPythonNetworkStream::Instance();
	net.SendLottoPickMoney(amount);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_FISHING_RENEWAL
PyObject* netStopFishingNew(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendFishingPacketNew(0, 0);
	return Py_BuildNone();
}

PyObject* netCatchFishingNew(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendFishingPacketNew(0, 2);
	return Py_BuildNone();
}

PyObject* netCatchFishingFailed(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendFishingPacketNew(0, 3);
	return Py_BuildNone();
}
#endif

void initnet()
{
	static PyMethodDef s_methods[] = {{"GetBettingGuildWarValue", netGetBettingGuildWarValue, METH_VARARGS},
									  {"EnableChatInsultFilter", netEnableChatInsultFilter, METH_VARARGS},
									  {"SetServerInfo", netSetServerInfo, METH_VARARGS},
									  {"GetServerInfo", netGetServerInfo, METH_VARARGS},
									  {"PreserveServerCommand", netPreserveServerCommand, METH_VARARGS},
									  {"GetPreservedServerCommand", netGetPreservedServerCommand, METH_VARARGS},

									  {"StartGame", netStartGame, METH_VARARGS},
									  {"Warp", netWarp, METH_VARARGS},
									  {"IsTest", netIsTest, METH_VARARGS},
									  {"SetMarkServer", netSetMarkServer, METH_VARARGS},
									  {"IsChatInsultIn", netIsChatInsultIn, METH_VARARGS},
									  {"IsInsultIn", netIsInsultIn, METH_VARARGS},
									  {"LoadInsultList", netLoadInsultList, METH_VARARGS},
									  {"UploadMark", netUploadMark, METH_VARARGS},
									  {"UploadSymbol", netUploadSymbol, METH_VARARGS},
									  {"GetGuildID", netGetGuildID, METH_VARARGS},
									  {"GetEmpireID", netGetEmpireID, METH_VARARGS},
									  {"GetMainActorVID", netGetMainActorVID, METH_VARARGS},
									  {"GetMainActorRace", netGetMainActorRace, METH_VARARGS},
									  {"GetMainActorEmpire", netGetMainActorEmpire, METH_VARARGS},
									  {"GetMainActorSkillGroup", netGetMainActorSkillGroup, METH_VARARGS},
									  {"GetAccountCharacterSlotDataInteger", netGetAccountCharacterSlotDataInteger, METH_VARARGS},
									  {"GetAccountCharacterSlotDataString", netGetAccountCharacterSlotDataString, METH_VARARGS},

									  // FIELD_MUSIC
									  {"GetFieldMusicFileName", netGetFieldMusicFileName, METH_VARARGS},
									  {"GetFieldMusicVolume", netGetFieldMusicVolume, METH_VARARGS},
									  // END_OF_FIELD_MUSIC

									  {"ToggleGameDebugInfo", netToggleGameDebugInfo, METH_VARARGS},
									  {"SetLoginInfo", netSetLoginInfo, METH_VARARGS},
									  {"SetPhaseWindow", netSetPhaseWindow, METH_VARARGS},
									  {"ClearPhaseWindow", netClearPhaseWindow, METH_VARARGS},
									  {"SetServerCommandParserWindow", netSetServerCommandParserWindow, METH_VARARGS},
									  {"SetAccountConnectorHandler", netSetAccountConnectorHandler, METH_VARARGS},
									  {"SetHandler", netSetHandler, METH_VARARGS},
									  {"SetTCPRecvBufferSize", netSetTCPRecvBufferSize, METH_VARARGS},
									  {"SetTCPSendBufferSize", netSetTCPSendBufferSize, METH_VARARGS},
									  {"SetUDPRecvBufferSize", netSetUDPRecvBufferSize, METH_VARARGS},
									  {"DirectEnter", netDirectEnter, METH_VARARGS},

									  {"LogOutGame", netLogOutGame, METH_VARARGS},
									  {"ExitGame", netExitGame, METH_VARARGS},
									  {"ExitApplication", netExitApplication, METH_VARARGS},
									  {"ConnectTCP", netConnectTCP, METH_VARARGS},
									  {"ConnectUDP", netConnectUDP, METH_VARARGS},
									  {"ConnectToAccountServer", netConnectToAccountServer, METH_VARARGS},
#ifdef ENABLE_SEND_TARGET_INFO
		{ "SendTargetInfoLoad",					netTargetInfoLoad,						METH_VARARGS },
#endif

									  {"SendLoginPacket", netSendLoginPacket, METH_VARARGS},
									  {"SendChinaMatrixCardPacket", netSendChinaMatrixCardPacket, METH_VARARGS},
									  {"SendRunupMatrixCardPacket", netSendRunupMatrixCardPacket, METH_VARARGS},
									  {"SendNEWCIBNPasspodAnswerPacket", netSendNEWCIBNPasspodAnswerPacket, METH_VARARGS},
									  {"SendSelectEmpirePacket", netSendSelectEmpirePacket, METH_VARARGS},
									  {"SendSelectCharacterPacket", netSendSelectCharacterPacket, METH_VARARGS},
									  {"SendChangeNamePacket", netSendChangeNamePacket, METH_VARARGS},
									  {"SendCreateCharacterPacket", netSendCreateCharacterPacket, METH_VARARGS},
									  {"SendDestroyCharacterPacket", netSendDestroyCharacterPacket, METH_VARARGS},
									  {"SendEnterGamePacket", netSendEnterGamePacket, METH_VARARGS},

									  {"SendItemUsePacket", netSendItemUsePacket, METH_VARARGS},
									  {"SendItemUseToItemPacket", netSendItemUseToItemPacket, METH_VARARGS},
									  {"SendItemDropPacket", netSendItemDropPacket, METH_VARARGS},
									  {"SendItemDropPacketNew", netSendItemDropPacketNew, METH_VARARGS},
#ifdef ENABLE_DESTROY_SYSTEM
		{ "SendItemDestroyPacket",				netSendItemDestroyPacket,				METH_VARARGS },
#endif
									  {"SendElkDropPacket", netSendElkDropPacket, METH_VARARGS},
									  {"SendGoldDropPacketNew", netSendGoldDropPacketNew, METH_VARARGS},
#if defined(ENABLE_CHEQUE_SYSTEM) && defined(ENABLE_CHEQUE_EXCHANGE_WINDOW)
		{ "SendWonExchangeSellPacket",			netSendWonExchangeSellPacket,			METH_VARARGS },
		{ "SendWonExchangeBuyPacket",			netSendWonExchangeBuyPacket,			METH_VARARGS },
#endif
									  {"SendItemMovePacket", netSendItemMovePacket, METH_VARARGS},
									  {"SendItemPickUpPacket", netSendItemPickUpPacket, METH_VARARGS},
#ifdef ENABLE_SUMMER_EVENT_ROULETTE
		{ "SoulRoulettePacket",					netSoulRoulettePacket,					METH_VARARGS },
#endif
#ifdef ENABLE_MINI_GAME_BNW
		{ "SendMiniGameBNW",						netSendMiniGameBNW,							METH_VARARGS },
#endif
#ifdef ENABLE_MINI_GAME_FINDM
		{ "SendMiniGameFindM",						netSendMiniGameFindM,						METH_VARARGS },
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
		{ "SendMiniGameYutNori",					netSendMiniGameYutNori,						METH_VARARGS },
#endif
									  {"SendGiveItemPacket", netSendGiveItemPacket, METH_VARARGS},

									  {"SetOfflinePhase", netSetOfflinePhase, METH_VARARGS},
									  {"Disconnect", netDisconnect, METH_VARARGS},
									  {"IsConnect", netIsConnect, METH_VARARGS},

									  {"SendChatPacket", netSendChatPacket, METH_VARARGS},
									  {"SendEmoticon", netSendEmoticon, METH_VARARGS},
									  {"SendWhisperPacket", netSendWhisperPacket, METH_VARARGS},
									  {"SendMobileMessagePacket", netSendMobileMessagePacket, METH_VARARGS},

									  {"SendCharacterPositionPacket", netSendCharacterPositionPacket, METH_VARARGS},

									  {"SendShopEndPacket", netSendShopEndPacket, METH_VARARGS},
									  {"SendShopBuyPacket", netSendShopBuyPacket, METH_VARARGS},
									  {"SendShopSellPacket", netSendShopSellPacket, METH_VARARGS},
									  {"SendShopSellPacketNew", netSendShopSellPacketNew, METH_VARARGS},

									  {"SendExchangeStartPacket", netSendExchangeStartPacket, METH_VARARGS},
									  {"SendExchangeItemAddPacket", netSendExchangeItemAddPacket, METH_VARARGS},
									  {"SendExchangeItemDelPacket", netSendExchangeItemDelPacket, METH_VARARGS},
									  {"SendExchangeElkAddPacket", netSendExchangeElkAddPacket, METH_VARARGS},
									  {"SendExchangeAcceptPacket", netSendExchangeAcceptPacket, METH_VARARGS},
									  {"SendExchangeExitPacket", netSendExchangeExitPacket, METH_VARARGS},

									  {"SendOnClickPacket", netOnClickPacket, METH_VARARGS},

									  // Emoticon String
									  {"RegisterEmoticonString", netRegisterEmoticonString, METH_VARARGS},

									  // Messenger
									  {"SendMessengerAddByVIDPacket", netSendMessengerAddByVIDPacket, METH_VARARGS},
									  {"SendMessengerAddByNamePacket", netSendMessengerAddByNamePacket, METH_VARARGS},
									  {"SendMessengerRemovePacket", netSendMessengerRemovePacket, METH_VARARGS},

#ifdef ENABLE_MESSENGER_BLOCK
		// Block
		{ "SendMessengerBlockAddByVIDPacket",		netSendMessengerBlockAddByVIDPacket,		METH_VARARGS },
		{ "SendMessengerBlockRemovePacket",			netSendMessengerBlockRemovePacket,			METH_VARARGS },
		{ "SendMessengerBlockAddByNamePacket",		netSendMessengerBlockAddByNamePacket,		METH_VARARGS },
		{ "SendMessengerBlockRemoveByVIDPacket",	netSendMessengerBlockRemoveByVIDPacket,		METH_VARARGS },
#endif

									  // Party
									  {"SendPartyInvitePacket", netSendPartyInvitePacket, METH_VARARGS},
									  {"SendPartyInviteAnswerPacket", netSendPartyInviteAnswerPacket, METH_VARARGS},
									  {"SendPartyExitPacket", netSendPartyExitPacket, METH_VARARGS},
									  {"SendPartyRemovePacket", netSendPartyRemovePacket, METH_VARARGS},
//									  {"SendPartyRemovePacket", netSendPartyRemovePacketPID, METH_VARARGS},
//									  {"SendPartyRemovePacketPID", netSendPartyRemovePacketPID, METH_VARARGS},
//									  {"SendPartyRemovePacketVID", netSendPartyRemovePacketVID, METH_VARARGS},
									  {"SendPartySetStatePacket", netSendPartySetStatePacket, METH_VARARGS},
									  {"SendPartyUseSkillPacket", netSendPartyUseSkillPacket, METH_VARARGS},
									  {"SendPartyParameterPacket", netSendPartyParameterPacket, METH_VARARGS},

									  // Safebox
#ifdef ENABLE_SAFEBOX_MONEY
									  {"SendSafeboxSaveMoneyPacket", netSendSafeboxSaveMoneyPacket, METH_VARARGS},
									  {"SendSafeboxWithdrawMoneyPacket", netSendSafeboxWithdrawMoneyPacket, METH_VARARGS},
#endif
									  {"SendSafeboxCheckinPacket", netSendSafeboxCheckinPacket, METH_VARARGS},
									  {"SendSafeboxCheckoutPacket", netSendSafeboxCheckoutPacket, METH_VARARGS},
									  {"SendSafeboxItemMovePacket", netSendSafeboxItemMovePacket, METH_VARARGS},

									  // Mall
									  {"SendMallCheckoutPacket", netSendMallCheckoutPacket, METH_VARARGS},

#ifdef ENABLE_GUILDSTORAGE_SYSTEM
		{ "SendGuildstorageCheckinPacket",		netSendGuildstorageCheckinPacket,		METH_VARARGS },
		{ "SendGuildstorageCheckoutPacket",		netSendGuildstorageCheckoutPacket,		METH_VARARGS },
		{ "SendGuildBankMove",					netSendGuildBankMove,					METH_VARARGS },
#endif

#ifdef ENABLE_OX_RENEWAL
		{ "SendQuestInputStringLongPacket",			netSendQuestInputStringLongPacket,			METH_VARARGS },
#endif

									  // Guild
									  {"SendAnswerMakeGuildPacket", netSendAnswerMakeGuildPacket, METH_VARARGS},
									  {"SendQuestInputStringPacket", netSendQuestInputStringPacket, METH_VARARGS},
									  {"SendQuestConfirmPacket", netSendQuestConfirmPacket, METH_VARARGS},
									  {"SendGuildAddMemberPacket", netSendGuildAddMemberPacket, METH_VARARGS},
									  {"SendGuildRemoveMemberPacket", netSendGuildRemoveMemberPacket, METH_VARARGS},
#if defined(ENABLE_GUILDRENEWAL_SYSTEM) && defined(ENABLE_EXTENDED_RENEWAL_FEATURES)
		{ "SendGuildVoteMemberOut",					netSendGuildVoteMemberOut,					METH_VARARGS },
		{ "SendGuildVoteChangeMaster",				netSendGuildVoteChangeMaster,				METH_VARARGS },
		{ "SendGuildVoteLandAbndon",				netSendGuildVoteLandAbndon,					METH_VARARGS },
#endif
									  {"SendGuildChangeGradeNamePacket", netSendGuildChangeGradeNamePacket, METH_VARARGS},
									  {"SendGuildChangeGradeAuthorityPacket", netSendGuildChangeGradeAuthorityPacket, METH_VARARGS},
									  {"SendGuildOfferPacket", netSendGuildOfferPacket, METH_VARARGS},
									  {"SendGuildPostCommentPacket", netSnedGuildPostCommentPacket, METH_VARARGS},
									  {"SendGuildDeleteCommentPacket", netSnedGuildDeleteCommentPacket, METH_VARARGS},
									  {"SendGuildRefreshCommentsPacket", netSendGuildRefreshCommentsPacket, METH_VARARGS},
									  {"SendGuildChangeMemberGradePacket", netSendGuildChangeMemberGradePacket, METH_VARARGS},
									  {"SendGuildUseSkillPacket", netSendGuildUseSkillPacket, METH_VARARGS},
									  {"SendGuildChangeMemberGeneralPacket", netSendGuildChangeMemberGeneralPacket, METH_VARARGS},
									  {"SendGuildInviteAnswerPacket", netSendGuildInviteAnswerPacket, METH_VARARGS},
									  {"SendGuildChargeGSPPacket", netSendGuildChargeGSPPacket, METH_VARARGS},
									  {"SendGuildDepositMoneyPacket", netSendGuildDepositMoneyPacket, METH_VARARGS},
									  {"SendGuildWithdrawMoneyPacket", netSendGuildWithdrawMoneyPacket, METH_VARARGS},

									  // Refine
									  {"SendRequestRefineInfoPacket", netSendRequestRefineInfoPacket, METH_VARARGS},
									  {"SendRefinePacket", netSendRefinePacket, METH_VARARGS},
									  {"SendSelectItemPacket", netSendSelectItemPacket, METH_VARARGS},

									  // SYSTEM
									  {"SetPacketSequenceMode", netSetPacketSequenceMode, METH_VARARGS},
									  {"SetEmpireLanguageMode", netSetEmpireLanguageMode, METH_VARARGS},

									  // For Test
									  {"SetSkillGroupFake", netSetSkillGroupFake, METH_VARARGS},

									  // Guild Symbol
									  {"SendGuildSymbol", netSendGuildSymbol, METH_VARARGS},
									  {"DisconnectUploader", netDisconnectUploader, METH_VARARGS},
									  {"RecvGuildSymbol", netRecvGuildSymbol, METH_VARARGS},

									  // Log
									  {"RegisterErrorLog", netRegisterErrorLog, METH_VARARGS},

#ifdef ENABLE_GROWTH_PET_SYSTEM
										  // Growth Pet System
									  { "CheckUsePetItem", netCheckUsePetItem, METH_VARARGS },
									  { "SendPetWindowType", netSendPetWindowType, METH_VARARGS },
									  { "SendPetHatchingWindowPacket", netSendPetHatchingWindowPacket, METH_VARARGS },
									  { "SendPetHatchingPacket", netSendPetHatchingPacket, METH_VARARGS },
									  { "SendPetLearnSkill", netSendPetLearnSkill, METH_VARARGS },
									  { "SendPetSkillUpgrade", netSendPetSkillUpgrade, METH_VARARGS },
									  { "SendPetSkillUpgradeRequest", netSendPetSkillUpgradeRequest, METH_VARARGS },
									  { "SendPetFeedPacket", netSendPetFeedPacket, METH_VARARGS },
									  { "SendPetDeleteSkill", netSendPetDeleteSkill, METH_VARARGS },
									  { "SendPetDeleteAllSkill", netSendPetDeleteAllSkill, METH_VARARGS },
									  { "SendPetNameChangePacket", netSendPetNameChangePacket, METH_VARARGS },
									  { "SendPetNameChangeWindowPacket", netSendPetNameChangeWindowPacket, METH_VARARGS },
#	ifdef ENABLE_PET_ATTR_DETERMINE
									  { "SendPetAttrDetermine", netSendPetAttrDetermine, METH_VARARGS },
									  { "SendChangePetPacket", netSendChangePetPacket, METH_VARARGS },
#	endif
#	ifdef ENABLE_PET_PRIMIUM_FEEDSTUFF
									  { "SendPetRevive", netSendPetRevive, METH_VARARGS },
#	endif
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
									  { "SendLanguageChangePacket",				netSendLanguageChangePacket,				METH_VARARGS },
									  { "SendTargetLanguageRequestPacket",		netSendTargetLanguageRequestPacket,			METH_VARARGS },
#endif

#if defined(ENABLE_GEM_SYSTEM) && defined(ENABLE_GEM_SHOP)
									  { "SendGemShopBuy", netSendGemShopBuy, METH_VARARGS },
									  { "SendSlotAdd", netSendSlotAdd, METH_VARARGS },
									  { "SendRequestRefresh", netSendRequestRefresh, METH_VARARGS },
									  { "SendSelectmetinstone", netSendSelectmetinstone, METH_VARARGS },
									  { "SendGemShopClose", netSendGemShopClose, METH_VARARGS },
#endif

#ifdef ENABLE_PRIVATESHOP_SEARCH_SYSTEM
		{ "SendPrivateShopSearchInfo", netSendPrivateShopSearchInfo, METH_VARARGS },
		{ "SendPrivateShopSearchInfoSub", netSendPrivateShopSearchInfoSub, METH_VARARGS },
		{ "SendPrivateShopSerchBuyItem", netSendPrivateShopSerchBuyItem, METH_VARARGS },
		{ "ClosePrivateShopSearchWindow", netClosePrivateShopSearchWindow, METH_VARARGS },
#endif

#ifdef ENABLE_LOADING_TIP
		{ "SetMapTarget", netSetMapTarget, METH_VARARGS },
		{ "GetMapTarget", netGetMapTarget, METH_VARARGS },
#endif

#ifdef ENABLE_MOVE_CHANNEL
									  // Move Channel
									  { "SetChannelName", netSetChannelName, METH_VARARGS },
									  { "GetChannelName", netGetChannelName, METH_VARARGS },
									  { "GetChannelNumber", netGetChannelNumber, METH_VARARGS },
									  { "GetChannelCount", netGetChannelCount, METH_VARARGS },
									  { "SetMapIndex", netSetMapIndex, METH_VARARGS },
									  { "GetMapIndex", netGetMapIndex, METH_VARARGS },
									  { "SetServerName", netSetServerName, METH_VARARGS },
									  { "ClearServerInfo", netClearServerInfo, METH_VARARGS },
									  { "MoveChannelGame", netMoveChannelGame, METH_VARARGS },
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		// Acce System
		{ "SendAcceRefineCheckIn", netSendAcceRefineCheckIn, METH_VARARGS },
		{ "SendAcceRefineCheckOut", netSendAcceRefineCheckOut, METH_VARARGS },
		{ "SendAcceRefineCanCle", netSendAcceRefineCanCle, METH_VARARGS },
		{ "SendAcceRefineAccept", netSendAcceRefineAccept, METH_VARARGS },
#endif

#ifdef ENABLE_MOVE_COSTUME_ATTR
		{ "SendItemCombinationPacket", netSendItemCombinationPacket, METH_VARARGS },
#endif

#ifdef ENABLE_GUILDBANK_LOG
		{ "GuildBankInfoOpen",						netGuildBankInfoOpen,						METH_VARARGS },
		{ "SendGuildBankInfoOpen",					netSendGuildBankInfoOpen,					METH_VARARGS },
#endif

#ifdef ENABLE_GUILD_DONATE_ATTENDANCE
		{ "GuildDonateInfoOpen",					netGuildDonateInfoOpen,					METH_VARARGS },
#endif

#ifdef ENABLE_ATTR_6TH_7TH
		{ "SendSkillBookCombinationPacket", netSendSkillBookCombinationPacket, METH_VARARGS },
		{ "SendRegistFragmentPacket", netSendRegistFragmentPacket, METH_VARARGS },
		{ "SendAttr67AddPacket", netSendAttr67AddPacket, METH_VARARGS },
		{ "SendAttr67ClosePacket", netSendAttr67ClosePacket, METH_VARARGS },
#endif

#ifdef ENABLE_PREMIUM_PRIVATE_SHOP
		{ "SendRemoveItemFromMyShop", netSendRemoveItemFromMyShop, METH_VARARGS },
		{ "SendAddItemToMyShop", netSendAddItemToMyShop, METH_VARARGS },
		{ "SendOpenMyShop", netSendOpenMyShop, METH_VARARGS },
#ifdef ENABLE_PREMIUM_PRIVATE_SHOP_EXPIRE
		{ "SendReopenMyShop", netSendReopenMyShop, METH_VARARGS },
#endif
		{ "SendCloseMyShop", netSendCloseMyShop, METH_VARARGS },
		{ "SendWithdrawMyShopMoney", netSendWithdrawMyShopMoney, METH_VARARGS },
		{ "SendRenameMyShop", netSendRenameMyShop, METH_VARARGS },

#ifdef ENABLE_MYSHOP_DECO
		{ "SendMyShopDecoSet", netSendMyShopDecoSet, METH_VARARGS },
		{ "SendMyShopDecoState", netSendMyShopDecoState, METH_VARARGS },
#endif
#endif

#ifdef ENABLE_REFINE_ELEMENT
		{ "ElementsSpellClose",						netSendElementsSpellClose,						METH_VARARGS },
		{ "ElementsSpellAdd",						netSendElementsSpellAdd,						METH_VARARGS },
		{ "ElementsSpellChange",					netSendElementsSpellChange,						METH_VARARGS },
#endif

#ifdef ENABLE_EXTEND_INVEN_ITEM_UPGRADE
			// Extend Inven System
		{ "SendExtendInvenButtonClick", netSendExtendInvenButtonClick, METH_VARARGS },
		{ "SendExtendInvenUpgrade", netSendExtendInvenUpgrade, METH_VARARGS },
#endif

#ifdef NEW_SELECT_CHARACTER
		{ "GetLoginID", netGetLoginID, METH_VARARGS },
#endif

#ifdef ENABLE_AURA_SYSTEM
		{ "SendAuraRefineCheckIn",					netSendAuraRefineCheckIn,					METH_VARARGS },
		{ "SendAuraRefineCheckOut",					netSendAuraRefineCheckOut,					METH_VARARGS },
		{ "SendAuraRefineAccept",					netSendAuraRefineAccept,					METH_VARARGS },
		{ "SendAuraRefineCancel",					netSendAuraRefineCancel,					METH_VARARGS },
#endif

#ifdef ENABLE_BINARY_SERVERINFO
		{ "GetServer_IPNAME",					netGetServer_IPNAME,					METH_VARARGS },
		{ "GetServer_PORT",						netGetServer_PORT,						METH_VARARGS },
#endif

#ifdef ENABLE_HWID_BAN
		{ "SendHwidBanPacket",						netSendHwidBanPacket,					METH_VARARGS },
#endif

#ifdef ENABLE_CHANGED_ATTR
		{ "SendChagedItemAttributePacket",			netSendChagedItemAttributePacket,		METH_VARARGS },
#endif

#ifdef ENABLE_EVENTS
#	ifdef ENABLE_MINI_GAME_OKEY_NORMAL
		{ "SendMiniGameRumiStart",					netSendMiniGameRumiStart,				METH_VARARGS },
		{ "SendMiniGameRumiExit",					netSendMiniGameRumiExit,				METH_VARARGS },
		{ "SendMiniGameRumiDeckCardClick",			netSendMiniGameRumiDeckCardClick,		METH_VARARGS },
		{ "SendMiniGameRumiHandCardClick",			netSendMiniGameRumiHandCardClick,		METH_VARARGS },
		{ "SendMiniGameRumiFieldCardClick",			netSendMiniGameRumiFieldCardClick,		METH_VARARGS },
		{ "SendMiniGameRumiDestroy",				netSendMiniGameRumiDestroy,				METH_VARARGS },
#	endif
#	ifdef ENABLE_MINI_GAME_CATCH_KING
		{ "SendMiniGameCatchKingStart",				netSendMiniGameCatchKingStart,				METH_VARARGS },
		{ "SendMiniGameCatchKingDeckCardClick",		netSendMiniGameCatchKingDeckCardClick,		METH_VARARGS },
		{ "SendMiniGameCatchKingFieldCardClick",	netSendMiniGameCatchKingFieldCardClick,		METH_VARARGS },
		{ "SendMiniGameCatchKingReward",			netSendMiniGameCatchKingReward,				METH_VARARGS },
#	endif

#	ifdef ENABLE_MONSTER_BACK
		{ "SendAttendanceGetReward",				netSendAttendanceGetReward,					METH_VARARGS },
#	endif

#	ifdef ENABLE_FISH_EVENT
		{ "SendUseFishBox",							netSendFishBoxUse,							METH_VARARGS },
		{ "SendAddFishBox",							netSendFishShapeAdd,						METH_VARARGS },
		{ "SendRequestFishEventBlock",				netSendRequestFishEventBlock,					METH_VARARGS }, 
#	endif
#endif

#ifdef ENABLE_MAILBOX
		{ "SendPostDelete",							netSendPostDelete,							METH_VARARGS },
		{ "SendPostGetItems",						netSendPostGetItems,						METH_VARARGS },
		{ "SendPostWriteConfirm",					netSendPostWriteConfirm,					METH_VARARGS },
		{ "SendPostWrite",							netSendPostWrite,							METH_VARARGS },
		{ "SendMailBoxClose",						netSendMailBoxClose,						METH_VARARGS },
		{ "SendPostAllDelete",						netSendPostAllDelete,						METH_VARARGS },
		{ "SendPostAllGetItems",					netSendPostAllGetItems,						METH_VARARGS },
		{ "RequestPostAddData",						netRequestPostAddData,						METH_VARARGS },
#endif

#ifdef ENABLE_KEYCHANGE_SYSTEM
		{ "OpenKeyChangeWindow",					netOpenKeyChangeWindow,						METH_VARARGS },
#endif

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		{ "SendChangeLookCheckIn",					netSendChangeLookCheckIn,					METH_VARARGS },
		{ "SendChangeLookCheckOut",					netSendChangeLookCheckOut,					METH_VARARGS },
		{ "SendChangeLookCheckInFreeYangItem",		netSendChangeLookCheckInFreeYangItem,		METH_VARARGS },
		{ "SendChangeLookCheckOutFreeYangItem",		netSendChangeLookCheckOutFreeYangItem,		METH_VARARGS },
		{ "SendChangeLookCheckOut",					netSendChangeLookCheckOut,					METH_VARARGS },
		{ "SendChangeLookAccept",					netSendChangeLookAccept,					METH_VARARGS },
		{ "SendChangeLookCanCle",					netSendChangeLookCancel,					METH_VARARGS },
#endif

#ifdef ENABLE_PARTY_MATCH
		{ "SendPartyMatchSearch",				netSendPartyMatchSearch,				METH_VARARGS },
		{ "SendPartyMatchCancel",				netSendPartyMatchCancel,				METH_VARARGS },
#endif

#ifdef ENABLE_BATTLE_PASS_SYSTEM
		{ "SendExtBattlePassAction",		netSendExtBattlePassAction,			METH_VARARGS },
		{ "SendExtBattlePassPremium",	netSendExtBattlePassPremium,	METH_VARARGS },
#endif

#ifdef ENABLE_EVENT_MANAGER
		{ "SendRequestEventQuest", netSendRequestEventQuest, METH_VARARGS },
		{ "SendRequestEventData", netSendRequestEventData, METH_VARARGS },
#endif

#ifdef ENABLE_FLOWER_EVENT
		{ "SendFlowerEventExchange", netSendFlowerEventExchange, METH_VARARGS },
#endif

#ifdef ENABLE_HUNTING_SYSTEM
		{ "SendHuntingAction", netSendHuntingAction, METH_VARARGS },
#endif

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
		{ "LottoOpenWindow",			netSendLottoOpenWindow,			METH_VARARGS },
		{ "LottoOpenRanking",			netSendLottoOpenRanking,			METH_VARARGS },
		{ "LottoBuyTicket",				netSendLottoBuyTicket,					METH_VARARGS },
		{ "LottoDeleteTicket",			netSendLottoDeleteTicket,				METH_VARARGS },
		{ "LottoReciveMoney",			netSendLottoReciveMoney,				METH_VARARGS },
		{ "LottoPickMoney",				netSendLottoPickMoney,					METH_VARARGS },
#endif

#ifdef ENABLE_FISHING_RENEWAL
		{ "StopFishingNew", netStopFishingNew, METH_VARARGS },
		{ "CatchFishingNew", netCatchFishingNew, METH_VARARGS },
		{ "CatchFishingFailed", netCatchFishingFailed, METH_VARARGS },
#endif

									  {nullptr, nullptr, 0}};

	PyObject* poModule = Py_InitModule("m2netm2g", s_methods);	//@fixme430

	PyModule_AddIntConstant(poModule, "ERROR_NONE", CPythonNetworkStream::ERROR_NONE);
	PyModule_AddIntConstant(poModule, "ERROR_CONNECT_MARK_SERVER", CPythonNetworkStream::ERROR_CONNECT_MARK_SERVER);
	PyModule_AddIntConstant(poModule, "ERROR_LOAD_MARK", CPythonNetworkStream::ERROR_LOAD_MARK);
	PyModule_AddIntConstant(poModule, "ERROR_MARK_WIDTH", CPythonNetworkStream::ERROR_MARK_WIDTH);
	PyModule_AddIntConstant(poModule, "ERROR_MARK_HEIGHT", CPythonNetworkStream::ERROR_MARK_HEIGHT);

	// MARK_BUG_FIX
	PyModule_AddIntConstant(poModule, "ERROR_MARK_UPLOAD_NEED_RECONNECT", CPythonNetworkStream::ERROR_MARK_UPLOAD_NEED_RECONNECT);
	PyModule_AddIntConstant(poModule, "ERROR_MARK_CHECK_NEED_RECONNECT", CPythonNetworkStream::ERROR_MARK_CHECK_NEED_RECONNECT);
	// END_OF_MARK_BUG_FIX

	PyModule_AddIntConstant(poModule, "PHASE_WINDOW_LOGIN", CPythonNetworkStream::PHASE_WINDOW_LOGIN);
	PyModule_AddIntConstant(poModule, "PHASE_WINDOW_SELECT", CPythonNetworkStream::PHASE_WINDOW_SELECT);
	PyModule_AddIntConstant(poModule, "PHASE_WINDOW_CREATE", CPythonNetworkStream::PHASE_WINDOW_CREATE);
	PyModule_AddIntConstant(poModule, "PHASE_WINDOW_LOAD", CPythonNetworkStream::PHASE_WINDOW_LOAD);
	PyModule_AddIntConstant(poModule, "PHASE_WINDOW_GAME", CPythonNetworkStream::PHASE_WINDOW_GAME);
	PyModule_AddIntConstant(poModule, "PHASE_WINDOW_EMPIRE", CPythonNetworkStream::PHASE_WINDOW_EMPIRE);
	PyModule_AddIntConstant(poModule, "PHASE_WINDOW_LOGO", CPythonNetworkStream::PHASE_WINDOW_LOGO);

	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_ID", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_ID);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_NAME", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_NAME);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_RACE", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_RACE);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_LEVEL", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_LEVEL);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_STR", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_STR);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_DEX", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_DEX);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_INT", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_INT);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_HTH", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_HTH);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_PLAYTIME", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_PLAYTIME);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_FORM", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_FORM);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_ADDR", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_ADDR);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_PORT", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_PORT);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_GUILD_ID", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_GUILD_ID);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_GUILD_NAME", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_GUILD_NAME);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_HAIR", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_HAIR);
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_ACCE", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_ACCE);
#endif
#ifdef ENABLE_AURA_SYSTEM
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_AURA", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_AURA);
#endif
#ifdef ENABLE_YOHARA_SYSTEM
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_CONQUEROR_LEVEL", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_CONQUEROR_LEVEL);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_SUNGMA_STR", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_SUNGMA_STR);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_SUNGMA_HP", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_SUNGMA_HP);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_SUNGMA_MOVE", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_SUNGMA_MOVE);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_SUNGMA_IMMUNE", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_SUNGMA_IMMUNE);
#endif

	PyModule_AddIntConstant(poModule, "SERVER_COMMAND_LOG_OUT", CPythonNetworkStream::SERVER_COMMAND_LOG_OUT);
	PyModule_AddIntConstant(poModule, "SERVER_COMMAND_RETURN_TO_SELECT_CHARACTER", CPythonNetworkStream::SERVER_COMMAND_RETURN_TO_SELECT_CHARACTER);
	PyModule_AddIntConstant(poModule, "SERVER_COMMAND_QUIT", CPythonNetworkStream::SERVER_COMMAND_QUIT);

	PyModule_AddIntConstant(poModule, "EMPIRE_A", 1);
	PyModule_AddIntConstant(poModule, "EMPIRE_B", 2);
	PyModule_AddIntConstant(poModule, "EMPIRE_C", 3);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_REFINE_FAIL", DS_SUB_HEADER_REFINE_FAIL);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE", DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL", DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY", DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL", DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL", DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_REFINE_SUCCEED", DS_SUB_HEADER_REFINE_SUCCEED);

#ifdef NEW_SELECT_CHARACTER
	PyModule_AddIntConstant(poModule, "CHARACTER_SLOT_COUNT_MAX", PLAYER_PER_ACCOUNT);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_LAST_PLAYTIME", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_LAST_PLAYTIME);
#endif

#ifdef ENABLE_BIOLOG_SYSTEM
	PyModule_AddIntConstant(poModule, "BIOLOG_MANAGER_OPEN", BIOLOG_MANAGER_SUBHEADER_CG_OPEN);
	PyModule_AddIntConstant(poModule, "BIOLOG_MANAGER_SEND", BIOLOG_MANAGER_SUBHEADER_CG_SEND);
	PyModule_AddIntConstant(poModule, "BIOLOG_MANAGER_TIMER", BIOLOG_MANAGER_SUBHEADER_CG_TIMER);
#endif
}
