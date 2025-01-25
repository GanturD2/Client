#include "StdAfx.h"
#include "Timer.h"

static uint32_t gs_dwBaseTime = 0;
static uint32_t gs_dwServerTime = 0;
static uint32_t gs_dwClientTime = 0;
static uint32_t gs_dwFrameTime = 0;

#pragma comment(lib, "winmm.lib")

BOOL ELTimer_Init()
{
	gs_dwBaseTime = timeGetTime();
	return 1;
}

uint32_t ELTimer_GetMSec()
{
	return timeGetTime() - gs_dwBaseTime;
}

VOID ELTimer_SetServerMSec(uint32_t dwServerTime)
{
	NANOBEGIN
	if (0 != dwServerTime) // nanomite를 위한 더미 if
	{
		gs_dwServerTime = dwServerTime;
		gs_dwClientTime = CTimer::Instance().GetCurrentMillisecond();
	}
	NANOEND
}

uint32_t ELTimer_GetServerMSec()
{
	return CTimer::Instance().GetCurrentMillisecond() - gs_dwClientTime + gs_dwServerTime;
}

uint32_t ELTimer_GetFrameMSec()
{
	return gs_dwFrameTime;
}

uint32_t ELTimer_GetServerFrameMSec()
{
	return ELTimer_GetFrameMSec() - gs_dwClientTime + gs_dwServerTime;
}

VOID ELTimer_SetFrameMSec()
{
	gs_dwFrameTime = ELTimer_GetMSec();
}

CTimer::CTimer()
{
	ELTimer_Init();

	NANOBEGIN
	{
		m_dwCurrentTime = 0;
		m_bUseRealTime = true;
		m_index = 0;

		m_dwElapsedTime = 0;

		m_fCurrentTime = 0.0f;
	}
	NANOEND
}

CTimer::~CTimer() = default;

void CTimer::SetBaseTime()
{
	m_dwCurrentTime = 0;
}

void CTimer::Advance()
{
	if (!m_bUseRealTime)
	{
		++m_index;

		if (m_index == 1)
			m_index = -1;

		m_dwCurrentTime += 16 + (m_index & 1);
		m_fCurrentTime = m_dwCurrentTime / 1000.0f;
	}
	else
	{
		uint32_t currentTime = ELTimer_GetMSec();

		if (m_dwCurrentTime == 0)
			m_dwCurrentTime = currentTime;

		m_dwElapsedTime = currentTime - m_dwCurrentTime;
		m_dwCurrentTime = currentTime;
	}
}

void CTimer::Adjust(int iTimeGap)
{
	m_dwCurrentTime += iTimeGap;
}

float CTimer::GetCurrentSecond() const
{
	if (m_bUseRealTime)
		return ELTimer_GetMSec() / 1000.0f;

	return m_fCurrentTime;
}

uint32_t CTimer::GetCurrentMillisecond() const
{
	if (m_bUseRealTime)
		return ELTimer_GetMSec();

	return m_dwCurrentTime;
}

float CTimer::GetElapsedSecond()
{
	return GetElapsedMilliecond() / 1000.0f;
}

uint32_t CTimer::GetElapsedMilliecond() const
{
	if (!m_bUseRealTime)
		return 16 + (m_index & 1);

	return m_dwElapsedTime;
}

void CTimer::UseCustomTime()
{
	m_bUseRealTime = false;
}
