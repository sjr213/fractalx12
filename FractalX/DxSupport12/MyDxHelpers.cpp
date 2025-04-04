#include "pch.h"
#include "MyDxHelpers.h"

#include <DirectXMath.h>
#include "SimpleMath.h"
#include <optional>

using namespace DirectX;
using namespace DirectX::SimpleMath;

namespace DXF::DxHelpers
{
	bool GetIntersection(const std::vector<DxSupport::Vertex>& vertices, const std::vector<std::int32_t>& indices,
		const XMVECTOR& clickTransform, XMVECTOR& projectedVector, double& eClosestDistance)
	{
		DWORD_PTR uTriangles = indices.size() / 3;
		auto indexIter = indices.begin();

		for (DWORD_PTR i = 0; i < uTriangles; ++i, indexIter += 3)
		{
			float eDistance = 0;

			auto v1 = XMLoadFloat3(&vertices[*indexIter].Pos);
			auto v2 = XMLoadFloat3(&vertices[*(indexIter + 1)].Pos);
			auto v3 = XMLoadFloat3(&vertices[*(indexIter + 2)].Pos);

			projectedVector = XMVector3Normalize(projectedVector);
			bool foundIntersection = TriangleTests::Intersects(clickTransform, projectedVector, v1, v2, v3, eDistance);

			// And it intersects in front of the screen close than we have seen
			if (foundIntersection && (eDistance >= 0) && (eClosestDistance > eDistance))
				eClosestDistance = eDistance;
		}

		return true;
	}

	std::optional<DXF::Vertex<float>> Create3DTuple(double closestDistance, const XMVECTOR& pointATransform, const XMVECTOR& projectedVector)
	{
		if (closestDistance == DBL_MAX)
			return std::nullopt;

		float x = static_cast<float>(XMVectorGetX(pointATransform) + closestDistance * XMVectorGetX(projectedVector));
		float y = static_cast<float>(XMVectorGetY(pointATransform) + closestDistance * XMVectorGetY(projectedVector));
		float z = static_cast<float>(XMVectorGetZ(pointATransform) + closestDistance * XMVectorGetZ(projectedVector));

		return Vertex<float>(x, y, z);
	}
}