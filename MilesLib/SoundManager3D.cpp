#include "Stdafx.h"
#include "SoundManager3D.h"

CSoundManager3D::CSoundManager3D()
{
	m_bInit = false;
}

CSoundManager3D::~CSoundManager3D() = default;

bool CSoundManager3D::Initialize()
{
	CSoundBase::Initialize();

	if (ms_pProviderDefault)
		return true;

	ms_ProviderVector.resize(MAX_PROVIDERS);

	HPROENUM enum3D = HPROENUM_FIRST;
	int i = 0;

	while (AIL_enumerate_3D_providers(&enum3D, &ms_ProviderVector[i].hProvider, &ms_ProviderVector[i].name) && (i < MAX_PROVIDERS))
	{
		TProvider * provider = &ms_ProviderVector[i];

		//if (strcmp(provider->name, "DirectSound3D Software Emulation") == 0)
		//if (strcmp(provider->name, "DirectSound3D Hardware Support") == 0)
		//if (strcmp(provider->name, "DirectSound3D 7+ Software - Pan and Volume") == 0)
		//if (strcmp(provider->name, "DirectSound3D 7+ Software - Light HRTF") == 0)
		//if (strcmp(provider->name, "DirectSound3D 7+ Software - Full HRTF") == 0)
		//if (strcmp(provider->name, "RAD Game Tools RSX 3D Audio") == 0)
		//if (strcmp(provider->name, "Dolby Surround") == 0)
		if (strcmp(provider->name, "Miles Fast 2D Positional Audio") == 0)
			ms_pProviderDefault = provider;

		++i;
	}

	if (!ms_pProviderDefault)
	{
		CSoundBase::Destroy();
		return false;
	}

	assert(ms_pProviderDefault != nullptr);

	if (M3D_NOERR != AIL_open_3D_provider(ms_pProviderDefault->hProvider))
	{
		//		assert(!"AIL_open_3D_provider error");
		//		char buf[64];
		//		sprintf(buf, "Error AIL_open_3D_provider: %s\n", AIL_last_error());
		//		OutputDebugString(buf);

		CSoundBase::Destroy();
		return false;
	}

	m_pListener = AIL_open_3D_listener(ms_pProviderDefault->hProvider);

	SetListenerPosition(0.0f, 0.0f, 0.0f);

	for (i = 0; i < INSTANCE_MAX_COUNT; ++i)
	{
		m_Instances[i].Initialize();
		m_bLockingFlag[i] = false;
	}

	m_bInit = true;
	return true;
}


void CSoundManager3D::Destroy()
{
	if (!m_bInit)
		return;

	for (auto & m_Instance : m_Instances)
		m_Instance.Destroy();

	if (m_pListener)
	{
		AIL_close_3D_listener(m_pListener);
		m_pListener = nullptr;
	}

	if (ms_pProviderDefault)
	{
		AIL_close_3D_provider(ms_pProviderDefault->hProvider);
		ms_pProviderDefault = nullptr;
	}

	CSoundBase::Destroy();
	m_bInit = false;
}

void CSoundManager3D::SetListenerDirection(float fxDir, float fyDir, float fzDir, float fxUp, float fyUp, float fzUp)
{
	if (nullptr == m_pListener)
		return;
	AIL_set_3D_orientation(m_pListener, fxDir, fyDir, -fzDir, fxUp, fyUp, -fzUp);
}

void CSoundManager3D::SetListenerPosition(float fX, float fY, float fZ)
{
	// 	assert(m_pListener != nullptr);
	if (nullptr == m_pListener)
		return;
	AIL_set_3D_position(m_pListener, fX, fY, -fZ);
}

void CSoundManager3D::SetListenerVelocity(float fDistanceX, float fDistanceY, float fDistanceZ, float fNagnitude)
{
	// 	assert(m_pListener != nullptr);
	if (nullptr == m_pListener)
		return;
	AIL_set_3D_velocity(m_pListener, fDistanceX, fDistanceY, -fDistanceZ, fNagnitude);
}

int CSoundManager3D::SetInstance(const char * c_pszFileName)
{
	uint32_t dwFileCRC = GetFileCRC(c_pszFileName);
	auto itor = ms_dataMap.find(dwFileCRC);

	CSoundData * pkSoundData;

	if (itor == ms_dataMap.end())
		pkSoundData = AddFile(dwFileCRC, c_pszFileName); // CSoundBase::AddFile
	else
		pkSoundData = itor->second;

	assert(pkSoundData != nullptr);

	static uint32_t k = 0;

	uint32_t start = k++;
	uint32_t end = start + INSTANCE_MAX_COUNT;

	while (start < end)
	{
		CSoundInstance3D * pkInst = &m_Instances[start % INSTANCE_MAX_COUNT];

		if (pkInst->IsDone())
		{
			if (!pkInst->SetSound(pkSoundData))
			{
				TraceError("CSoundManager3D::GetInstance (filename: %s)", c_pszFileName);
				// NOTE : 사운드가 없을 경우 Failed to set. return nullptr. - [levites]
				return -1;
			}

			return (start % INSTANCE_MAX_COUNT);
		}

		++start;

		// 설마 uint32_t 한계값을 넘어갈리야 없겠지만.. 그래도.. 혹시나.. - [levites]
		if (start > 50000)
		{
			start = 0;
			return -1;
		}
	}

	return -1;
}

ISoundInstance * CSoundManager3D::GetInstance(uint32_t dwIndex)
{
	if (dwIndex >= INSTANCE_MAX_COUNT)
	{
		assert(dwIndex < INSTANCE_MAX_COUNT);
		return nullptr;
	}
	return &m_Instances[dwIndex];
}

__forceinline bool CSoundManager3D::IsValidInstanceIndex(int iIndex)
{
	if (iIndex >= 0 && iIndex < INSTANCE_MAX_COUNT)
		return true;

	return false;
}

void CSoundManager3D::Lock(int iIndex)
{
	if (!IsValidInstanceIndex(iIndex))
		return;

	m_bLockingFlag[iIndex] = true;
}

void CSoundManager3D::Unlock(int iIndex)
{
	if (!IsValidInstanceIndex(iIndex))
		return;

	m_bLockingFlag[iIndex] = false;
}
