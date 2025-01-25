#include "StdAfx.h"

#include <d3d.h> // needed for processing .dds files
#include <cstring>
#include <cassert>

#include "../EterBase/MappedFile.h"
#include "../EterBase/Debug.h"

#include "DXTCImage.h"

struct DXTColBlock
{
	uint16_t col0;
	uint16_t col1;

	// no bit fields - use bytes
	uint8_t row[4];
};

struct DXTAlphaBlockExplicit
{
	uint16_t row[4];
};

struct DXTAlphaBlock3BitLinear
{
	uint8_t alpha0;
	uint8_t alpha1;

	uint8_t stuff[6];
};

// use cast to struct instead of RGBA_MAKE as struct is much
struct Color8888
{
	uint8_t b; // Last one is MSB, 1st is LSB.
	uint8_t g; // order of the output ARGB or BGRA, etc...
	uint8_t r; // change the order of names to change the
	uint8_t a;
};

struct Color565
{
	unsigned nBlue : 5; // order of names changes
	unsigned nGreen : 6; //  byte order of output to 32 bit
	unsigned nRed : 5;
};

/////////////////////////////////////
// should be in ddraw.h
#ifndef MAKEFOURCC
#	define MAKEFOURCC(ch0, ch1, ch2, ch3) \
		((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) | ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24))
#endif // defined(MAKEFOURCC)

CDXTCImage::CDXTCImage()
{
	Initialize();
}

CDXTCImage::~CDXTCImage() = default;

void CDXTCImage::Initialize()
{
	m_nWidth = 0;
	m_nHeight = 0;

	for (auto & m_pbCompBufferByLevel : m_pbCompBufferByLevels)
		m_pbCompBufferByLevel = nullptr;
}

void CDXTCImage::Clear()
{
	for (auto & i : m_bCompVector)
		i.clear();

	Initialize();
}

bool CDXTCImage::LoadFromFile(const char * filename)
{
	// only understands .dds files for now
	// return true if success
	int next = 1;

	static char fileupper[MAX_PATH + 1];

	strncpy(fileupper, filename, MAX_PATH);
	_strupr(fileupper);

	bool knownformat = false;

	for (int i = 0; i < next; ++i)
	{
		char * found = strstr(fileupper, ".DDS");

		if (found != nullptr)
		{
			knownformat = true;
			break;
		}
	}

	if (!knownformat)
	{
		Tracef("Unknown file format encountered! [%s]\n", filename);
		return (false);
	}

	CMappedFile mappedFile;
	LPCVOID pvMap;

	if (!mappedFile.Create(filename, &pvMap, 0, 0))
	{
		Tracef("Can't open file for reading! [%s]\n", filename);
		return false;
	}

	return LoadFromMemory(static_cast<const uint8_t *>(pvMap));
}

