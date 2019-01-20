#ifndef STLSLICER_H
#define STLSLICER_H

#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>

#include "STLGeometry.h"

constexpr auto OUTSIDE_VALUE = 0x00;
constexpr auto INSIDE_VALUE = UCHAR_MAX;

class STLSlicer
{
private:
	array<size_t, 3>	volSize;			// voxels
	Point3D				buildVol;			// mm
	Point3D				origin;
	Point3D				voxelSize;
	double				maxZ;

	STLGeometry*	geometryPtr;

	static bool sortFacetZMin(STLFacet &facet1, STLFacet &facet2)
	{
		return facet1.BBMin(2) < facet2.BBMin(2);
	}

	static bool sortFacetZMax(STLFacet &facet1, STLFacet &facet2)
	{
		return facet1.BBMax(2) < facet2.BBMax(2);
	}

	static bool reverseSortFacetX(STLFacet &facet1, STLFacet &facet2)
	{
		return facet1.xIntercept > facet2.xIntercept;
	}

	bool inside(fp_t xPosition, const vector<STLFacet> &facets)
	{
		int insideCount = 0;
		for (auto fItr = facets.begin(); fItr != facets.end(); fItr++)
		{
			if (fItr->xIntercept < xPosition)
				break;

			if (fItr->normal[0] > 0)
				insideCount++;
			else
				insideCount--;
		}

		return (insideCount > 0);
	}

	void CalcMaxZ()
	{
		maxZ = max_element(geometryPtr->facets.begin(), geometryPtr->facets.end(), sortFacetZMax)->BBMax(2);
	}

public:

	// Note: this sorts the facets in the geometry
	STLSlicer(STLGeometry *theGeometry, array<size_t, 3> theVolSize, Point3D theBuildVol, Point3D theOrigin, Point3D theVoxelSize) :
		geometryPtr(theGeometry),
		volSize(theVolSize),
		buildVol(theBuildVol),
		origin(theOrigin),
		voxelSize(theVoxelSize)
	{
		sort(geometryPtr->facets.begin(), geometryPtr->facets.end(), sortFacetZMin);
		CalcMaxZ();
	}

	bool Slice(cv::Mat *slice, fp_t zPosition)
	{
		slice->setTo(OUTSIDE_VALUE);

		// just return (false) if the z Position is above the top of the object
		if (zPosition > maxZ)
			return false;

		// first - find all facets that cover the correct Z position
		vector<STLFacet> zFacets;
		for (uint i = 0; i < geometryPtr->facets.size(); i++)
		{
			auto zMinFacet = geometryPtr->facets[i];
			auto zMinVal = zMinFacet.BBMin(2);
			if (zMinVal > zPosition)
				break;

			auto zMaxVal = zMinFacet.BBMax(2);
			if (zMaxVal > zPosition)
				zFacets.push_back(zMinFacet);
		}

		// next calculate the bounding box (in XY plane) of those facets in the YZ plane
		auto minX = zFacets[0].BBMin(0);
		auto maxX = zFacets[0].BBMax(0);
		auto minY = zFacets[0].BBMin(1);
		auto maxY = zFacets[0].BBMax(1);
		for (auto fItr = zFacets.begin(); fItr != zFacets.end(); fItr++)
		{
			minX = min(minX, fItr->BBMin(0));
			maxX = max(maxX, fItr->BBMax(0));
			minY = min(minY, fItr->BBMin(1));
			maxY = max(maxY, fItr->BBMax(1));
		}

		fp_t yPosition = -origin[1] * voxelSize[1];
		for (uint y = 0; y < volSize[1]; y++)
		{
			// ignore voxels outside of the BBox
			if (yPosition < minY || yPosition > maxY)
			{
				yPosition += voxelSize[1];
				continue;
			}

			// find all facets that intersect the line @ (yPosition, zPosition)
			vector<STLFacet> yzFacets;
			for (auto fItr = zFacets.begin(); fItr != zFacets.end(); fItr++)
			{
				fp_t xIntersection;
				if (fItr->InsideYZ(yPosition, zPosition, xIntersection))
				{
					yzFacets.push_back(*fItr);
				}
			}

			// sort the facets in ascending x order (at the given voxel position)
			sort(yzFacets.begin(), yzFacets.end(), reverseSortFacetX);

			// TODO: use a threadpool to parallalize the computation of each y-column
			fp_t xPosition = -origin[0] * voxelSize[0];
			for (uint x = 0; x < volSize[0]; x++)
			{
				// ignore voxels outside of the BBox
				if (xPosition < minX || xPosition > maxX)
				{
					xPosition += voxelSize[0];
					continue;
				}

				if (inside(xPosition, yzFacets))
					slice->at<uchar>(y, x) = INSIDE_VALUE;

				xPosition += voxelSize[0];
			}
			yPosition += voxelSize[1];
		}

		return true;
	}
};

#endif
