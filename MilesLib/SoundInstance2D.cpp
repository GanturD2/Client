#include "Stdafx.h"
#include "SoundManager2D.h"

CSoundInstance2D::CSoundInstance2D() : m_sample(nullptr), m_pSoundData(nullptr) {}

CSoundInstance2D::~CSoundInstance2D()
{
	Destroy();
}

void CSoundInstance2D::Destroy()
{
	SAFE_RELEASE(m_pSoundData);

	if (m_sample)
	{
		AIL_release_sample_handle(m_sample);
		m_sample = nullptr;
	}
}

bool CSoundInstance2D::Initialize()
{
	if (m_sample)
		return true;

	m_sample = AIL_allocate_sample_handle(ms_DIGDriver);
	return m_sample ? true : false;
}

bool CSoundInstance2D::SetSound(CSoundData * pSoundData)
{
	assert(m_sample != nullptr && pSoundData != nullptr);

	// 레퍼런스 카운트가 1이 될 때 로드를 해야 제대로 사이즈가 리턴
	// 되므로 반드시 Get을 호출 하고 진행해야 한다.
	// 또, m_pSoundData가 pSoundData와 같고 m_pSoundData의 레퍼런스
	// 카운터가 1일 경우, 불필요하게 로드가 일어나므로 미리 레퍼런스
	// 카운터를 올려놔야 한다.
	LPVOID lpData = pSoundData->Get();

	AIL_init_sample(m_sample);

	if (AIL_set_sample_file(m_sample, lpData, pSoundData->GetSize()) == 0)
	{
		if (m_pSoundData != nullptr)
		{
			m_pSoundData->Release();
			m_pSoundData = nullptr;
		}

		pSoundData->Release();
		TraceError("%s: %s", AIL_last_error(), pSoundData->GetFileName());
		return false;
	}

	if (m_pSoundData != nullptr)
	{
		m_pSoundData->Release();
		m_pSoundData = nullptr;
	}

	m_pSoundData = pSoundData;
	return true;
}

bool CSoundInstance2D::IsDone() const
{
	return AIL_sample_status(m_sample) == SMP_DONE;
}

void CSoundInstance2D::Play(int iLoopCount, uint32_t dwPlayCycleTimeLimit) const
{
	AIL_set_sample_loop_count(m_sample, iLoopCount);
	AIL_start_sample(m_sample);
}

void CSoundInstance2D::Pause() const
{
	AIL_stop_sample(m_sample);
}

void CSoundInstance2D::Resume() const
{
	AIL_resume_sample(m_sample);
}

void CSoundInstance2D::Stop()
{
	AIL_end_sample(m_sample);
	m_sample = nullptr;
}

void CSoundInstance2D::GetVolume(float & rfVolume) const
{
	AIL_sample_volume_pan(m_sample, &rfVolume, nullptr);
}

void CSoundInstance2D::SetVolume(float volume) const
{
	volume = std::max(0.0f, std::min(1.0f, volume));
	AIL_set_sample_volume_pan(m_sample, volume, 0.5f);
}

void CSoundInstance2D::SetPosition(float x, float y, float z) const
{
	assert(!"must not call this method");
}

void CSoundInstance2D::SetOrientation(float x_face, float y_face, float z_face, float x_normal, float y_normal, float z_normal) const
{
	assert(!"must not call this method");
}

void CSoundInstance2D::SetVelocity(float fx, float fy, float fz, float fMagnitude) const
{
	assert(!"must not call this method");
}
