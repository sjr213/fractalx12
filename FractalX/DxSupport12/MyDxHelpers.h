#pragma once

#include <DirectXMath.h>
#include "FrameResourceFx.h"
#include <optional>
#include "Vertex.h"

namespace DXF::DxHelpers
{
	bool GetIntersection(const std::vector<DxSupport::Vertex>& vertices, const std::vector<std::int32_t>& indices,
		const DirectX::XMVECTOR& clickTransform, DirectX::XMVECTOR& projectedVector, double& eClosestDistance);

	std::optional<DXF::Vertex<float>> Create3DTuple(double closestDistance,
		const DirectX::XMVECTOR& pointATransform, const DirectX::XMVECTOR& projectedVector);
}