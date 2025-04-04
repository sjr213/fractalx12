#pragma  once

#include "vector"
#include "VertexTypes.h"
#include "TraceParams.h"

namespace DXF
{
	struct DxVertexData
	{
		static const int VertexVersion = 1;
		std::vector<DirectX::VertexPositionNormalTexture> Vertices;
		std::vector<unsigned int> Indices;
		StretchDistanceParams StretchParams;
	};
}