#pragma once

#include "../EterLib/NetStream.h"

class CServerStateChecker : public CSingleton<CServerStateChecker>
{
public:
	CServerStateChecker();
	~CServerStateChecker();
	CLASS_DELETE_COPYMOVE(CServerStateChecker);

	void Create(PyObject * poWnd);
	void AddChannel(uint32_t uServerIndex, const char * c_szAddr, uint32_t uPort);
	void Request();
	void Update();

	void Initialize();

private:
	typedef struct SChannel
	{
		uint32_t uServerIndex;
		const char * c_szAddr;
		uint32_t uPort;
	} TChannel;

	PyObject * m_poWnd;

	std::list<TChannel> m_lstChannel;

	CNetworkStream m_kStream;
};