#include "StdAfx.h"
#include "MarkManager.h"

CGuildMarkManager::CGuildMarkManager()
{
#if _MSC_VER >= 1200
	_mkdir("mark");
#endif
	// 남은 mark id 셋을 만든다. (서버용)
	for (uint32_t i = 0; i < MAX_IMAGE_COUNT * CGuildMarkImage::MARK_TOTAL_COUNT; ++i)
		m_setFreeMarkID.emplace(i);
}

CGuildMarkManager::~CGuildMarkManager() = default;

bool CGuildMarkManager::GetMarkImageFilename(uint32_t imgIdx, std::string & path) const
{
	if (imgIdx >= MAX_IMAGE_COUNT)
		return false;

	char buf[64];
	snprintf(buf, sizeof(buf), "mark/%s_%lu.tga", m_pathPrefix.c_str(), imgIdx);
	path = buf;
	return true;
}

void CGuildMarkManager::SetMarkPathPrefix(const char * prefix)
{
	m_pathPrefix = prefix;
}

// 마크 인덱스 불러오기 (서버에서만 사용)
bool CGuildMarkManager::LoadMarkIndex()
{
	std::string path = fmt::format("mark/{}_index", m_pathPrefix);
	msl::file_ptr fPtr(path);

	if (!fPtr)
		return false;

	uint32_t guildID;
	uint32_t markID;

	char line[256];

	while (fgets(line, sizeof(line) - 1, fPtr.get()))
	{
		sscanf(line, "%u %u", &guildID, &markID);
		line[0] = '\0';
		AddMarkIDByGuildID(guildID, markID);
	}

	LoadMarkImages();
	return true;
}

bool CGuildMarkManager::SaveMarkIndex()
{
	msl::file_ptr fPtr(fmt::format("mark/{}_index", m_pathPrefix), "w");
	if (!fPtr)
	{
		TraceError("MarkManager::SaveMarkData: cannot open index file.");
		return false;
	}

	for (auto & it : m_mapGID_MarkID)
		fprintf(fPtr.get(), "%u %u\n", it.first, it.second);

	Tracenf("MarkManager::SaveMarkData: index count %d", m_mapGID_MarkID.size());
	return true;
}

void CGuildMarkManager::LoadMarkImages()
{
	bool isMarkExists[MAX_IMAGE_COUNT]{};

	for (auto & it : m_mapGID_MarkID)
	{
		uint32_t markID = it.second;

		if (markID < MAX_IMAGE_COUNT * CGuildMarkImage::MARK_TOTAL_COUNT)
			isMarkExists[markID / CGuildMarkImage::MARK_TOTAL_COUNT] = true;
	}

	for (uint32_t i = 0; i < MAX_IMAGE_COUNT; ++i)
	{
		if (isMarkExists[i])
			__GetImage(i);
	}
}

void CGuildMarkManager::SaveMarkImage(uint32_t imgIdx)
{
	std::string path;

	if (GetMarkImageFilename(imgIdx, path))
	{
		if (!__GetImage(imgIdx)->Save(path.c_str()))
			TraceError("%s Save failed\n", path.c_str());
	}
}

CGuildMarkImage * CGuildMarkManager::__GetImage(uint32_t imgIdx)
{
	if (auto it = m_mapIdx_Image.find(imgIdx); it != m_mapIdx_Image.end())
		return it->second.get();

	std::string imagePath;
	if (GetMarkImageFilename(imgIdx, imagePath))
	{
		auto pkImage = std::make_unique<CGuildMarkImage>();
		auto pkImagePtr = pkImage.get();
		pkImage->Load(imagePath.c_str());
		m_mapIdx_Image.emplace(imgIdx, std::move(pkImage));
		return pkImagePtr;
	}
	return nullptr;
}

