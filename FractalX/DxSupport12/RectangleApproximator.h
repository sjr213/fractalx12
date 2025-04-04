#pragma once

#include <d3d11_1.h>
#include <memory>
#include <SimpleMath.h>
#include "TriangleData.h"

namespace DXF
{
	std::vector<DirectX::XMFLOAT3> MakeVertices(int vertexesWide, int vertexesHigh, double rectHalfWidth, double rectHalfHeight);

	std::vector<Triangle> CreateTriangles(int vertexesWide, int vertexesHigh);

	std::shared_ptr<TriangleData> GenerateCrudeTrianglesForRectangle(int vertexesWide, int vertexesHigh,
		double rectHalfWidth, double rectHalfHeight, const std::function<void(double)>& setProgress);
}