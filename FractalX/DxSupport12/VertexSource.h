#pragma once

namespace DXF
{
	enum class VertexSource
	{
		Spherical = 0,
		CustomVertexPair = 1,
		Rectangular = 2
	};

	int VertexSourceToInt(VertexSource source);

	VertexSource VertexSourceFromInt(int val);

}