bool CGuildMarkManager::AddMarkIDByGuildID(uint32_t guildID, uint32_t markID)
{
	if (markID >= MAX_IMAGE_COUNT * CGuildMarkImage::MARK_TOTAL_COUNT)
		return false;

	m_mapGID_MarkID.emplace(guildID, markID);
	m_setFreeMarkID.erase(markID);
	return true;
}

uint32_t CGuildMarkManager::GetMarkID(uint32_t guildID)
{
	auto it = m_mapGID_MarkID.find(guildID);

	if (it == m_mapGID_MarkID.end())
		return INVALID_MARK_ID;

	return it->second;
}

uint32_t CGuildMarkManager::__AllocMarkID(uint32_t guildID)
{
	auto it = m_setFreeMarkID.lower_bound(0);

	if (it == m_setFreeMarkID.end())
		return INVALID_MARK_ID;

	uint32_t markID = *it;

	uint32_t imgIdx = markID / CGuildMarkImage::MARK_TOTAL_COUNT;
	CGuildMarkImage * pkImage = __GetImage(imgIdx); // 이미지가 없다면 만들기 위해

	if (pkImage && AddMarkIDByGuildID(guildID, markID))
		return markID;

	return INVALID_MARK_ID;
}

uint32_t CGuildMarkManager::GetMarkImageCount() const
{
	return m_mapIdx_Image.size();
}

uint32_t CGuildMarkManager::GetMarkCount() const
{
	return m_mapGID_MarkID.size();
}

// SERVER
void CGuildMarkManager::CopyMarkIdx(char * pcBuf) const
{
	auto * pwBuf = reinterpret_cast<uint16_t *>(pcBuf);

	for (const auto & it : m_mapGID_MarkID)
	{
		*(pwBuf++) = it.first; // guild id
		*(pwBuf++) = it.second; // mark id
	}
}

// SERVER
uint32_t CGuildMarkManager::SaveMark(uint32_t guildID, uint8_t * pbMarkImage)
{
	uint32_t idMark;

	if ((idMark = GetMarkID(guildID)) == INVALID_MARK_ID)
	{
		if ((idMark = __AllocMarkID(guildID)) == INVALID_MARK_ID)
		{
			TraceError("CGuildMarkManager: cannot alloc mark id %u", guildID);
			return false;
		}

		Tracenf("SaveMark: mark id alloc %u", idMark);
	}
	else
		Tracenf("SaveMark: mark id found %u", idMark);

	uint32_t imgIdx = (idMark / CGuildMarkImage::MARK_TOTAL_COUNT);
	CGuildMarkImage * pkImage = __GetImage(imgIdx);

	if (pkImage)
	{
		printf("imgidx %u ", imgIdx);

		std::string pathImage;
		GetMarkImageFilename(imgIdx, pathImage);
		pkImage->Save(pathImage.c_str());

		SaveMarkIndex();
	}

	return idMark;
}

// SERVER
void CGuildMarkManager::DeleteMark(uint32_t guildID)
{
	auto it = m_mapGID_MarkID.find(guildID);

	if (it == m_mapGID_MarkID.end())
		return;

	CGuildMarkImage * pkImage;

	if ((pkImage = __GetImage(it->second / CGuildMarkImage::MARK_TOTAL_COUNT)) != nullptr)
		pkImage->DeleteMark(it->second % CGuildMarkImage::MARK_TOTAL_COUNT);

	m_mapGID_MarkID.erase(it);
	m_setFreeMarkID.emplace(it->second);

	SaveMarkIndex();
}

// SERVER
void CGuildMarkManager::GetDiffBlocks(uint32_t imgIdx, const uint32_t * crcList, std::map<uint8_t, const SGuildMarkBlock *> & mapDiffBlocks)
{
	mapDiffBlocks.clear();

	// 클라이언트에서 서버에 없는 이미지를 요청할 수는 없다.
	if (m_mapIdx_Image.end() == m_mapIdx_Image.find(imgIdx))
	{
		Tracenf("invalid idx %u", imgIdx); // @warme668
		return;
	}

	CGuildMarkImage * p = __GetImage(imgIdx);

	if (p)
		p->GetDiffBlocks(crcList, mapDiffBlocks);
}

