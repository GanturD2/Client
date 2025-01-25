///////////////////////////////////////////////////////////////////////
//	Preprocessor
#include "StdAfx.h"

#include <windows.h>
#include "../EterBase/Random.h"
#include "BoundaryShapeManager.h"

///////////////////////////////////////////////////////////////////////
//	CBoundaryShapeManager::CBoundaryShapeManager
CBoundaryShapeManager::CBoundaryShapeManager() = default;


///////////////////////////////////////////////////////////////////////
//	CBoundaryShapeManager::~CBoundaryShapeManager

CBoundaryShapeManager::~CBoundaryShapeManager() = default;


///////////////////////////////////////////////////////////////////////
//	CBoundaryShapeManager::LoadBsfFile

bool CBoundaryShapeManager::LoadBsfFile(const char * pszFilename)
{
	bool bSuccess = true;
	try
	{
		msl::file_ptr fPtr(pszFilename, "rb");
		if (fPtr)
		{
			// number of boundary shapes
			unsigned int nNumBoundaries;
			if (fread(&nNumBoundaries, sizeof(unsigned int), 1, fPtr.get()) == 1)
			{
				for (unsigned int i = 0; i < nNumBoundaries && bSuccess; ++i)
				{
					SBoundaryShape sShape;

					// number of contours for this shape
					unsigned int nNumContours;
					if (fread(&nNumContours, sizeof(unsigned int), 1, fPtr.get()) == 1)
					{
						for (unsigned int j = 0; j < nNumContours && bSuccess; ++j)
						{
							// number of points in this contour
							std::vector<SPoint> vPoints;
							unsigned int nNumPoints;
							if (fread(&nNumPoints, sizeof(unsigned int), 1, fPtr.get()) == 1)
							{
								// read the points
								for (unsigned int k = 0; k < nNumPoints && bSuccess; ++k)
								{
									SPoint sPoint;

									if (fread(sPoint.m_afData, sizeof(float), 3, fPtr.get()) == 3)
									{
										vPoints.emplace_back(sPoint);

										// update extents
										if (j == 0 && k == 0)
										{
											// first point of this shape
											memcpy(sShape.m_afMin, sPoint.m_afData, 3 * sizeof(float));
											memcpy(sShape.m_afMax, sPoint.m_afData, 3 * sizeof(float));
										}
										else
										{
											// check extents
											for (int l = 0; l < 3; ++l)
											{
												if (sPoint.m_afData[l] < sShape.m_afMin[l])
													sShape.m_afMin[l] = sPoint.m_afData[l];
												else if (sPoint.m_afData[l] > sShape.m_afMax[l])
													sShape.m_afMax[l] = sPoint.m_afData[l];
											}
										}
									}
									else
									{
										m_strCurrentError = "Error in CBoundaryShapeManager::LoadBsfFile(): Failed to read point";
										bSuccess = false;
									}
								}
								sShape.m_vContours.emplace_back(vPoints);
							}
							else
							{
								m_strCurrentError = "Error in CBoundaryShapeManager::LoadBsfFile(): Failed to read number of points";
								bSuccess = false;
							}
						}
						m_vBoundaries.emplace_back(sShape);
					}
					else
					{
						m_strCurrentError = "Error in CBoundaryShapeManager::LoadBsfFile(): Failed to read number of contours";
						bSuccess = false;
					}
				}
			}
			else
			{
				m_strCurrentError = "Error in CBoundaryShapeManager::LoadBsfFile(): Failed to read number of boundaries";
				bSuccess = false;
			}
		}
		else
		{
			m_strCurrentError = std::string("Error in CBoundaryShapeManager::LoadBsfFile(): Could not open ") + std::string(pszFilename);
			bSuccess = false;
		}
	}

	catch (...)
	{
		m_strCurrentError = "Error in CBoundaryShapeManager::LoadBsfFile(): Unknown exception";
	}

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//	CBoundaryShapeManager::PointInside

bool CBoundaryShapeManager::PointInside(float fX, float fY)
{
	bool bInside = false;

	for (uint32_t i = 0; i < m_vBoundaries.size() && !bInside; ++i)
		bInside = PointInShape(m_vBoundaries[i], fX, fY);

	return bInside;
}


///////////////////////////////////////////////////////////////////////
//	CBoundaryShapeManager::PointInShape

bool CBoundaryShapeManager::PointInShape(SBoundaryShape & sShape, float fX, float fY)
{
	bool bInside = false;

	for (auto & m_vContour : sShape.m_vContours)
	{
		for (uint32_t i = 0, j = m_vContour.size() - 1; i < m_vContour.size(); j = i++)
		{
			if ((((m_vContour[i][1] <= fY) && (fY < m_vContour[j][1])) || ((m_vContour[j][1] <= fY) && (fY < m_vContour[i][1]))) &&
				(fX < (m_vContour[i][0] - m_vContour[i][0]) * (fY - m_vContour[i][1]) / (m_vContour[j][1] - m_vContour[i][1]) +
					 m_vContour[i][0]))
				bInside = !bInside;
		}
	}

	return bInside;
}


///////////////////////////////////////////////////////////////////////
//	CBoundaryShapeManager::RandomPoint

bool CBoundaryShapeManager::RandomPoint(float & fX, float & fY)
{
	bool bSuccess = false;

	if (!m_vBoundaries.empty())
	{
		// pick a random boundary shape
		int nIndex = random_range(0, m_vBoundaries.size() - 1);
		SBoundaryShape & sShape = m_vBoundaries[nIndex];

		// pick a point at random within its extents
		fX = frandom(sShape.m_afMin[0], sShape.m_afMax[0]);
		fY = frandom(sShape.m_afMin[1], sShape.m_afMax[1]);

		// try it
		bSuccess = PointInShape(sShape, fX, fY);
	}

	return bSuccess;
}
