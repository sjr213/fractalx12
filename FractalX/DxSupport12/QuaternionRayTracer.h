#pragma once

#include "IRayTracer.h"

namespace DXF
{
	struct ModelData;

	std::unique_ptr<IRayTracer> CreateQuaternionRayTracer(const TraceParams& traceParams, const ModelData& modelData);
}