// CLIENT
bool CGuildMarkManager::SaveBlockFromCompressedData(uint32_t imgIdx, uint32_t posBlock, const uint8_t * pbBlock, uint32_t dwSize)
{
	CGuildMarkImage * pkImage = __GetImage(imgIdx);

	if (pkImage)
		pkImage->SaveBlockFromCompressedData(posBlock, pbBlock, dwSize);

	return false;
}

// CLIENT
bool CGuildMarkManager::GetBlockCRCList(uint32_t imgIdx, uint32_t * crcList)
{
	// 클라이언트에서 서버에 없는 이미지를 요청할 수는 없다.
	if (m_mapIdx_Image.end() == m_mapIdx_Image.find(imgIdx))
	{
		Tracenf("invalid idx %u", imgIdx); // @warme668
		return false;
	}

	CGuildMarkImage * p = __GetImage(imgIdx);

	if (p)
		p->GetBlockCRCList(crcList);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////
// Symbol
///////////////////////////////////////////////////////////////////////////////////////
const CGuildMarkManager::TGuildSymbol * CGuildMarkManager::GetGuildSymbol(uint32_t guildID)
{
	auto it = m_mapSymbol.find(guildID);

	if (it == m_mapSymbol.end())
		return nullptr;

	return &it->second;
}

bool CGuildMarkManager::LoadSymbol(const char * filename)
{
	msl::file_ptr fPtr(filename, "rb");
	if (!fPtr)
		return true;

	uint32_t symbolCount;
	fread(&symbolCount, 4, 1, fPtr.get());

	for (uint32_t i = 0; i < symbolCount; i++)
	{
		uint32_t guildID;
		uint32_t dwSize;
		fread(&guildID, 4, 1, fPtr.get());
		fread(&dwSize, 4, 1, fPtr.get());

		TGuildSymbol gs;
		gs.raw.resize(dwSize);
		fread(&gs.raw[0], 1, dwSize, fPtr.get());
		gs.crc = GetCRC32(reinterpret_cast<const char *>(&gs.raw[0]), dwSize);
		m_mapSymbol.emplace(guildID, gs);
	}
	return true;
}

void CGuildMarkManager::SaveSymbol(const char * filename)
{
	msl::file_ptr fPtr(filename, "wb");
	if (!fPtr)
	{
		TraceError("Cannot open Symbol file (name: %s)", filename);
		return;
	}

	uint32_t symbolCount = m_mapSymbol.size();
	fwrite(&symbolCount, 4, 1, fPtr.get());

	for (auto & it : m_mapSymbol)
	{
		uint32_t guildID = it.first;
		uint32_t dwSize = it.second.raw.size();
		fwrite(&guildID, 4, 1, fPtr.get());
		fwrite(&dwSize, 4, 1, fPtr.get());
		fwrite(&it.second.raw[0], 1, dwSize, fPtr.get());
	}
}

void CGuildMarkManager::UploadSymbol(uint32_t guildID, int iSize, const uint8_t * pbyData)
{
	Tracenf("GuildSymbolUpload guildID %lu Size %d", guildID, iSize);

	if (m_mapSymbol.find(guildID) == m_mapSymbol.end())
		m_mapSymbol.emplace(guildID, TGuildSymbol());

	TGuildSymbol & rSymbol = m_mapSymbol[guildID];
	rSymbol.raw.clear();

	if (iSize > 0)
	{
		rSymbol.raw.reserve(iSize);
		std::copy(pbyData, (pbyData + iSize), std::back_inserter(rSymbol.raw));
		rSymbol.crc = GetCRC32(reinterpret_cast<const char *>(pbyData), iSize);
	}
}
