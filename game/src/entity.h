#ifndef __INC_METIN_II_GAME_ENTITY_H__
#define __INC_METIN_II_GAME_ENTITY_H__

#include <unordered_map>
#include <unordered_set>

class SECTREE;

class CEntity
{
public:
	typedef std::unordered_map<LPENTITY, int> ENTITY_MAP;

public:
	CEntity();
	virtual ~CEntity();

	virtual void EncodeInsertPacket(LPENTITY entity) = 0;
	virtual void EncodeRemovePacket(LPENTITY entity) = 0;

protected:
	void Initialize(int type = -1);
	void Destroy();


public:
	void SetType(int type);
	int GetType() const;
	bool IsType(int type) const;

	void ViewCleanup();
	void ViewInsert(LPENTITY entity, bool recursive = true);
	void ViewRemove(LPENTITY entity, bool recursive = true);
	void ViewReencode();

	int GetViewAge() const noexcept { return m_iViewAge; }

	long GetX() const noexcept { return m_pos.x; }
	long GetY() const noexcept { return m_pos.y; }
	long GetZ() const noexcept { return m_pos.z; }
	const PIXEL_POSITION& GetXYZ() const noexcept { return m_pos; }

	void SetXYZ(long x, long y, long z) noexcept { m_pos.x = x, m_pos.y = y, m_pos.z = z; }
	void SetXYZ(const PIXEL_POSITION& pos) noexcept { m_pos = pos; }

	LPSECTREE GetSectree() const noexcept { return m_pSectree; }
	void SetSectree(LPSECTREE tree) noexcept { m_pSectree = tree; }

	void UpdateSectree();
	void PacketAround(const void* data, int bytes, LPENTITY except = nullptr);
	void PacketView(const void* data, int bytes, LPENTITY except = nullptr);

	void BindDesc(LPDESC _d) noexcept { m_lpDesc = _d; }
	LPDESC GetDesc() const noexcept { return m_lpDesc; }

	void SetMapIndex(long l) { m_lMapIndex = l; }
	long GetMapIndex() const { return m_lMapIndex; }

	void SetObserverMode(bool bFlag);
	bool IsObserverMode() const { return m_bIsObserver; }

protected:
	bool m_bIsObserver;
	bool m_bObserverModeChange;
	ENTITY_MAP m_map_view;
	long m_lMapIndex;

private:
	LPDESC m_lpDesc;

	int m_iType;
	bool m_bIsDestroyed;

	PIXEL_POSITION m_pos;

	int m_iViewAge;

	LPSECTREE m_pSectree;
};

#endif
