#include "pch.h"
#include "VertexSource.h"

namespace DXF
{
	int VertexSourceToInt(VertexSource source)
	{
		if (source == VertexSource::Spherical)
			return 0;
		if (source == VertexSource::CustomVertexPair)
			return 1;
		if (source == VertexSource::Rectangular)
			return 2;

		return 0;
	}

	VertexSource VertexSourceFromInt(int val)
	{
		if (val == 0)
			return VertexSource::Spherical;
		if (val == 1)
			return VertexSource::CustomVertexPair;
		if (val == 2)
			return VertexSource::Rectangular;

		return VertexSource::Spherical;
	}
}