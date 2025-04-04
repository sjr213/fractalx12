#include "pch.h"
#include "DxSerialize.h"

#include "BackgroundVertexData.h"
#include "SerializationException.h"
#include "Perspective.h"
#include "RotationGroup.h"
#include "RotationParams.h"
#include "SphereApproximator.h"
#include "TraceParamsSerialize.h"
#include "VertexData.h"

namespace DXF
{
	bool IsVersionSupported(int version, int minVersion, int maxVersion)
	{
		return (version >= minVersion && version <= maxVersion);
	}

	void Serialize(CArchive& ar, bool& val)
	{
		int num = val;
		if (ar.IsStoring())
			ar << num;
		else
		{
			ar >> num;
			val = static_cast<bool>(num);
		}
	}

	void Serialize(CArchive& ar, size_t& n)
	{
		if (ar.IsStoring())
			ar << n;
		else
			ar >> n;
	}

	void Serialize(CArchive& ar, std::wstring& str)
	{
		CString cstr = str.c_str();
		if (ar.IsStoring())
			ar << cstr;
		else
		{
			ar >> cstr;
			str = cstr.GetString();
		}
	}

	void Serialize(CArchive& ar, unsigned int& n)
	{
		if (ar.IsStoring())
			ar << n;
		else
			ar >> n;
	}

	static void Serialize(CArchive& ar, DirectX::XMFLOAT2& xf2)
	{
		if (ar.IsStoring())
		{
			ar << xf2.x << xf2.y;
		}
		else
		{
			ar >> xf2.x >> xf2.y;
		}
	}

	void Serialize(CArchive& ar, DirectX::XMFLOAT3& xf3)
	{
		if (ar.IsStoring())
		{
			ar << xf3.x << xf3.y << xf3.z;
		}
		else
		{
			ar >> xf3.x >> xf3.y >> xf3.z;
		}
	}

	static void Serialize(CArchive& ar, Triangle& triangle)
	{
		if (ar.IsStoring())
		{
			ar << triangle.one << triangle.two << triangle.three;
		}
		else
		{
			ar >> triangle.one >> triangle.two >> triangle.three;
		}
	}

	void Serialize(CArchive& ar, DirectX::VertexPositionNormalTexture& vpnt)
	{
		Serialize(ar, vpnt.position);
		Serialize(ar, vpnt.normal);
		Serialize(ar, vpnt.textureCoordinate);
	}

	void Serialize(CArchive& ar, DxVertexData& vertexData)
	{
		if (ar.IsStoring())
		{
			ar << DxVertexData::VertexVersion;
		}
		else
		{
			int version = 0;
			ar >> version;

			assert(version == DxVertexData::VertexVersion);
			if (version != DxVertexData::VertexVersion)
				throw DxCore::CSerializationException("VertexData version not supported");
		}

		Serialize(ar, vertexData.Vertices);
		Serialize(ar, vertexData.Indices);
		Serialize(ar, vertexData.StretchParams);
	}

	void Serialize(CArchive& ar, DxBackgroundVertexData& bkgndVertexData)
	{
		if (!bkgndVertexData.VertexData)
			throw DxCore::CSerializationException("Background vertex missing");

		if (ar.IsStoring())
		{
			ar << DxBackgroundVertexData::BkgndVertexVersion;
		}
		else
		{
			int version = DxBackgroundVertexData::BkgndVertexVersion;
			ar >> version;

			if (version != DxBackgroundVertexData::BkgndVertexVersion)
				throw DxCore::CSerializationException("Background vertex version not supported");
		}

		Serialize(ar, *bkgndVertexData.VertexData);
	}

	void Serialize(CArchive& ar, TriangleData& triangles)
	{
		const int TriangleVersion = 1;

		if (ar.IsStoring())
		{
			ar << TriangleVersion;
		}
		else
		{
			int version = 0;
			ar >> version;

			if (version != TriangleVersion)
				throw DxCore::CSerializationException("Triangle data version less than 1");
		}

		Serialize(ar, triangles.Triangles);
		Serialize(ar, triangles.Vertices);
	}

	void Serialize(CArchive& ar, RotationParams& rotationParams)
	{
		int version = RotationParams::RotationVersion;

		if (ar.IsStoring())
		{
			ar << version;
			ar << RotationActionToInt(rotationParams.Action);
			ar << rotationParams.AngleXDegrees << rotationParams.AngleYDegrees << rotationParams.AngleZDegrees;
		}
		else
		{
			ar >> version;

			if (version != RotationParams::RotationVersion)
				throw DxCore::CSerializationException("Rotation param version not supported");

			int action = 0;
			ar >> action;
			rotationParams.Action = RotationActionFromInt(action);
			ar >> rotationParams.AngleXDegrees >> rotationParams.AngleYDegrees >> rotationParams.AngleZDegrees;
		}
	}

	void Serialize(CArchive& ar, std::tuple<float, float, float>& f3)
	{
		if (ar.IsStoring())
		{
			ar << std::get<0>(f3) << std::get<1>(f3) << std::get<2>(f3);
		}
		else
		{
			ar >> std::get<0>(f3) >> std::get<1>(f3) >> std::get<2>(f3);
		}
	}

	void Serialize(CArchive& ar, std::tuple<double, double, double>& d3)
	{
		if (ar.IsStoring())
		{
			ar << std::get<0>(d3) << std::get<1>(d3) << std::get<2>(d3);
		}
		else
		{
			ar >> std::get<0>(d3) >> std::get<1>(d3) >> std::get<2>(d3);
		}
	}

	void Serialize(CArchive& ar, DxPerspective& perspective)
	{
		if (ar.IsStoring())
		{
			ar << perspective.NearPlane << perspective.FarPlane;
		}
		else
		{
			ar >> perspective.NearPlane >> perspective.FarPlane;
		}
	}

	void Serialize(CArchive& ar, RotationGroup& rotationGroup)
	{
		int version = RotationGroup::RotationGroupVersion;

		if (ar.IsStoring())
		{
			ar << version;
			ar << RotationSelectionTypeToInt(rotationGroup.RotationType);
			Serialize(ar, rotationGroup.RotationParamsMain);
			Serialize(ar, rotationGroup.RotationParamsBackground);
		}
		else
		{
			ar >> version;

			if (version != RotationGroup::RotationGroupVersion)
				throw DxCore::CSerializationException("Rotation group version not supported");

			int type = 0;
			ar >> type;
			rotationGroup.RotationType = RotationSelectionTypeFromInt(type);
			Serialize(ar, rotationGroup.RotationParamsMain);
			Serialize(ar, rotationGroup.RotationParamsBackground);
		}
	}
}