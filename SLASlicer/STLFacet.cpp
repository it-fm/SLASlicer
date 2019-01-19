#include "STLFacet.h"
#include <string>

//solid OpenSCAD_Model
//	facet normal -1 -0 0
//		outer loop
//			vertex -53.97  53.97 0.79375
//			vertex -53.97 -53.97 0
//			vertex -53.97 -53.97 0.79375
//		endloop
//	endfacet
//	...
//endsolid OpenSCAD_Model

std::istream & operator >> (std::istream &in, Point3D &point)
{
	in >> point[0];
	in >> point[1];
	in >> point[2];

	return in;
}

std::ostream & operator << (std::ostream &out, Point3D &point)
{
	out << point[0] << ' ';
	out << point[1] << ' ';
	out << point[2] << ' ';

	return out;
}

std::istream & operator >> (std::istream &in, STLFacet &patch)
{
	// assumes has already read the "facet" keyword
	std::string keyword1, keyword2;

	in >> keyword1;
	if (keyword1 == "normal")
	{
		in >> patch.normal;

		in >> keyword1 >> keyword2;
		if (keyword1 == "outer" && keyword2 == "loop")
		{
			//Point3D vertices[3];
			for (int i = 0; i < 3; i++)
			{
				in >> keyword1;
				if (keyword1 == "vertex")
				{
					in >> patch.vertices[i];
				}
				else
				{
					in.setstate(std::ios::failbit);
					return in;
				}
			}

			in >> keyword1;
			if (keyword1 != "endloop")
			{
				in.setstate(std::ios::failbit);
				return in;
			}
		}
		else
		{
			in.setstate(std::ios::failbit);
			return in;
		}

		// TODO: compute normal if it is all zeros
		patch.CalcBBox();
		return in;
	}

	in.setstate(std::ios::failbit);
	return in;
}

std::ostream & operator << (std::ostream &out, STLFacet &patch)
{
	out << "facet normal " << patch.normal << std::endl;
	out << "\touter loop" << std::endl;
	for (int i = 0; i < 3; i++)
		out << "\t\tvertex " << patch.vertices[i] << std::endl;
	out << "\tendloop" << std::endl;
	out << "endfacet" << std::endl;

	return out;
}

