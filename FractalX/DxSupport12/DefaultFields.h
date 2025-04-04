#pragma once

#include "ColorArgb.h"
#include "Vertex.h"

namespace DXF
{
	Vertex<float> GetDefaultWorldScale();

	Vertex<float> GetDefaultTarget();

	Vertex<float> GetDefaultCamera();

	double GetMaxStretch();

	std::vector<Vertex<float>> GetDefaultBackgroundModel();

	DxColor::ColorArgb GetDefaultShadowColor();
}