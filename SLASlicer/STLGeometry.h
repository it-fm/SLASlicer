#ifndef STLGEOM_H
#define STLGEOM_H

#include <list>
#include <vector>
#include <iostream>

using namespace std;

// example:
//	solid OpenSCAD_Model
//		facet normal -1 -0 0
//			outer loop
//				vertex -53.97  53.97 0.79375
//				vertex -53.97 -53.97 0
//				vertex -53.97 -53.97 0.79375
//			endloop
//		endfacet
//		...
//	endsolid OpenSCAD_Model

// TODO: deteremin if file is binary or text by looking for token "solid" at the start of the file

constexpr auto MAX_LINE_LENGTH = 512;

#include "STLFacet.h"

class STLGeometry
{
public:
	vector<STLFacet>	facets;

	STLGeometry() :
		facets()
	{}

	// TODO: determine if the file is text or binary by looking for the "solid" keyword at start
	bool ReadBinary(istream &in)
	{
		constexpr auto HEADER_LENGTH = 80;

		char		header[HEADER_LENGTH];
		uint32_t	facetCount;
		uint16_t	attributeBytes;

		in.read(header, HEADER_LENGTH);										// just discard

		in.read((char *)&facetCount, sizeof(facetCount));

		for (uint32_t i = 0; i < facetCount; i++)
		{
			STLFacet newFacet;
			newFacet.ReadBinary(in);

			facets.push_back(newFacet);
		}

		in.read((char *)&attributeBytes, sizeof(attributeBytes));

		return (attributeBytes == 0);
	}

	bool Inside(const Point3D &pt)
	{
		// Strategy:
		//    1) Extend a ray from the point to (eg) +X infinity
		//    2) Count the # of times the ray intersects a facet.
		//    3) The point is "inside" facet if the dot product of the normal vector with the ray (defined to be parallel to the x-axis) is positive --> if the X-component of the normal is positive
		//    4) count the # of crossings that are inside vs outside.  If the inside count is > outside count, then the point is inside the shell.
		//
		//    Any point lying on the facet is considered INSIDE (regardless of normal direction)

		int	insideCount  = 0;
		int outsideCount = 0;
		for (auto facet : facets)
		{
			fp_t xIntersection;
			outsideCount++;
			if (facet.InsideYZ(pt[1], pt[2], xIntersection))
			{
				if (facet.normal[0] >= 0)
					insideCount++;
				else outsideCount++;
			}
		}

		return (insideCount > outsideCount);
	}

	void Scale(fp_t xScale, fp_t yScale, fp_t zScale)
	{
		for (auto &facet : facets)
			facet.Scale(xScale, yScale, zScale);
	}

	void Translate(fp_t xShift, fp_t yShift, fp_t zShift)
	{
		for (auto &facet : facets)
			facet.Scale(xShift, yShift, zShift);
	}

	friend istream & operator >> (istream &in,  STLGeometry &geom);
	friend ostream & operator << (ostream &out, STLGeometry &geom);
};

#endif

