#include "pch.h"
#include "RectangleApproximator.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

namespace DXF
{
	// Imagine a 3 x 3 cube
	//		0	1	2
	//		3	4	5
	//		6	7	8
	std::vector<XMFLOAT3> MakeVertices(int vertexesWide, int vertexesHigh, double rectHalfWidth, double rectHalfHeight)
	{
		std::vector<XMFLOAT3> vertices;
		vertices.reserve(vertexesWide * vertexesHigh);

		float width = static_cast<float>(2 * rectHalfWidth);
		float height = static_cast<float>(2 * rectHalfHeight);

		float deltaX = width / (vertexesWide - 1);
		float deltaY = height / (vertexesHigh - 1);

		for (int yIndex = 0; yIndex < vertexesHigh; ++yIndex)
		{
			float y = static_cast<float>(rectHalfHeight) - yIndex * deltaY;

			for (int xIndex = 0; xIndex < vertexesWide; ++xIndex)
			{
				float x = -1*static_cast<float>(rectHalfWidth) + xIndex * deltaX;
				vertices.push_back(XMFLOAT3(x, y, 1.0f));
			}
		}

		return vertices;
	}

	// The first triangle for above cube would be 0, 1, 3. Followed by 1, 4, 3
	// Then (1,2,4), (2,5,4), (3,4,6), (4,7,6), (4,5,7), (5,8,7)
	std::vector<Triangle> CreateTriangles(int vertexesWide, int vertexesHigh)
	{
		std::vector<Triangle> triangles;
		int trianglesAcross = vertexesWide - 1;
		int trianglesDown = vertexesHigh - 1;
		triangles.reserve(trianglesAcross * trianglesDown);

		for (int y = 0; y < trianglesDown; ++y)
		{
			for (int x = 0; x < trianglesAcross; ++x)
			{
				int topLeft = y * vertexesWide + x;
				triangles.push_back(Triangle(topLeft, topLeft + 1, topLeft + vertexesWide));
				triangles.push_back(Triangle(topLeft + 1, topLeft + vertexesWide + 1, topLeft + vertexesWide));
			}
		}

		return triangles;
	}

	std::shared_ptr<TriangleData> GenerateCrudeTrianglesForRectangle(int vertexesWide, int vertexesHigh, 
		double rectHalfWidth, double rectHalfHeight, const std::function<void(double)>& setProgress)
	{
		std::vector<XMFLOAT3> vertices = MakeVertices(vertexesWide, vertexesHigh, rectHalfWidth, rectHalfHeight);
		setProgress(0.5);
		std::vector<Triangle> triangles = CreateTriangles(vertexesWide, vertexesHigh);

		std::shared_ptr<TriangleData> data = std::make_shared<TriangleData>();
		data->Vertices = vertices;
		data->Triangles = triangles;

		setProgress(1.0);

		return data;
	}
}