#pragma once

#include "OriginTarget.h"
#include "SeedTriangles.h"
#include "VertexSource.h"
#include "Vertex.h"

namespace DXF
{
	struct ModelData
	{
		static const int ModelDataVersion = 3;
		int VertexIterations = 5;
		SeedTriangles TriangleSeeds = SeedTriangles::Eight;
		VertexSource SourceVertices = VertexSource::Spherical;
		Vertex<float> VertexTL;		// top left
		Vertex<float> VertexTR;		// top right
		Vertex<float> VertexBL;		// bottom left
		Vertex<float> VertexBR;		// bottom right
		int VerticesWide = 1000;	// For Rectangular VertexSource
		int VerticesHigh = 1000;	// For Rectangular VertexSource
		double RectHalfWidth = 1.0;	// For Rectangular VertexSource 
		double RectHalfHeight = 1.0;// For Rectangular VertexSource	
		OriginTarget OriginTargetType = OriginTarget::Point;
	};

	void Serialize(CArchive& ar, ModelData& modelData);
}