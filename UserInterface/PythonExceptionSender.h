#pragma once

class CPythonExceptionSender : public IPythonExceptionSender
{
public:
	CPythonExceptionSender();
	virtual ~CPythonExceptionSender();
	CLASS_DELETE_COPYMOVE(CPythonExceptionSender);

	void Send() override;

protected:
	std::set<uint32_t> m_kSet_dwSendedExceptionCRC;
};
