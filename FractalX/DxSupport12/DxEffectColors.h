
#pragma once
#include "SimpleMath.h"

namespace DXF
{
	struct DxEffectColors
	{
		DxEffectColors()
			: AmbientColor(DirectX::SimpleMath::Vector4(0.24f, 0.24f, 0.24f, 1.0f))
			, DiffuseColor(DirectX::SimpleMath::Vector4(0.75f, 0.75f, 0.75f, 1.0f))
			, SpecularColor(DirectX::SimpleMath::Vector4(0.75f, 0.75f, 0.75f, 1.0f))
			, Roughness(0.125f)
		{}

		DirectX::SimpleMath::Color AmbientColor;
		DirectX::SimpleMath::Color DiffuseColor;
		DirectX::SimpleMath::Color SpecularColor;
		float Roughness;
	};
}