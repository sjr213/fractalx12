#include "pch.h"
#include "ModelData.h"

#include <assert.h>
#include "SerializationException.h"
#include "VertexSerialization.h"

namespace DXF
{
	// version 2 - added VertexSource.Rectangular, RectWidth, RectHeight, and OriginTarget
	void Serialize(CArchive& ar, ModelData& modelData)
	{
		if (ar.IsStoring())
		{
			ar << ModelData::ModelDataVersion;
			ar << modelData.VertexIterations;
			ar << GetIndexForSeedTriangle(modelData.TriangleSeeds);
			ar << VertexSourceToInt(modelData.SourceVertices);
			Serialize(ar, modelData.VertexTL);
			Serialize(ar, modelData.VertexTR);
			Serialize(ar, modelData.VertexBL);
			Serialize(ar, modelData.VertexBR);
			ar << modelData.VerticesWide;
			ar << modelData.VerticesHigh;	
			ar << OriginTargetToInt(modelData.OriginTargetType);
			ar << modelData.RectHalfWidth;
			ar << modelData.RectHalfHeight;
		}
		else
		{
			int version = 0;
			ar >> version;

			if (version < 1)
				throw DxCore::CSerializationException("ModelData version less than 1");

			if (version > ModelData::ModelDataVersion)
				throw DxCore::CSerializationException("ModelData version greater than ModelDataVersion");

			ar >> modelData.VertexIterations;

			int seedType = 0;
			ar >> seedType;

			modelData.TriangleSeeds = GetSeedTriangleFromIndex(seedType);

			int nVertexSource = 0;
			ar >> nVertexSource;
			modelData.SourceVertices = VertexSourceFromInt(nVertexSource);
			Serialize(ar, modelData.VertexTL);
			Serialize(ar, modelData.VertexTR);
			Serialize(ar, modelData.VertexBL);
			Serialize(ar, modelData.VertexBR);

			if (version > 1)
			{
				ar >> modelData.VerticesWide;
				ar >> modelData.VerticesHigh;
				int originTargetType = 1;
				ar >> originTargetType;
				modelData.OriginTargetType = OriginTargetFromInt(originTargetType);
			}

			if (version > 2)
			{
				ar >> modelData.RectHalfWidth;
				ar >> modelData.RectHalfHeight;
			}
		}
	}
}