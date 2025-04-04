#pragma once

#include <d2d1_2.h>
#include <vector>

namespace DxSupport::MyDxUtil
{
	D2D1_RECT_F CalculateD2Rect(D2D1_SIZE_F rtSize, const D2D1_RECT_F& relativeRect);

	D2D1_RECT_F MakeRelativeRect(const CRect& client, CPoint topLeft, CPoint bottomRight);

	std::vector<UINT8> GenerateTextureData(const std::vector<uint32_t>& colors);
}