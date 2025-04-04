#include "pch.h"
#include "InglesRayTracer3.h"

#include <algorithm>
#include <limits>
#include "MathFunctions.h"
#include "ModelData.h"
#include "NotSupportedException.h"
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
	class InglesRayTracer3 : public IRayTracer
	{
	private:
		TraceParams m_traceParams;
		ModelData m_modelData;
		std::function<void(const Vector3Double&, const Vector3Double&, Vector3Double&, Vector3Double&, const DXF::Vertex<double>&)> m_calculateNextCycle;

	protected:

		void CalculateNextCycleSquared(const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
		{
			Q1.x = Q.x * Q.x - Q.y * Q.y - Q.z * Q.z + C.X;
			Q1.y = 2.0 * Q.x * Q.y + C.Y;
			Q1.z = 2.0 * Q.x * Q.z + C.Z;

			dQ1.x = 2.0 * Q.x * dQ.x - 2.0 * Q.y * dQ.y - 2.0 * Q.z * dQ.z;
			dQ1.y = 2.0 * Q.y * dQ.x + 2.0 * Q.x * dQ.y;
			dQ1.z = 2.0 * Q.z * dQ.x + 2.0 * Q.x * dQ.z;
		}

		void CalculateNextCycleCubed(const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
		{
			Q1.x = 3.0 * Q.x * Q.x * Q.x - 3.0 * Q.x * Q.y * Q.y - 3.0 * Q.x * Q.z * Q.z + C.X;
			Q1.y = 3.0 * Q.x * Q.x * Q.y - Q.y * Q.y * Q.y - Q.y * Q.z * Q.z + C.Y;
			Q1.z = 3.0 * Q.x * Q.x * Q.z - Q.y * Q.y * Q.z - Q.z * Q.z * Q.z + C.Z;

			dQ1.x = 3.0 * Q.x * Q.x * dQ.x -3.0 * Q.y * Q.y * dQ.x -3.0 * Q.z * Q.z * dQ.x - 6.0 * Q.x * Q.y * dQ.y - 6.0 * Q.x * Q.z * dQ.z;
			dQ1.y = 6.0 * Q.x * Q.y * dQ.x + 3.0 * Q.x * Q.x * dQ.y - 3.0 * Q.y * Q.y * dQ.y - Q.z * Q.z * dQ.y - 2.0 * Q.y * Q.z * dQ.z;
			dQ1.z = 6.0 * Q.x * Q.z * dQ.x - 2.0 * Q.y * Q.z * dQ.y + 6.0 * Q.x * Q.x * dQ.z - Q.y * Q.y * dQ.z - 3.0 * Q.z * Q.z * dQ.z;
		}

		void CalculateNextCycleSineX(const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
		{
			Q1.x = sin(Q.x * Q.x) - Q.y * Q.y - Q.z * Q.z + C.X;
			Q1.y = 2.0 * Q.x * Q.y + C.Y;
			Q1.z = 2.0 * Q.x * Q.z + C.Z;

			dQ1.x = cos(Q.x * Q.x) * 2.0 * Q.x * dQ.x - 2.0 * Q.y * dQ.y - 2.0 * Q.z * dQ.z;
			dQ1.y = 2.0 * Q.y * dQ.x + 2.0 * Q.x * dQ.y;
			dQ1.z = 2.0 * Q.z * dQ.x + 2.0 * Q.x * dQ.z;
		}

		void CalculateNextCycleAlt1(const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
		{
			Q1.x = Q.x * Q.x - Q.y * Q.y - Q.z * Q.z + 2.0 * Q.y * Q.z + C.X;
			Q1.y = 2.0 * Q.x * Q.y + C.Y;
			Q1.z = 2.0 * Q.x * Q.z + C.Z;

			dQ1.x = 2.0 * Q.x * dQ.x - 2.0 * Q.y * dQ.y + 2.0 * Q.z * dQ.y - 2.0 * Q.z * dQ.z + 2.0 * Q.y * dQ.z;
			dQ1.y = 2.0 * Q.y * dQ.x + 2.0 * Q.x * dQ.y;
			dQ1.z = 2.0 * Q.z * dQ.x + 2.0 * Q.x * dQ.z;
		}

		void CalculateNextCycleAlt2(const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
		{
			Q1.x = Q.x * Q.x - Q.y * Q.y + C.X;
			Q1.y = 2.0 * Q.x * Q.y - Q.z * Q.z + C.Y;
			Q1.z = 2.0 * Q.x * Q.z + C.Z;

			dQ1.x = 2.0 * Q.x * dQ.x - 2.0 * Q.y * dQ.y;
			dQ1.y = 2.0 * Q.y * dQ.x + 2.0 * Q.x * dQ.y - 2.0 * Q.z * dQ.z;
			dQ1.z = 2.0 * Q.z * dQ.x + 2.0 * Q.x * dQ.z;
		}

		void CalculateNextCycleAlt3(const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
		{
			Q1.x = Q.x * Q.x - Q.y * Q.y - 2.0 * Q.y * Q.z + C.X;
			Q1.y = 2.0 * Q.x * Q.y - Q.z * Q.z + C.Y;
			Q1.z = 2.0 * Q.x * Q.z + C.Z;

			dQ1.x = 2.0 * Q.x * dQ.x - 2.0 * Q.y * dQ.y - 2.0 * Q.z * dQ.y - 2.0 * Q.y * dQ.z;
			dQ1.y = 2.0 * Q.y * dQ.x + 2.0 * Q.x * dQ.y - 2.0 * Q.z * dQ.z;
			dQ1.z = 2.0 * Q.z * dQ.x + 2.0 * Q.x * dQ.z;
		}

		void CalculateNextCycleAlt4(const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
		{
			Q1.x = Q.x * Q.x - Q.y * Q.y + 2.0 * Q.y * Q.z + C.X;
			Q1.y = 2.0 * Q.x * Q.y - Q.z * C.Y;
			Q1.z = 2.0 * Q.x * Q.z + C.Z;

			dQ1.x = 2.0 * Q.x * dQ.x - 2.0 * Q.y * dQ.y + 2.0 * Q.z * dQ.y + 2.0 * Q.y * dQ.z;
			dQ1.y = 2.0 * Q.y * dQ.x + 2.0 * Q.x * dQ.y - 2.0 * Q.z * dQ.z;
			dQ1.z = 2.0 * Q.z * dQ.x + 2.0 * Q.x * dQ.z;
		}

		void CalculateNextCycleAlt5(const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
		{
			Q1.x = Q.x * Q.x - Q.y * Q.y + C.X;
			Q1.y = 2.0 * Q.x * Q.y - Q.z * Q.z + C.Y;
			Q1.z = 2.0 * Q.x * Q.z + 2.0 * Q.y * Q.z + C.Z;

			dQ1.x = 2.0 * Q.x * dQ.x - 2.0 * Q.y * dQ.y;
			dQ1.y = 2.0 * Q.y * dQ.x + 2.0 * Q.x * dQ.y - 2.0 * Q.z * dQ.z;
			dQ1.z = 2.0 * Q.z * dQ.x + 2.0 * Q.z * dQ.y + 2.0 * Q.x * dQ.z + 2.0 * Q.y * dQ.z;
		}

		void CalculateNextCycleCubedCorrect(const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
		{
			Q1.x = 3.0 * Q.x * Q.x * Q.x - 3.0 * Q.x * Q.y * Q.y - 3.0 * Q.x * Q.z * Q.z + C.X;
			Q1.y = 3.0 * Q.x * Q.x * Q.y - Q.y * Q.y * Q.y - Q.y * Q.z * Q.z + C.Y;
			Q1.z = 3.0 * Q.x * Q.x * Q.z - Q.y * Q.y * Q.z - Q.z * Q.z * Q.z + C.Z;

			dQ1.x = 9.0 * Q.x * Q.x * dQ.x - 3.0 * Q.y * Q.y * dQ.x - 3.0 * Q.z * Q.z * dQ.x - 6.0 * Q.x * Q.y * dQ.y - 6.0 * Q.x * Q.z * dQ.z;
			dQ1.y = 6.0 * Q.x * Q.y * dQ.x + 3.0 * Q.x * Q.x * dQ.y - 3.0 * Q.y * Q.y * dQ.y - Q.z * Q.z * dQ.y - 2.0 * Q.y * Q.z * dQ.z;
			dQ1.z = 6.0 * Q.x * Q.z * dQ.x - 2.0 * Q.y * Q.z * dQ.y + 3.0 * Q.x * Q.x * dQ.z - Q.y * Q.y * dQ.z - 3.0 * Q.z * Q.z * dQ.z;
		}

		void CalculateNextCycleTest1(const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
		{
			Q1.x = Q.x * Q.x * Q.y - Q.y * Q.y * Q.z + C.X;
			Q1.y = Q.x * cos(Q.y * Q.y) + C.Y;
			Q1.z = Q.y * tan(Q.z * Q.z) + C.Z;

			dQ1.x = 2.0 * Q.x * Q.y * dQ.x + Q.x * Q.x * dQ.y - 2.0 * Q.y * Q.z * dQ.y - Q.y * Q.y * dQ.z;
			dQ1.y = cos(Q.y * Q.y) * dQ.x - 2.0 * Q.x * Q.y * sin(Q.y * Q.y) * dQ.y;
			dQ1.z = tan(Q.z * Q.z) * dQ.y + 2.0 * Q.y * Q.z * MathFunctions::sec(Q.z * Q.z) * dQ.z;
		}

		void CalculateNextCycleTest2(const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
		{
			double term = (Q.x * Q.x - Q.y * Q.y) / Q.z;

			Q1.x = term + C.X;
			Q1.y = Q.y * Q.y - Q.z + C.Y;
			Q1.z = Q.z * Q.z * Q.z - Q.x * Q.x - Q.y + C.Z;

			dQ1.x = term * 2.0 * Q.x * dQ.x - term * 2.0 * Q.y * dQ.y - (term / Q.z) * dQ.z;
			dQ1.y = 2.0 * Q.y * dQ.y - dQ.z;
			dQ1.z = 3.0 * Q.z * Q.z * dQ.z - 2.0 * Q.x * dQ.x - dQ.y;
		}

		void CalculateNextCycleSquaredSplitDerviative(const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
		{
			double term = (Q.x * Q.x - Q.y * Q.y) / Q.z;

			Q1.x = Q.x * Q.x - Q.y * Q.y - Q.z * Q.z + C.X;
			Q1.y = 2.0 * Q.x * Q.y + C.Y;
			Q1.z = 2.0 * Q.x * Q.z + C.Z;

			dQ1.x = 2.0 * (Q.x + Q.y + Q.z) * dQ.x;
			dQ1.y = 2.0 * (Q.x - Q.y) * dQ.y;
			dQ1.z = 2.0 * (Q.x - Q.z) * dQ.z;
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

			int i = 0;
			for (; i < m_traceParams.Bulb.Iterations; ++i)
			{
				if (r > m_traceParams.Fractal.Bailout)
					break;

				m_calculateNextCycle(z, dz, z1, dz1, C);

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

			return log(r) * r / dr;
		}

		void SetNextCycle()
		{
			switch (m_traceParams.Fractal.InglesEquation)
			{
			case(Ingles3EquationType::I_Squared):
			{
				m_calculateNextCycle = [&](const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
				{
					this->CalculateNextCycleSquared(Q, dQ, Q1, dQ1, C);
				};
				break;
			}
			case(Ingles3EquationType::I_Cubed):
			{
				m_calculateNextCycle = [&](const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
				{
					this->CalculateNextCycleCubed(Q, dQ, Q1, dQ1, C);
				};
				break;
			}
			case(Ingles3EquationType::I_SinX):
			{
				m_calculateNextCycle = [&](const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
				{
					this->CalculateNextCycleSineX(Q, dQ, Q1, dQ1, C);
				};
				break;
			}
			case(Ingles3EquationType::I_Alt1):
			{
				m_calculateNextCycle = [&](const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
				{
					this->CalculateNextCycleAlt1(Q, dQ, Q1, dQ1, C);
				};
				break;
			}
			case(Ingles3EquationType::I_Alt2):
			{
				m_calculateNextCycle = [&](const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
				{
					this->CalculateNextCycleAlt2(Q, dQ, Q1, dQ1, C);
				};
				break;
			}
			case(Ingles3EquationType::I_Alt3):
			{
				m_calculateNextCycle = [&](const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
				{
					this->CalculateNextCycleAlt3(Q, dQ, Q1, dQ1, C);
				};
				break;
			}
			case(Ingles3EquationType::I_Alt4):
			{
				m_calculateNextCycle = [&](const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
				{
					this->CalculateNextCycleAlt4(Q, dQ, Q1, dQ1, C);
				};
				break;
			}
			case(Ingles3EquationType::I_Alt5):
			{
				m_calculateNextCycle = [&](const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
				{
					this->CalculateNextCycleAlt5(Q, dQ, Q1, dQ1, C);
				};
				break;
			}
			case(Ingles3EquationType::I_CubedCorrect):
			{
				m_calculateNextCycle = [&](const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
				{
					this->CalculateNextCycleCubedCorrect(Q, dQ, Q1, dQ1, C);
				};
				break;
			}
			case(Ingles3EquationType::I_Test1):
			{
				m_calculateNextCycle = [&](const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
				{
					this->CalculateNextCycleTest1(Q, dQ, Q1, dQ1, C);
				};
				break;
			}
			case(Ingles3EquationType::I_Test2):
			{
				m_calculateNextCycle = [&](const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
				{
					this->CalculateNextCycleTest2(Q, dQ, Q1, dQ1, C);
				};
				break;
			}
			case(Ingles3EquationType::I_Squared_Split_Derivative):
			{
				m_calculateNextCycle = [&](const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
				{
					this->CalculateNextCycleSquaredSplitDerviative(Q, dQ, Q1, dQ1, C);
				};
				break;
			}
			default:
			{
				throw DxCore::CNotSupportedException("Unknown Ingles3EquationType");
			}
			}

		}

	public:

		InglesRayTracer3(const TraceParams& traceParams, const ModelData& modelData)
			: m_traceParams(traceParams)
			, m_modelData(modelData)
		{
			SetNextCycle();
		}

		virtual ~InglesRayTracer3()
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

	std::unique_ptr<IRayTracer> CreateInglesRayTracer3(const TraceParams& traceParams, const ModelData& modelData)
	{
		return std::make_unique<InglesRayTracer3>(traceParams, modelData);
	}
}