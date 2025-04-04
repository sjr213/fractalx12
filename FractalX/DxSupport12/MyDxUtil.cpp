#include "pch.h"
#include "MyDxUtil.h"



namespace DxSupport::MyDxUtil
{
	D2D1_RECT_F CalculateD2Rect(D2D1_SIZE_F rtSize, const D2D1_RECT_F& relativeRect)
	{
		return D2D1::RectF(
			rtSize.width * relativeRect.left,
			rtSize.height * relativeRect.top,
			rtSize.width * relativeRect.right,
			rtSize.height * relativeRect.bottom
		);
	}

	D2D1_RECT_F MakeRelativeRect(const CRect& client, CPoint topLeft, CPoint bottomRight)
	{
		auto left = std::min(topLeft.x, bottomRight.x);
		auto right = std::max(topLeft.x, bottomRight.x);
		auto top = std::min(topLeft.y, bottomRight.y);
		auto bottom = std::max(topLeft.y, bottomRight.y);

		left = std::max(left, client.left);
		left = std::min(left, client.right);
		right = std::min(right, client.right);
		right = std::max(right, client.left);
		top = std::max(top, client.top);
		top = std::min(top, client.bottom);
		bottom = std::min(bottom, client.bottom);
		bottom = std::max(bottom, client.top);

		float relativeLeft = 0.0f;
		float relativeRight = 0.0f;
		float relativeTop = 0.0f;
		float relativeBottom = 0.0f;

		if (client.Width() != 0)
		{
			relativeLeft = static_cast<float>(left - client.left) / client.Width();
			relativeRight = static_cast<float>(right - client.left) / client.Width();
		}

		if (client.Height() != 0)
		{
			relativeTop = static_cast<float>(top - client.top) / client.Height();
			relativeBottom = static_cast<float>(bottom - client.top) / client.Height();
		}

		return D2D1::RectF(relativeLeft, relativeTop, relativeRight, relativeBottom);
	}

	std::vector<UINT8> GenerateTextureData(const std::vector<uint32_t>& colors)
	{
		std::vector<UINT8> components;
		components.reserve(4 * colors.size());

		for (const auto& color : colors)
		{
			components.push_back((uint8_t)(color >> 24));
			components.push_back((uint8_t)(color >> 16));
			components.push_back((uint8_t)(color >> 8));
			components.push_back((uint8_t)color);
		}

		return components;
	}
}