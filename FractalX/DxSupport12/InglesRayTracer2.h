#pragma once

#include "IRayTracer.h"

namespace DXF
{
	struct ModelData;

	std::unique_ptr<IRayTracer> CreateInglesRayTracer2(const TraceParams& traceParams, const ModelData& modelData);
}