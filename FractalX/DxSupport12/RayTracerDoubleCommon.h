#pragma once

#include <functional>
#include "IRayTracer.h"


namespace DXF
{
	struct ModelData;
	struct TraceParams;
	struct Vector3Double;

	std::unique_ptr<IRayTracer> CreateInternalDoubleRayTracer(const TraceParams& traceParams, const ModelData& modelData,
		const std::function<double(Vector3Double&)>& estimateDistance);
}