#include "STLGeometry.h"
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

istream & operator >> (istream &in, STLGeometry &geom)
{
	string keyword;
	in >> keyword;

	if (keyword == "solid")
	{
		// get the (optional) name -- on the same line
		char geometryName[MAX_LINE_LENGTH];
		in.getline(geometryName, MAX_LINE_LENGTH);
		
		while (!(in >> keyword).eof())
		{
			if (keyword == "facet")
			{
				STLFacet patch;
				in >> patch;
				geom.facets.push_back(patch);

				in >> keyword;
				if (keyword != "endfacet")
				{
					throw new logic_error("Bad STL File Format");
				}
			}
			else if (keyword == "endsolid")
			{
				return in;
			}
		}
		return in;
	}
	else
	{
		throw new logic_error("Bad STL File Format");
	}
}

ostream & operator << (ostream &out, const STLGeometry &geom)
{
	out << "solid " << endl;
	for (auto facet : geom.facets)
		out << facet;
	out << "endsolid" << endl;
	return out;
}


