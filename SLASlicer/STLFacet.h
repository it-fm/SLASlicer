#ifndef STLFACET_H
#define STLFACET_H

#include <iostream>
#include <cmath>
#include <array>

using fp_t = float;

using Point3D = std::array<fp_t, 3>;
using Vector3D = Point3D;

class STLFacet
{
private:
	Point3D		bbMin;
	Point3D		bbMax;

	float		ReadFloat(std::istream &in)
	{
		// assumes correct "endianness"
		float  value;
		in.read((char *)&value, sizeof(value));
		return value;
	}

	Point3D		ReadPoint3D(std::istream &in)
	{
		Point3D pt;
		pt[0] = ReadFloat(in);
		pt[1] = ReadFloat(in);
		pt[2] = ReadFloat(in);

		return pt;
	}

	fp_t		BBMinValue(unsigned int index) { return std::min(std::min(vertices[0][index], vertices[1][index]), vertices[2][index]); };
	fp_t		BBMaxValue(unsigned int index) { return std::max(std::max(vertices[0][index], vertices[1][index]), vertices[2][index]); };

	void		CalcBBox()
	{
		for (int i = 0; i < 3; i++)
		{
			bbMin[i] = BBMinValue(i);
			bbMax[i] = BBMaxValue(i);
		}
	}

public:
	Point3D		vertices[3];
	Vector3D	normal;
	fp_t		xIntercept;

	STLFacet()
	{
		STLFacet( { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 } );
	}
	STLFacet(const Point3D &v1, const Point3D &v2, const Point3D &v3, const Point3D &n)
	{
		vertices[0] = v1;
		vertices[1] = v2;
		vertices[2] = v3;
		normal = n;
		xIntercept = 0;
		CalcBBox();
	}

	~STLFacet()
	{
	}

	fp_t BBMin(int index) { return bbMin[index]; }
	fp_t BBMax(int index) { return bbMax[index]; }

	bool ReadBinary(std::istream &in)
	{
		normal = ReadPoint3D(in);
		vertices[0] = ReadPoint3D(in);
		vertices[1] = ReadPoint3D(in);
		vertices[2] = ReadPoint3D(in);
		CalcBBox();
	}

	void Scale(fp_t xScale, fp_t yScale, fp_t zScale)
	{
		for (int i = 0; i < 3; i++)
		{
			vertices[i][0] *= xScale;
			vertices[i][1] *= yScale;
			vertices[i][2] *= zScale;
		}
		CalcBBox();
	}

	void Translate(fp_t xShift, fp_t yShift, fp_t zShift)
	{
		for (int i = 0; i < 3; i++)
		{
			vertices[i][0] += xShift;
			vertices[i][1] += yShift;
			vertices[i][2] += zShift;
		}
		CalcBBox();
	}

	// original source:
	// https://wrf.ecse.rpi.edu//Research/Short_Notes/pnpoly.html
	//
	//int pnpoly(int nvert, float *vertx, float *verty, float testx, float testy)
	//{
	//	int i, j, c = 0;
	//	for (i = 0, j = nvert - 1; i < nvert; j = i++) {
	//		if (((verty[i] > testy) != (verty[j] > testy)) &&
	//			(testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
	//			c = !c;
	//	}
	//	return c;
	//}
	const bool InsideYZ(fp_t yCoordinate, fp_t zCoordinate, fp_t &xIntersection)
	{
		bool inside = false;
		int i, j;
		for (i = 0, j = 2; i < 3; j = i++) {
			if (((vertices[i][2] > zCoordinate) != (vertices[j][2] > zCoordinate)) &&
				(yCoordinate < (vertices[j][1] - vertices[i][1]) * (zCoordinate - vertices[i][2]) / (vertices[j][2] - vertices[i][2]) + vertices[i][1]))
				inside = !inside;
		}

		if (inside)
		{
			// the line going through [0, yCoordinate, zCoordinate] in the direction parallel to the x-axis intersects with the facet, now determine where on the line it does so
			// use the translated coordinate system so the line now *is* the x-axis
			//
			// the algorithm is based on the fact that any vector laying on the plane determined by the facet, when projected onto the plane's (unit) normal, will be zero
			// and that the x-intercept in the translated coordinate system will have form: [x-intercept, 0, 0]
			// so dot((vertex - [x-intercept, 0, 0]), normal) = 0 where we choose vertices[0] as the vertex arbitrarily

			if (normal[0] != 0)
			{
				xIntercept  = vertices[0][0] * normal[0];
				xIntercept += (vertices[0][1] - yCoordinate) * normal[1];
				xIntercept += (vertices[0][2] - zCoordinate) * normal[2];
				xIntercept /= normal[0];
			}
			else
				xIntercept = BBMax(0);		// TODO -- is this what I want to do?
		}

		return inside;
	}

	friend std::istream & operator >> (std::istream &in, STLFacet &patch);
	friend std::ostream & operator << (std::ostream &out, STLFacet &patch);
};

#endif
