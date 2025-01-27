#include "Stdafx.h"
#include "SoundManagerStream.h"

CSoundManagerStream::CSoundManagerStream() = default;

CSoundManagerStream::~CSoundManagerStream() = default;

bool CSoundManagerStream::Initialize()
{
	CSoundBase::Initialize();

	if (ms_DIGDriver)
		return true;

	ms_DIGDriver = AIL_open_digital_driver(44100, 16, 2, 0);

	for (auto & m_Instance : m_Instances)
		m_Instance.Initialize();

	return true;
}

void CSoundManagerStream::Destroy()
{
	for (auto & m_Instance : m_Instances)
		m_Instance.Stop();

	CSoundBase::Destroy();
}

bool CSoundManagerStream::SetInstance(uint32_t dwIndex, const char * filename)
{
	if (!CheckInstanceIndex(dwIndex))
		return false;

	HSTREAM hStream = AIL_open_stream(ms_DIGDriver, filename, 0);

	if (nullptr == hStream)
		return false;

	m_Instances[dwIndex].SetStream(hStream);

	return true;
}

CSoundInstanceStream * CSoundManagerStream::GetInstance(uint32_t dwIndex)
{
	if (!CheckInstanceIndex(dwIndex))
		return nullptr;

	return &m_Instances[dwIndex];
}

bool CSoundManagerStream::CheckInstanceIndex(uint32_t dwIndex)
{
	if (dwIndex >= uint32_t(MUSIC_INSTANCE_MAX_NUM))
		return false;

	return true;
}