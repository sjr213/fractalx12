#include "stdafx.h"
#include "CppUnitTest.h"
#include <CppUnitTestAssert.h>

#include "RectangleApproximator.h"

using namespace DXF;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace DxTests
{
	TEST_CLASS(RectangleApproximatorTest)
	{
	public:
		static constexpr float TOLERANCE = 0.0001f;

		// See comment in source file for methods being tested
		TEST_METHOD(TestMakeVertices)
		{
			auto vertices = MakeVertices(5, 5, 1.0, 1.0);

			Assert::AreEqual(25, static_cast<int>(vertices.size()), L"Wrong number of vertices", LINE_INFO());

			auto pt0 = vertices.at(0);
			Assert::AreEqual(-1.0f, pt0.x, TOLERANCE, L"First point x is wrong", LINE_INFO());
			Assert::AreEqual(1.0f, pt0.y, TOLERANCE, L"First point y is wrong", LINE_INFO());
			Assert::AreEqual(1.0f, pt0.z, TOLERANCE, L"First point z is wrong", LINE_INFO());

			auto pt24 = vertices.at(24);
			Assert::AreEqual(1.0f, pt24.x, TOLERANCE, L"Last point x is wrong", LINE_INFO());
			Assert::AreEqual(-1.0f, pt24.y, TOLERANCE, L"Last point y is wrong", LINE_INFO());
			Assert::AreEqual(1.0f, pt24.z, TOLERANCE, L"Last point z is wrong", LINE_INFO());
		}

		TEST_METHOD(TestCreateTriangles)
		{
			auto triangles = CreateTriangles(4,3);
			Assert::AreEqual(12, static_cast<int>(triangles.size()), L"Wrong number of triangles", LINE_INFO());

			Assert::AreEqual(0u, triangles.at(0).one, L"First index of first triangle is wrong", LINE_INFO());
			Assert::AreEqual(1u, triangles.at(0).two, L"Second index of first triangle is wrong", LINE_INFO());
			Assert::AreEqual(4u, triangles.at(0).three, L"Third index of first triangle is wrong", LINE_INFO());
			Assert::AreEqual(2u, triangles.at(3).one, L"First index of 4th triangle is wrong", LINE_INFO());
			Assert::AreEqual(6u, triangles.at(3).two, L"Second index of 4th triangle is wrong", LINE_INFO());
			Assert::AreEqual(5u, triangles.at(3).three, L"Third index of 4th triangle is wrong", LINE_INFO());
			Assert::AreEqual(3u, triangles.at(5).one, L"First index of 6th triangle is wrong", LINE_INFO());
			Assert::AreEqual(7u, triangles.at(5).two, L"Second index of 6th triangle is wrong", LINE_INFO());
			Assert::AreEqual(6u, triangles.at(5).three, L"Third index of 6th triangle is wrong", LINE_INFO());
			Assert::AreEqual(4u, triangles.at(6).one, L"First index of 7th triangle is wrong", LINE_INFO());
			Assert::AreEqual(5u, triangles.at(6).two, L"Second index of 7th triangle is wrong", LINE_INFO());
			Assert::AreEqual(8u, triangles.at(6).three, L"Third index of 7th triangle is wrong", LINE_INFO());
			Assert::AreEqual(7u, triangles.at(11).one, L"First index of 12th triangle is wrong", LINE_INFO());
			Assert::AreEqual(11u, triangles.at(11).two, L"Second index of 12th triangle is wrong", LINE_INFO());
			Assert::AreEqual(10u, triangles.at(11).three, L"Third index of 12th triangle is wrong", LINE_INFO());
		}
	};
}