bool CDXTCImage::LoadHeaderFromMemory(const uint8_t * c_pbMap)
{
	// Read magic number
	//uint32_t dwMagic = *(uint32_t *) c_pbMap;
	c_pbMap += sizeof(uint32_t);

	//!@#
	//	if (dwMagic != MAKEFOURCC('D','D','S',' '))
	//		return false;

	DDSURFACEDESC2 ddsd; // read from dds file

	// Read the surface description
	memcpy(&ddsd, c_pbMap, sizeof(DDSURFACEDESC2));
	c_pbMap += sizeof(DDSURFACEDESC2);

	// Does texture have mipmaps?
	m_bMipTexture = (ddsd.dwMipMapCount > 0) ? TRUE : FALSE;

	// Clear unwanted flags
	// Can't do this!!!  surface not re-created here
	//    ddsd.dwFlags &= (~DDSD_PITCH);
	//    ddsd.dwFlags &= (~DDSD_LINEARSIZE);

	// Is it DXTC ?
	// I sure hope pixelformat is valid!
	m_xddPixelFormat.dwFlags = ddsd.ddpfPixelFormat.dwFlags;
	m_xddPixelFormat.dwFourCC = ddsd.ddpfPixelFormat.dwFourCC;
	m_xddPixelFormat.dwSize = ddsd.ddpfPixelFormat.dwSize;
	m_xddPixelFormat.dwRGBBitCount = ddsd.ddpfPixelFormat.dwRGBBitCount;
	m_xddPixelFormat.dwRGBAlphaBitMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
	m_xddPixelFormat.dwRBitMask = ddsd.ddpfPixelFormat.dwRBitMask;
	m_xddPixelFormat.dwGBitMask = ddsd.ddpfPixelFormat.dwGBitMask;
	m_xddPixelFormat.dwBBitMask = ddsd.ddpfPixelFormat.dwBBitMask;

	DecodePixelFormat(m_strFormat, &m_xddPixelFormat);

	if (m_CompFormat != PF_DXT1 && m_CompFormat != PF_DXT3 && m_CompFormat != PF_DXT5)
		return false;

	if (ddsd.dwMipMapCount > MAX_MIPLEVELS)
		ddsd.dwMipMapCount = MAX_MIPLEVELS;

	m_nWidth = ddsd.dwWidth;
	m_nHeight = ddsd.dwHeight;
	//!@#
	m_dwMipMapCount = max(1, ddsd.dwMipMapCount);
	m_dwFlags = ddsd.dwFlags;

	if (ddsd.dwFlags & DDSD_PITCH)
	{
		m_lPitch = ddsd.lPitch;
		m_pbCompBufferByLevels[0] = c_pbMap;
	}
	else
	{
		m_lPitch = ddsd.dwLinearSize;

		if (ddsd.dwFlags & DDSD_MIPMAPCOUNT)
		{
			for (uint32_t dwLinearSize = ddsd.dwLinearSize, i = 0; i < m_dwMipMapCount; ++i, dwLinearSize >>= 2)
			{
				m_pbCompBufferByLevels[i] = c_pbMap;
				c_pbMap += dwLinearSize;
			}
		}
		else
			m_pbCompBufferByLevels[0] = c_pbMap;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CDXTCImage::LoadFromMemory(const uint8_t * c_pbMap)
{
	if (!LoadHeaderFromMemory(c_pbMap))
		return false;

	if (m_dwFlags & DDSD_PITCH)
	{
		uint32_t dwBytesPerRow = m_nWidth * m_xddPixelFormat.dwRGBBitCount / 8;

		m_nCompSize = m_lPitch * m_nHeight;
		m_nCompLineSz = dwBytesPerRow;

		m_bCompVector[0].resize(m_nCompSize);
		uint8_t * pDest = &m_bCompVector[0][0];

		c_pbMap = m_pbCompBufferByLevels[0];

		for (int yp = 0; yp < m_nHeight; ++yp)
		{
			memcpy(pDest, c_pbMap, dwBytesPerRow);
			pDest += m_lPitch;
			c_pbMap += m_lPitch;
		}
	}
	else
	{
		if (m_dwFlags & DDSD_MIPMAPCOUNT)
		{
			for (uint32_t dwLinearSize = m_lPitch, i = 0; i < m_dwMipMapCount; ++i, dwLinearSize >>= 2)
			{
				m_bCompVector[i].resize(dwLinearSize);
				Copy(i, &m_bCompVector[i][0], dwLinearSize);
			}
		}
		else
		{
			m_bCompVector[0].resize(m_lPitch);
			Copy(0, &m_bCompVector[0][0], m_lPitch);
		}
	}

	// done reading file
	return true;
}

bool CDXTCImage::Copy(int miplevel, uint8_t * pbDest, long lDestPitch) const
{
	if (!(m_dwFlags & DDSD_MIPMAPCOUNT))
		if (miplevel)
			return false;

	memcpy(pbDest, m_pbCompBufferByLevels[miplevel], m_lPitch >> (miplevel * 2));
	//pbDest += lDestPitch;
	return true;
}

void CDXTCImage::Unextract(uint8_t * pbDest, int /*iWidth*/, int /*iHeight*/, int iPitch)
{
	if (!m_pbCompBufferByLevels[0])
		return;

	auto * pPos = const_cast<uint8_t *>(&m_pbCompBufferByLevels[0][0]);
	int xblocks = m_nWidth / 4;
	int yblocks = (m_nHeight / 4) * ((iPitch / m_nWidth) / 2);

	for (int y = 0; y < yblocks; ++y)
	{
		auto * pBlock = reinterpret_cast<DXTColBlock *>(pPos + y * xblocks * 8);

		memcpy(pbDest, pBlock, xblocks * 8);
		pbDest += xblocks * 8;
	}
}

void CDXTCImage::Decompress(int miplevel, uint32_t * pdwDest)
{
	switch (m_CompFormat)
	{
	case PF_DXT1:
		DecompressDXT1(miplevel, pdwDest);
		break;

	case PF_DXT3:
		DecompressDXT3(miplevel, pdwDest);
		break;

	case PF_DXT5:
		DecompressDXT5(miplevel, pdwDest);
		break;

	case PF_ARGB:
		DecompressARGB(miplevel, pdwDest);
		break;

	case PF_UNKNOWN:
		break;
	case PF_DXT2:
		break;
	case PF_DXT4:
		break;
	default:
		break;
	}
}

inline void GetColorBlockColors(DXTColBlock * pBlock, Color8888 * col_0, Color8888 * col_1, Color8888 * col_2, Color8888 * col_3,
								uint16_t & wrd)
{
	// There are 4 methods to use - see the Time_ functions.
	// 1st = shift = does normal approach per byte for color comps
	// 2nd = use freak variable bit field color565 for component extraction
	// 3rd = use super-freak uint32_t adds BEFORE shifting the color components
	//  This lets you do only 1 add per color instead of 3 uint8_t adds and
	//  might be faster
	// Call RunTimingSession() to run each of them & output result to txt file

	auto * pCol = reinterpret_cast<Color565 *>(&(pBlock->col0));

	col_0->a = 0xff;
	col_0->r = pCol->nRed;
	col_0->r <<= 3; // shift to full precision
	col_0->g = pCol->nGreen;
	col_0->g <<= 2;
	col_0->b = pCol->nBlue;
	col_0->b <<= 3;

	pCol = reinterpret_cast<Color565 *>(&(pBlock->col1));
	col_1->a = 0xff;
	col_1->r = pCol->nRed;
	col_1->r <<= 3; // shift to full precision
	col_1->g = pCol->nGreen;
	col_1->g <<= 2;
	col_1->b = pCol->nBlue;
	col_1->b <<= 3;

	if (pBlock->col0 > pBlock->col1)
	{
		// Four-color block: derive the other two colors.
		// 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
		// These two bit codes correspond to the 2-bit fields
		// stored in the 64-bit block.
		wrd = static_cast<uint16_t>((static_cast<uint16_t>(col_0->r) * 2 + static_cast<uint16_t>(col_1->r)) / 3);
		// no +1 for rounding
		// as bits have been shifted to 888
		col_2->r = static_cast<uint8_t>(wrd);

		wrd = static_cast<uint16_t>((static_cast<uint16_t>(col_0->g) * 2 + static_cast<uint16_t>(col_1->g)) / 3);
		col_2->g = static_cast<uint8_t>(wrd);

		wrd = static_cast<uint16_t>((static_cast<uint16_t>(col_0->b) * 2 + static_cast<uint16_t>(col_1->b)) / 3);
		col_2->b = static_cast<uint8_t>(wrd);
		col_2->a = 0xff;

		wrd = static_cast<uint16_t>((static_cast<uint16_t>(col_0->r) + static_cast<uint16_t>(col_1->r) * 2) / 3);
		col_3->r = static_cast<uint8_t>(wrd);

		wrd = static_cast<uint16_t>((static_cast<uint16_t>(col_0->g) + static_cast<uint16_t>(col_1->g) * 2) / 3);
		col_3->g = static_cast<uint8_t>(wrd);

		wrd = static_cast<uint16_t>((static_cast<uint16_t>(col_0->b) + static_cast<uint16_t>(col_1->b) * 2) / 3);
		col_3->b = static_cast<uint8_t>(wrd);
		col_3->a = 0xff;
	}
	else
	{
		// Three-color block: derive the other color.
		// 00 = color_0,  01 = color_1,  10 = color_2,
		// 11 = transparent.
		// These two bit codes correspond to the 2-bit fields
		// stored in the 64-bit block.

		// explicit for each component, unlike some refrasts...

		// Tracef("block has alpha\n");
		wrd = static_cast<uint16_t>((static_cast<uint16_t>(col_0->r) + static_cast<uint16_t>(col_1->r)) / 2);
		col_2->r = static_cast<uint8_t>(wrd);
		wrd = static_cast<uint16_t>((static_cast<uint16_t>(col_0->g) + static_cast<uint16_t>(col_1->g)) / 2);
		col_2->g = static_cast<uint8_t>(wrd);
		wrd = static_cast<uint16_t>((static_cast<uint16_t>(col_0->b) + static_cast<uint16_t>(col_1->b)) / 2);
		col_2->b = static_cast<uint8_t>(wrd);
		col_2->a = 0xff;

		col_3->r = 0x00; // random color to indicate alpha
		col_3->g = 0x00;
		col_3->b = 0x00;
		col_3->a = 0x00;
	}
} // Get color block colors (...)


inline void DecodeColorBlock(uint32_t * pImPos, DXTColBlock * pColorBlock, int width, const uint32_t * col_0, const uint32_t * col_1,
							 const uint32_t * col_2, const uint32_t * col_3)
{
	// bit masks = 00000011, 00001100, 00110000, 11000000
	const uint32_t masks[] = {3, 12, 3 << 4, 3 << 6};
	const int shift[] = {0, 2, 4, 6};

	// r steps through lines in y
	for (int y = 0; y < 4; ++y, pImPos += width - 4) // no width * 4 as uint32_t ptr inc will * 4
	{
		// width * 4 bytes per pixel per line
		// each j dxtc row is 4 lines of pixels

		// pImPos = (uint32_t*) ((uint32_t) pBase + i * 16 + (y + j * 4) * m_nWidth * 4);

		// n steps through pixels
		for (int n = 0; n < 4; ++n)
		{
			uint32_t bits = pColorBlock->row[y] & masks[n];
			bits >>= shift[n];

			switch (bits)
			{
			case 0:
				*pImPos = *col_0;
				pImPos++; // increment to next uint32_t
				break;

			case 1:
				*pImPos = *col_1;
				pImPos++;
				break;

			case 2:
				*pImPos = *col_2;
				pImPos++;
				break;

			case 3:
				*pImPos = *col_3;
				pImPos++;
				break;

			default:
				Tracef("Your logic is jacked! bits == 0x%x\n", bits);
				pImPos++;
				break;
			}
		}
	}
}

inline void DecodeAlphaExplicit(uint32_t * pImPos, DXTAlphaBlockExplicit * pAlphaBlock, int width, uint32_t alphazero)
{
	// alphazero is a bit mask that when & with the image color
	//  will zero the alpha bits, so if the image DWORDs  are
	//  ARGB then alphazero will be 0x00ffffff or if
	//  RGBA then alphazero will be 0xffffff00
	//  alphazero constructed automaticaly from field order of Color8888 structure

	Color8888 col{};

	//Tracef("\n");
	for (int row = 0; row < 4; row++, pImPos += width - 4)
	{
		// pImPow += pImPos += width-4 moves to next row down
		uint16_t wrd = pAlphaBlock->row[row];

		// Tracef("0x%.8x\t\t", wrd);
		for (int pix = 0; pix < 4; ++pix)
		{
			// zero the alpha bits of image pixel
			*pImPos &= alphazero;

			col.a = static_cast<uint8_t>(wrd & 0x000f); // get only low 4 bits
			//			col.a <<= 4;				// shift to full byte precision
			// NOTE:  with just a << 4 you'll never have alpha
			// of 0xff,  0xf0 is max so pure shift doesn't quite
			// cover full alpha range.
			// It's much cheaper than divide & scale though.
			// To correct for this, and get 0xff for max alpha,
			//  or the low bits back in after left shifting
			col.a = static_cast<uint8_t>(col.a | (col.a << 4)); // This allows max 4 bit alpha to be 0xff alpha
			//  in final image, and is crude approach to full
			//  range scale

			*pImPos |= *reinterpret_cast<uint32_t *>(&col); // or the bits into the prev. nulled alpha

			wrd >>= 4; // move next bits to lowest 4

			pImPos++; // move to next pixel in the row
		}
	}
}

static uint8_t gBits[4][4];
static uint16_t gAlphas[8];
static Color8888 gACol[4][4];

inline void DecodeAlpha3BitLinear(uint32_t * pImPos, DXTAlphaBlock3BitLinear * pAlphaBlock, int width, uint32_t alphazero)
{
	gAlphas[0] = pAlphaBlock->alpha0;
	gAlphas[1] = pAlphaBlock->alpha1;

	// 8-alpha or 6-alpha block?
	if (gAlphas[0] > gAlphas[1])
	{
		// 8-alpha block:  derive the other 6 alphas.
		// 000 = alpha_0, 001 = alpha_1, others are interpolated
		gAlphas[2] = static_cast<uint16_t>((6 * gAlphas[0] + gAlphas[1]) / 7); // Bit code 010
		gAlphas[3] = static_cast<uint16_t>((5 * gAlphas[0] + 2 * gAlphas[1]) / 7); // Bit code 011
		gAlphas[4] = static_cast<uint16_t>((4 * gAlphas[0] + 3 * gAlphas[1]) / 7); // Bit code 100
		gAlphas[5] = static_cast<uint16_t>((3 * gAlphas[0] + 4 * gAlphas[1]) / 7); // Bit code 101
		gAlphas[6] = static_cast<uint16_t>((2 * gAlphas[0] + 5 * gAlphas[1]) / 7); // Bit code 110
		gAlphas[7] = static_cast<uint16_t>((gAlphas[0] + 6 * gAlphas[1]) / 7); // Bit code 111
	}
	else
	{
		// 6-alpha block:  derive the other alphas.
		// 000 = alpha_0, 001 = alpha_1, others are interpolated
		gAlphas[2] = static_cast<uint16_t>((4 * gAlphas[0] + gAlphas[1]) / 5); // Bit code 010
		gAlphas[3] = static_cast<uint16_t>((3 * gAlphas[0] + 2 * gAlphas[1]) / 5); // Bit code 011
		gAlphas[4] = static_cast<uint16_t>((2 * gAlphas[0] + 3 * gAlphas[1]) / 5); // Bit code 100
		gAlphas[5] = static_cast<uint16_t>((gAlphas[0] + 4 * gAlphas[1]) / 5); // Bit code 101
		gAlphas[6] = 0; // Bit code 110
		gAlphas[7] = 255; // Bit code 111
	}

	// Decode 3-bit fields into array of 16 BYTES with same value

	// first two rows of 4 pixels each:
	// pRows = (Alpha3BitRows*) & (pAlphaBlock->stuff[0]);
	const uint32_t mask = 0x00000007; // bits = 00 00 01 11
	uint32_t bits = *(reinterpret_cast<uint32_t *>(&(pAlphaBlock->stuff[0])));

	gBits[0][0] = static_cast<uint8_t>(bits & mask);
	bits >>= 3;
	gBits[0][1] = static_cast<uint8_t>(bits & mask);
	bits >>= 3;
	gBits[0][2] = static_cast<uint8_t>(bits & mask);
	bits >>= 3;
	gBits[0][3] = static_cast<uint8_t>(bits & mask);
	bits >>= 3;
	gBits[1][0] = static_cast<uint8_t>(bits & mask);
	bits >>= 3;
	gBits[1][1] = static_cast<uint8_t>(bits & mask);
	bits >>= 3;
	gBits[1][2] = static_cast<uint8_t>(bits & mask);
	bits >>= 3;
	gBits[1][3] = static_cast<uint8_t>(bits & mask);

	// now for last two rows:
	bits = *(reinterpret_cast<uint32_t *>(&(pAlphaBlock->stuff[3]))); // last 3 bytes

	gBits[2][0] = static_cast<uint8_t>(bits & mask);
	bits >>= 3;
	gBits[2][1] = static_cast<uint8_t>(bits & mask);
	bits >>= 3;
	gBits[2][2] = static_cast<uint8_t>(bits & mask);
	bits >>= 3;
	gBits[2][3] = static_cast<uint8_t>(bits & mask);
	bits >>= 3;
	gBits[3][0] = static_cast<uint8_t>(bits & mask);
	bits >>= 3;
	gBits[3][1] = static_cast<uint8_t>(bits & mask);
	bits >>= 3;
	gBits[3][2] = static_cast<uint8_t>(bits & mask);
	bits >>= 3;
	gBits[3][3] = static_cast<uint8_t>(bits & mask);

	// decode the codes into alpha values
	int row, pix;

	for (row = 0; row < 4; ++row)
	{
		for (pix = 0; pix < 4; ++pix)
		{
			gACol[row][pix].a = static_cast<uint8_t>(gAlphas[gBits[row][pix]]);

			assert(gACol[row][pix].r == 0);
			assert(gACol[row][pix].g == 0);
			assert(gACol[row][pix].b == 0);
		}
	}

	// Write out alpha values to the image bits
	for (row = 0; row < 4; ++row, pImPos += width - 4)
	{
		// pImPow += pImPos += width - 4 moves to next row down
		for (pix = 0; pix < 4; ++pix)
		{
			// zero the alpha bits of image pixel
			*pImPos &= alphazero;
			*pImPos |= *(reinterpret_cast<uint32_t *>(&(gACol[row][pix]))); // or the bits into the prev. nulled alpha
			pImPos++;
		}
	}
}

void CDXTCImage::DecompressDXT1(int miplevel, uint32_t * pdwDest)
{
	// This was hacked up pretty quick & slopily
	// decompresses to 32 bit format 0xARGB
	int xblocks, yblocks;
#ifdef DEBUG
	if ((ddsd.dwWidth % 4) != 0)
		Tracef("****** warning width not div by 4!  %d\n", ddsd.dwWidth);

	if ((ddsd.dwHeight % 4) != 0)
		Tracef("****** warning Height not div by 4! %d\n", ddsd.dwHeight);

	Tracef("end check\n");
#endif
	uint32_t nWidth = m_nWidth >> miplevel;
	uint32_t nHeight = m_nHeight >> miplevel;

	xblocks = nWidth / 4;
	yblocks = nHeight / 4;

	auto * pBase = pdwDest;
	auto * pPos = reinterpret_cast<uint16_t *>(&m_bCompVector[miplevel][0]); // pos in compressed data

	Color8888 col_0, col_1, col_2, col_3;
	uint16_t wrd;

	for (int y = 0; y < yblocks; ++y)
	{
		// 8 bytes per block
		auto * pBlock = reinterpret_cast<DXTColBlock *>((uint32_t) pPos + y * xblocks * 8);

		for (int x = 0; x < xblocks; ++x, ++pBlock)
		{
			// inline func:
			GetColorBlockColors(pBlock, &col_0, &col_1, &col_2, &col_3, wrd);

			auto * pImPos = (uint32_t *) ((uint32_t) pBase + x * 16 + (y * 4) * nWidth * 4);
			DecodeColorBlock(pImPos, pBlock, nWidth, reinterpret_cast<uint32_t *>(&col_0), reinterpret_cast<uint32_t *>(&col_1),
							 reinterpret_cast<uint32_t *>(&col_2), reinterpret_cast<uint32_t *>(&col_3));
			// Set to RGB test pattern
			//	pImPos = (uint32_t*) ((uint32_t) pBase + i * 4 + j * m_nWidth * 4);
			//	*pImPos = ((i * 4) << 16) | ((j * 4) << 8) | ((63 - i) * 4);

			// checkerboard of only col_0 and col_1 basis colors:
			//	pImPos = (uint32_t *) ((uint32_t) pBase + i * 8 + j * m_nWidth * 8);
			//	*pImPos = *((uint32_t *) &col_0);
			//	pImPos += 1 + m_nWidth;
			//	*pImPos = *((uint32_t *) &col_1);
		}
	}
}

void CDXTCImage::DecompressDXT3(int miplevel, uint32_t * pdwDest)
{
	int xblocks, yblocks;
#ifdef DEBUG
	if ((ddsd.dwWidth % 4) != 0)
		Tracef("****** warning width not div by 4! %d\n", ddsd.dwWidth);

	if ((ddsd.dwHeight % 4) != 0)
		Tracef("****** warning Height not div by 4! %d\n", ddsd.dwHeight);

	Tracef("end check\n");
#endif
	uint32_t nWidth = m_nWidth >> miplevel;
	uint32_t nHeight = m_nHeight >> miplevel;

	xblocks = nWidth / 4;
	yblocks = nHeight / 4;

	auto * pBase = pdwDest;
	auto * pPos = reinterpret_cast<uint16_t *>(&m_bCompVector[miplevel][0]); // pos in compressed data

	Color8888 col_0, col_1, col_2, col_3;
	uint16_t wrd;

	// fill alphazero with appropriate value to zero out alpha when
	//  alphazero is ANDed with the image color 32 bit uint32_t:
	col_0.a = 0;
	col_0.r = col_0.g = col_0.b = 0xff;

	uint32_t alphazero = *(reinterpret_cast<uint32_t *>(&col_0));

	for (int y = 0; y < yblocks; ++y)
	{
		// 8 bytes per block
		// 1 block for alpha, 1 block for color
		auto * pBlock = reinterpret_cast<DXTColBlock *>((uint32_t)(pPos + y * xblocks * 16));

		for (int x = 0; x < xblocks; ++x, ++pBlock)
		{
			// inline
			// Get alpha block
			auto * pAlphaBlock = reinterpret_cast<DXTAlphaBlockExplicit *>(pBlock);

			// inline func:
			// Get color block & colors
			pBlock++;
			GetColorBlockColors(pBlock, &col_0, &col_1, &col_2, &col_3, wrd);

			// Decode the color block into the bitmap bits
			// inline func:
			auto * pImPos = (uint32_t *) ((uint32_t)(pBase + x * 16 + (y * 4) * nWidth * 4));

			DecodeColorBlock(pImPos, pBlock, nWidth, reinterpret_cast<uint32_t *>(&col_0), reinterpret_cast<uint32_t *>(&col_1),
							 reinterpret_cast<uint32_t *>(&col_2), reinterpret_cast<uint32_t *>(&col_3));

			// Overwrite the previous alpha bits with the alpha block
			//  info
			// inline func:
			DecodeAlphaExplicit(pImPos, pAlphaBlock, nWidth, alphazero);
		}
	}
}

void CDXTCImage::DecompressDXT5(int level, uint32_t * pdwDest)
{
	int xblocks, yblocks;
#ifdef DEBUG
	if ((ddsd.dwWidth % 4) != 0)
		Tracef("****** warning width not div by 4! %d\n", ddsd.dwWidth);

	if ((ddsd.dwHeight % 4) != 0)
		Tracef("****** warning Height not div by 4! %d\n", ddsd.dwHeight);

	Tracef("end check\n");
#endif
	uint32_t nWidth = m_nWidth >> level;
	uint32_t nHeight = m_nHeight >> level;

	xblocks = nWidth / 4;
	yblocks = nHeight / 4;

	auto * pBase = pdwDest;
	uint16_t * pPos = pPos = reinterpret_cast<uint16_t *>(&m_bCompVector[level][0]); // pos in compressed data

	Color8888 col_0, col_1, col_2, col_3;
	uint16_t wrd;

	// fill alphazero with appropriate value to zero out alpha when
	// alphazero is ANDed with the image color 32 bit uint32_t:
	col_0.a = 0;
	col_0.r = col_0.g = col_0.b = 0xff;
	uint32_t alphazero = *(reinterpret_cast<uint32_t *>(&col_0));

	////////////////////////////////
	// Tracef("blocks: x: %d y: %d\n", xblocks, yblocks);
	for (int y = 0; y < yblocks; ++y)
	{
		// 8 bytes per block
		// 1 block for alpha, 1 block for color
		auto * pBlock = reinterpret_cast<DXTColBlock *>((uint32_t)(pPos + y * xblocks * 16));

		for (int x = 0; x < xblocks; ++x, ++pBlock)
		{
			// inline
			// Get alpha block
			auto * pAlphaBlock = reinterpret_cast<DXTAlphaBlock3BitLinear *>(pBlock);

			// inline func:
			// Get color block & colors
			pBlock++;

			// Tracef("pBlock: 0x%.8x\n", pBlock);
			GetColorBlockColors(pBlock, &col_0, &col_1, &col_2, &col_3, wrd);

			// Decode the color block into the bitmap bits
			// inline func:
			auto * pImPos = (uint32_t *) ((uint32_t)(pBase + x * 16 + (y * 4) * nWidth * 4));

			//DecodeColorBlock(pImPos, pBlock, nWidth, (uint32_t *)&col_0, (uint32_t *)&col_1, (uint32_t *)&col_2, (uint32_t *)&col_3);
			DecodeColorBlock(pImPos, pBlock, nWidth, reinterpret_cast<uint32_t *>(&col_0), reinterpret_cast<uint32_t *>(&col_1),
							 reinterpret_cast<uint32_t *>(&col_2), reinterpret_cast<uint32_t *>(&col_3));

			// Overwrite the previous alpha bits with the alpha block
			//  info
			DecodeAlpha3BitLinear(pImPos, pAlphaBlock, nWidth, alphazero);
		}
	}
} // dxt5

void CDXTCImage::DecompressARGB(int level, uint32_t * pdwDest)
{
	uint32_t lPitch = m_lPitch >> (level * 2);
	memcpy(pdwDest, &m_bCompVector[level][0], lPitch);
}

//-----------------------------------------------------------------------------
// Name: GetNumberOfBits()
// Desc: Returns the number of bits set in a uint32_t mask
//	from microsoft mssdk d3dim sample "Compress"
//-----------------------------------------------------------------------------
static uint16_t GetNumberOfBits(uint32_t dwMask)
{
	uint16_t wBits;
	for (wBits = 0; dwMask; wBits++)
		dwMask = (dwMask & (dwMask - 1));

	return wBits;
}

//-----------------------------------------------------------------------------
// Name: PixelFormatToString()
// Desc: Creates a string describing a pixel format.
//	adapted from microsoft mssdk D3DIM Compress example
//  PixelFormatToString()
//-----------------------------------------------------------------------------
void CDXTCImage::DecodePixelFormat(CHAR * strPixelFormat, XDDPIXELFORMAT * pxddpf)
{
	switch (pxddpf->dwFourCC)
	{
	case 0:
	{
		// This dds texture isn't compressed so write out ARGB format
		uint16_t a = GetNumberOfBits(pxddpf->dwRGBAlphaBitMask);
		uint16_t r = GetNumberOfBits(pxddpf->dwRBitMask);
		uint16_t g = GetNumberOfBits(pxddpf->dwGBitMask);
		uint16_t b = GetNumberOfBits(pxddpf->dwBBitMask);

		_snprintf(strPixelFormat, 31, "ARGB-%d%d%d%d%s", a, r, g, b, pxddpf->dwBBitMask & DDPF_ALPHAPREMULT ? "-premul" : "");
		m_CompFormat = PF_ARGB;
	}
	break;

	case MAKEFOURCC('D', 'X', 'T', '1'):
		strncpy(strPixelFormat, "DXT1", 31);
		m_CompFormat = PF_DXT1;
		break;

	case MAKEFOURCC('D', 'X', 'T', '2'):
		strncpy(strPixelFormat, "DXT2", 31);
		m_CompFormat = PF_DXT2;
		break;

	case MAKEFOURCC('D', 'X', 'T', '3'):
		strncpy(strPixelFormat, "DXT3", 31);
		m_CompFormat = PF_DXT3;
		break;

	case MAKEFOURCC('D', 'X', 'T', '4'):
		strncpy(strPixelFormat, "DXT4", 31);
		m_CompFormat = PF_DXT4;
		break;

	case MAKEFOURCC('D', 'X', 'T', '5'):
		strncpy(strPixelFormat, "DXT5", 31);
		m_CompFormat = PF_DXT5;
		break;

	default:
		strcpy(strPixelFormat, "Format Unknown");
		m_CompFormat = PF_UNKNOWN;
		break;
	}
}
