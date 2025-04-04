#pragma once

namespace DXF
{
	// Each value corresponds to an index in the vertex vector
	struct Triangle
	{
		Triangle()
			: one(0)
			, two(0)
			, three(0)
		{}

		Triangle(unsigned int a, unsigned int b, unsigned int c)
			: one(a)
			, two(b)
			, three(c)
		{}

		unsigned int one;
		unsigned int two;
		unsigned int three;
	};

	struct TriangleData
	{
		std::vector<DirectX::XMFLOAT3> Vertices;
		std::vector<Triangle> Triangles;
	};
}
