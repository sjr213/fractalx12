#include "pch.h"
#include "InglesRayTracer.h"

#include <algorithm>
#include <limits>
#include "ModelData.h"
#include "RayTracerCommon.h"
#include "RayTracerDoubleCommon.h"
#include <SimpleMath.h>
#include "SphereApproximator.h"
#include "TraceParams.h"
#include "VertexData.h"
#include "Vector3Double.h"


using namespace DirectX;
using namespace DirectX::SimpleMath;

namespace DXF
{
	class InglesRayTracer : public IRayTracer
	{
	private:
		TraceParams m_traceParams;
		ModelData m_modelData;

	protected:

		void CalculateNextCycle(const Vector3Double& z, const Vector3Double& dz, Vector3Double& z1, Vector3Double& dz1, const DXF::Vertex<double>& C)
		{
			z1.x = z.x * z.x - z.y * z.y - z.z * z.z + C.X;
			z1.y = 2.0 * z.x * z.y + C.Y;
			z1.z = 2.0 * z.x * z.z + C.Z;

			dz1.x = 2.0 * z.x * dz.x;	// not sure if this = 2*z*z1
			dz1.y = 2.0 * z.y * dz.y;
			dz1.z = 2.0 * z.z * dz.z;
		}

		double EstimateDistance(Vector3Double& pos)
		{
			Vector3Double z = pos;
			Vector3Double dz(1.0, 1.0, 1.0);
			double dr = 1.0;
			double r = 0.0;
			Vector3Double z1;
			Vector3Double dz1;
			DXF::Vertex<double> C = (m_traceParams.Fractal.Mandlelbrot) ? DXF::Vertex<double>(0.0, 0.0, 0.0) : m_traceParams.Fractal.ConstantC;

			if (m_traceParams.Fractal.NormalizationType == BulbNormalizeType::AltRoots)
			{
				r = z.Length(m_traceParams.Fractal.NormalizationRoot);
			}
			else
			{
				r = z.Length();
			}

			for (int i = 0; i < m_traceParams.Bulb.Iterations; ++i)
			{
				if (r > m_traceParams.Fractal.Bailout)
					break;

				CalculateNextCycle(z, dz, z1, dz1, C);

				if (IsNan(z1))
					break;

				if (m_traceParams.Fractal.NormalizationType == BulbNormalizeType::AltRoots)
				{
					r = z1.Length(m_traceParams.Fractal.NormalizationRoot);
					dr = dz1.Length(m_traceParams.Fractal.NormalizationRoot);
				}
				else
				{
					r = z1.Length();
					dr = dz1.Length();
				}

				z = z1;
				dz = dz1;

				z += pos;
			}

			if (r == 0.0 || dr == 0.0)
				return 0.0;

			return 0.5 * log(r) * r / dr;
		}

	public:

		InglesRayTracer(const TraceParams& traceParams, const ModelData& modelData)
			: m_traceParams(traceParams)
			, m_modelData(modelData)
		{}

		virtual ~InglesRayTracer()
		{}

		std::shared_ptr<DxVertexData> RayTrace(const TriangleData& data, const std::function<void(double)>& setProgress) override
		{
			auto tracer = CreateInternalDoubleRayTracer(m_traceParams, m_modelData,
				[this](Vector3Double& pos)
				{
					return EstimateDistance(pos);
				});

			return tracer->RayTrace(data, setProgress);
		}

		std::shared_ptr<DxVertexData> RayTraceStretch(const TriangleData& data, const std::function<void(double)>& setProgress) override
		{
			auto tracer = CreateInternalDoubleRayTracer(m_traceParams, m_modelData, 
				[this](Vector3Double& pos)
				{
					return EstimateDistance(pos);
				});

			return tracer->RayTraceStretch(data, setProgress);
		}

	};

	std::unique_ptr<IRayTracer> CreateInglesRayTracer(const TraceParams& traceParams, const ModelData& modelData)
	{
		return std::make_unique<InglesRayTracer>(traceParams, modelData);
	}
}