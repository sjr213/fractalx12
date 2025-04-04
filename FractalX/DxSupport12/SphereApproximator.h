#pragma once

#include <d3d11_1.h>
#include <memory>
#include <SimpleMath.h>
#include "SeedTriangles.h"

#include "TriangleData.h"
#include "VertexData.h"

namespace DXF
{
	struct ModelData;

	std::shared_ptr<TriangleData> GenerateCrudeTriangles(int depth, SeedTriangles seeds, const std::function<void(double)>& setProgress);

	std::shared_ptr<TriangleData> GenerateTrianglesFromCrudeVertices(const ModelData& modelData, const std::function<void(double)>& setProgress);

	void NormalizeVector(const DirectX::XMFLOAT3& start, DirectX::XMFLOAT3& end);

	void NormalizeVectors(TriangleData& data);

	// input should be normalized first
	std::shared_ptr<DxVertexData> CreateModelFromSphereApproximation(const TriangleData& data);
}