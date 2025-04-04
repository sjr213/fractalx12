#include "stdafx.h"

#include "CppUnitTest.h"
#include <CppUnitTestAssert.h>
#include <d2d1.h>
#include <atltypes.h>
#include "MyDxUtil.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace DxTests
{
	TEST_CLASS(DxCalculationTest)
	{
	public:
		static constexpr float TOLERANCE = 0.0001f;

		TEST_METHOD(TestACalculateD2Rect)
		{
			D2D1_SIZE_F size = { 100.0f, 200.0f };
			D2D1_RECT_F relativeRect = { 0.1f, 0.3f, 0.2f, 0.5f };

			auto resultRect = DxSupport::MyDxUtil::CalculateD2Rect(size, relativeRect);

			Assert::AreEqual(10.0f, resultRect.left, TOLERANCE, L"Left is wrong", LINE_INFO());
			Assert::AreEqual(60.f, resultRect.top, TOLERANCE, L"Top is wrong", LINE_INFO());
			Assert::AreEqual(20.0f, resultRect.right, TOLERANCE, L"Right is wrong", LINE_INFO());
			Assert::AreEqual(100.f, resultRect.bottom, TOLERANCE, L"Bottom is wrong", LINE_INFO());
		}

		TEST_METHOD(TestACalculateD2RectWithZeroSize)
		{
			D2D1_SIZE_F size = { 0.0f, 0.0f };
			D2D1_RECT_F relativeRect = { 0.1f, 0.3f, 0.2f, 0.5f };

			auto resultRect = DxSupport::MyDxUtil::CalculateD2Rect(size, relativeRect);

			Assert::AreEqual(0.0f, resultRect.left, TOLERANCE, L"Left is wrong", LINE_INFO());
			Assert::AreEqual(0.f, resultRect.top, TOLERANCE, L"Top is wrong", LINE_INFO());
			Assert::AreEqual(0.0f, resultRect.right, TOLERANCE, L"Right is wrong", LINE_INFO());
			Assert::AreEqual(0.f, resultRect.bottom, TOLERANCE, L"Bottom is wrong", LINE_INFO());
		}

		TEST_METHOD(TestACalculateD2RectWithRelativeGreaterThanOne)
		{
			D2D1_SIZE_F size = { 100.0f, 200.0f };
			D2D1_RECT_F relativeRect = { 0.1f, 0.3f, 2.0f, 1.5f };

			auto resultRect = DxSupport::MyDxUtil::CalculateD2Rect(size, relativeRect);

			Assert::AreEqual(10.0f, resultRect.left, TOLERANCE, L"Left is wrong", LINE_INFO());
			Assert::AreEqual(60.f, resultRect.top, TOLERANCE, L"Top is wrong", LINE_INFO());
			Assert::AreEqual(200.0f, resultRect.right, TOLERANCE, L"Right is wrong", LINE_INFO());
			Assert::AreEqual(300.0f, resultRect.bottom, TOLERANCE, L"Bottom is wrong", LINE_INFO());
		}

		TEST_METHOD(TestMakeRelativeRect)
		{
			CRect client(0, 0, 200, 100);
			CPoint topLeft(10, 10);
			CPoint bottomRight(50, 40);

			auto relRect = DxSupport::MyDxUtil::MakeRelativeRect(client, topLeft, bottomRight);
			Assert::AreEqual(0.05f, relRect.left, TOLERANCE, L"Left is wrong", LINE_INFO());
			Assert::AreEqual(0.1f, relRect.top, TOLERANCE, L"Top is wrong", LINE_INFO());
			Assert::AreEqual(0.25f, relRect.right, TOLERANCE, L"Right is wrong", LINE_INFO());
			Assert::AreEqual(0.4f, relRect.bottom, TOLERANCE, L"Bottom is wrong", LINE_INFO());
		}

		TEST_METHOD(TestMakeRelativeRectSwitchedPts)
		{
			CRect client(0, 0, 200, 100);
			CPoint topLeft(50, 40);
			CPoint bottomRight(10, 10);
			
			auto relRect = DxSupport::MyDxUtil::MakeRelativeRect(client, topLeft, bottomRight);
			Assert::AreEqual(0.05f, relRect.left, TOLERANCE, L"Left is wrong", LINE_INFO());
			Assert::AreEqual(0.1f, relRect.top, TOLERANCE, L"Top is wrong", LINE_INFO());
			Assert::AreEqual(0.25f, relRect.right, TOLERANCE, L"Right is wrong", LINE_INFO());
			Assert::AreEqual(0.4f, relRect.bottom, TOLERANCE, L"Bottom is wrong", LINE_INFO());
		}

		TEST_METHOD(TestMakeRelativeRectWithZeroRect)
		{
			CRect client(0, 0, 0, 0);
			CPoint topLeft(10, 10);
			CPoint bottomRight(50, 40);

			auto relRect = DxSupport::MyDxUtil::MakeRelativeRect(client, topLeft, bottomRight);
			Assert::AreEqual(0.0f, relRect.left, TOLERANCE, L"Left is wrong", LINE_INFO());
			Assert::AreEqual(0.0f, relRect.top, TOLERANCE, L"Top is wrong", LINE_INFO());
			Assert::AreEqual(0.0f, relRect.right, TOLERANCE, L"Right is wrong", LINE_INFO());
			Assert::AreEqual(0.0f, relRect.bottom, TOLERANCE, L"Bottom is wrong", LINE_INFO());
		}

		// The points are outside the client area so relative rect is zero
		TEST_METHOD(TestMakeRelativeRectOutOfRange)
		{
			CRect client(100, 50, 200, 100);
			CPoint topLeft(10, 10);
			CPoint bottomRight(50, 40);

			auto relRect = DxSupport::MyDxUtil::MakeRelativeRect(client, topLeft, bottomRight);
			Assert::AreEqual(0.0f, relRect.left, TOLERANCE, L"Left is wrong", LINE_INFO());
			Assert::AreEqual(0.0f, relRect.top, TOLERANCE, L"Top is wrong", LINE_INFO());
			Assert::AreEqual(0.0f, relRect.right, TOLERANCE, L"Right is wrong", LINE_INFO());
			Assert::AreEqual(0.0f, relRect.bottom, TOLERANCE, L"Bottom is wrong", LINE_INFO());
		}

		TEST_METHOD(TestMakeRelativeRectDisplaced)
		{
			CRect client(100, 50, 200, 100);
			CPoint topLeft(110, 60);
			CPoint bottomRight(150, 90);

			auto relRect = DxSupport::MyDxUtil::MakeRelativeRect(client, topLeft, bottomRight);
			Assert::AreEqual(0.1f, relRect.left, TOLERANCE, L"Left is wrong", LINE_INFO());
			Assert::AreEqual(0.2f, relRect.top, TOLERANCE, L"Top is wrong", LINE_INFO());
			Assert::AreEqual(0.5f, relRect.right, TOLERANCE, L"Right is wrong", LINE_INFO());
			Assert::AreEqual(0.8f, relRect.bottom, TOLERANCE, L"Bottom is wrong", LINE_INFO());
		}

		TEST_METHOD(TestGenerateTextureData)
		{
			std::vector<uint32_t> colors = { 0x000001FF, 0x0F0E0100};
			
			auto bytes = DxSupport::MyDxUtil::GenerateTextureData(colors);

			Assert::IsTrue(0x00 == bytes[0], L"Color Index 0 wrong", LINE_INFO());
			Assert::IsTrue(0x00 == bytes[1], L"Color Index 1 wrong", LINE_INFO());
			Assert::IsTrue(0x01 == bytes[2], L"Color Index 2 wrong", LINE_INFO());
			Assert::IsTrue(0xFF == bytes[3], L"Color Index 3 wrong", LINE_INFO());
			Assert::IsTrue(0x0F == bytes[4], L"Color Index 4 wrong", LINE_INFO());
			Assert::IsTrue(0x0E == bytes[5], L"Color Index 5 wrong", LINE_INFO());
			Assert::IsTrue(0x01 == bytes[6], L"Color Index 6 wrong", LINE_INFO());
			Assert::IsTrue(0x00 == bytes[7], L"Color Index 7 wrong", LINE_INFO());
		}
	};
}