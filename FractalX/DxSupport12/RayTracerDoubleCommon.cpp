#include "pch.h"
#include "RayTracerDoubleCommon.h"

#include <algorithm>
#include "DefaultFields.h"
#include <DirectXMath.h>
#include "ModelData.h"
#include "RayTracerCommon.h"
#include "SphereApproximator.h"
#include "TraceParams.h"
#include "Vector3Double.h"
#include "Vector4Double.h"
#include <memory>

using namespace DirectX;
using namespace DirectX::SimpleMath;

namespace DXF
{
	namespace
	{
		double CalculateStepStretchDistance(const std::function<double(const Vector3Double&, const Vector3Double&, Vector3Double&)>& Marcher,
			const Vector3Double& start, const Vector3Double& direction, Vector3Double& p, const StretchDistanceParams& stretchParams)
		{
			double distance = Marcher(start, direction, p);

			if (distance < stretchParams.MinDistance)
				return 0.0;

			if (distance > stretchParams.MaxDistance)
				return 1.0;

			return (distance - stretchParams.MinDistance) / (stretchParams.MaxDistance - stretchParams.MinDistance);
		}

		StretchDistanceParams EstimateStepStretchRange(const std::function<double(const Vector3Double&, const Vector3Double&, Vector3Double&)>& Marcher,
			const TriangleData& data, const std::function<void(double)>& setProgress, const TraceParams& traceParams, const ModelData& modelData)
		{
			double minDistance = std::numeric_limits<double>::max();
			double maxDistance = 0.0;

			size_t nVertices = data.Vertices.size();

			for (size_t nVertex = 0; nVertex < nVertices; nVertex += 3)
			{
				const XMFLOAT3& v = data.Vertices.at(nVertex);

				auto origin = traceParams.Bulb.Origin;
				if (modelData.OriginTargetType == OriginTarget::Parallel)
				{
					origin = v;
					origin.z = 0.0f;
				}

				XMFLOAT3 pt;
				if (traceParams.Fractal.Mandlelbrot)
					pt = MakeStartingPoint(traceParams.Bulb.Distance, origin, v, traceParams.Fractal.ConstantC);
				else
					pt = MakeStartingPoint(traceParams.Bulb.Distance, origin, v);

				Vector3Double direction = -1.0f * Vector3Double(v);
				Vector3Double p;
				double distance = Marcher(pt, direction, p);

				minDistance = std::min(minDistance, distance);
				maxDistance = std::max(maxDistance, distance);

				if (nVertex % 30)
					setProgress(static_cast<double>(nVertex) / nVertices);
			}

			StretchDistanceParams stretchParams = traceParams.Stretch;
			stretchParams.MinDistance = std::min(minDistance, GetMaxStretch());
			stretchParams.MaxDistance = std::min(maxDistance, GetMaxStretch());

			return stretchParams;
		}
	}

	class InternalDoubleRayTracer : public IRayTracer
	{
	private:
		std::function<double(Vector3Double&)> m_estimateDistance;
		TraceParams m_traceParams;
		ModelData m_modelData;

	protected:

		XMFLOAT3 CalculateNormal(const Vector3Double& p)
		{
			double distance = m_traceParams.Bulb.Distance;

			Vector3Double xp = Vector3Double(p.x + distance, p.y, p.z);
			double xPlus = m_estimateDistance(xp);

			Vector3Double xm = Vector3Double(p.x - distance, p.y, p.z);
			double xMinus = m_estimateDistance(xm);

			Vector3Double yp = Vector3Double(p.x, p.y + distance, p.z);
			double yPlus = m_estimateDistance(yp);

			Vector3Double ym = Vector3Double(p.x, p.y - distance, p.z);
			double yMinus = m_estimateDistance(ym);

			Vector3Double zp = Vector3Double(p.x, p.y, p.z + distance);
			double zPlus = m_estimateDistance(zp);

			Vector3Double zm = Vector3Double(p.x, p.y, p.z - distance);
			double zMinus = m_estimateDistance(zm);

			Vector3Double almostNormal(xPlus - xMinus, yPlus - yMinus, zPlus - zMinus);

			return almostNormal.Normalize().ToVector3();
		}

		double RayMarch(const Vector3Double& start, const Vector3Double& direction, Vector3Double& p)
		{
			double totalDistance = 0.0;
			int steps;

			double lastDistance = std::numeric_limits<double>::max();

			for (steps = 0; steps < m_traceParams.Bulb.MaxRaySteps; steps++)
			{
				p = totalDistance * direction + start;
				double distance = m_estimateDistance(p);
				totalDistance += distance / m_traceParams.Bulb.StepDivisor;                      // note change 
				if (distance < m_traceParams.Bulb.MinRayDistance)
					break;

				if (m_traceParams.Bulb.BlockDistanceIncrease && distance > lastDistance)
					break;

				if (totalDistance > m_traceParams.Bulb.MaxDistance)
					break;

				lastDistance = distance;
			}

			return 1.0 - ((double)steps) / m_traceParams.Bulb.MaxRaySteps;
		}

		double RayMarchFractional(const Vector3Double& start, const Vector3Double& direction, Vector3Double& p)
		{
			double totalDistance = 0.0;
			double distance = 0.0;
			int steps;

			double lastDistance = std::numeric_limits<double>::max();

			for (steps = 0; steps < m_traceParams.Bulb.MaxRaySteps; ++steps)
			{
				p = totalDistance * direction + start;
				distance = m_estimateDistance(p);
				totalDistance += distance / m_traceParams.Bulb.StepDivisor;
				if (distance < m_traceParams.Bulb.MinRayDistance)
					break;

				if (m_traceParams.Bulb.BlockDistanceIncrease && distance > lastDistance)
					break;

				if (totalDistance > m_traceParams.Bulb.MaxDistance)
					break;

				lastDistance = distance;
			}

			double delta = steps;
			if (steps < m_traceParams.Bulb.MaxRaySteps - 1 && distance < m_traceParams.Bulb.MinRayDistance)
				delta = steps + distance / m_traceParams.Bulb.MinRayDistance;

			delta = std::min(delta, static_cast<double>(m_traceParams.Bulb.MaxRaySteps));

			return 1.0 - delta / m_traceParams.Bulb.MaxRaySteps;
		}

		double RayMarchDistance(const Vector3Double& start, const Vector3Double& direction, Vector3Double& p)
		{
			double totalDistance = 0.0;
			int steps;

			double lastDistance = std::numeric_limits<double>::max();

			for (steps = 0; steps < m_traceParams.Bulb.MaxRaySteps; steps++)
			{
				p = totalDistance * direction + start;
				double distance = m_estimateDistance(p);
				totalDistance += distance / m_traceParams.Bulb.StepDivisor;                      // note change 
				if (distance < m_traceParams.Bulb.MinRayDistance)
					break;

				if (m_traceParams.Bulb.BlockDistanceIncrease && distance > lastDistance)
					break;

				if (totalDistance > m_traceParams.Bulb.MaxDistance)
					break;

				lastDistance = distance;
			}

			// We might want to set lastDistance to distance if it breaks on first step
			return lastDistance;
		}

		double CalculateStretchDistance(const Vector3Double& start, const Vector3Double& direction, Vector3Double& p, const StretchDistanceParams& stretchParams)
		{
			double distance = RayMarchDistance(start, direction, p);

			if (distance < stretchParams.MinDistance)
				return 0.0;

			if (distance > stretchParams.MaxDistance)
				return 1.0;

			return (distance - stretchParams.MinDistance) / (stretchParams.MaxDistance - stretchParams.MinDistance);
		}

		StretchDistanceParams EstimateStretchRange(const TriangleData& data, const std::function<void(double)>& setProgress)
		{
			double minDistance = std::numeric_limits<double>::max();
			double maxDistance = 0.0;

			size_t nVertices = data.Vertices.size();

			for (size_t nVertex = 0; nVertex < nVertices; nVertex += 3)
			{
				const XMFLOAT3& v = data.Vertices.at(nVertex);

				auto origin = m_traceParams.Bulb.Origin;
				if (m_modelData.OriginTargetType == OriginTarget::Parallel)
				{
					origin = v;
					origin.z = 0.0f;
				}

				XMFLOAT3 pt;
				if (m_traceParams.Fractal.Mandlelbrot)
					pt = MakeStartingPoint(m_traceParams.Bulb.Distance, origin, v, m_traceParams.Fractal.ConstantC);
				else
					pt = MakeStartingPoint(m_traceParams.Bulb.Distance, origin, v);

				Vector3Double dV = v;
				Vector3Double direction = -1.0 * dV;
				Vector3Double p;
				double distance = RayMarchDistance(pt, direction, p);

				minDistance = std::min(minDistance, distance);
				maxDistance = std::max(maxDistance, distance);

				if (nVertex % 30)
					setProgress(static_cast<double>(nVertex) / nVertices);
			}

			StretchDistanceParams stretchParams = m_traceParams.Stretch;
			stretchParams.MinDistance = minDistance;
			stretchParams.MaxDistance = maxDistance;

			return stretchParams;
		}

		std::shared_ptr<DxVertexData> RayTraceNormal(const TriangleData& data, const std::function<void(double)>& setProgress)
		{
			std::shared_ptr<DxVertexData> vData = std::make_shared<DxVertexData>();
			vData->StretchParams = m_traceParams.Stretch;

			double total = data.Vertices.size() + 2.0;
			int progress = 1;

			std::function<double(const Vector3Double&, const Vector3Double&, Vector3Double&)> marcher = nullptr;

			if (m_traceParams.Bulb.Fractional)
				marcher = [this](const Vector3Double& start, const Vector3Double& direction, Vector3Double& p)
			{
				Vector3Double startD(start);
				Vector3Double directionD(direction);
				Vector3Double pD(p);
				double distance = RayMarchFractional(startD, directionD, pD);
				p = pD.ToVector3();

				return distance;
			};
			else
				marcher = [this](const Vector3Double& start, const Vector3Double& direction, Vector3Double& p)
			{
				Vector3Double startD(start);
				Vector3Double directionD(direction);
				Vector3Double pD(p);
				double distance = RayMarch(startD, directionD, pD);
				p = pD.ToVector3();

				return distance;
			};

			for (const XMFLOAT3& v : data.Vertices)
			{
				auto origin = m_traceParams.Bulb.Origin;
				if (m_modelData.OriginTargetType == OriginTarget::Parallel)
				{
					origin = v;
					origin.z = 0.0f;
				}
					
				XMFLOAT3 pt;
				if(m_traceParams.Fractal.Mandlelbrot)
					pt = MakeStartingPoint(m_traceParams.Bulb.Distance, origin, v, m_traceParams.Fractal.ConstantC);
				else
					pt = MakeStartingPoint(m_traceParams.Bulb.Distance, origin, v);

				Vector3Double direction = -1.0 * Vector3Double(v - origin);
				Vector3Double p;
				double distance = marcher(pt, direction, p);

				auto normal = CalculateNormal(p);

				vData->Vertices.emplace_back(p.ToVector3(), normal, Vector2(static_cast<float>(distance), 0.0f));

				++progress;
				if (progress % 20 == 0)
					setProgress(progress / total);
			}

			for (const Triangle& t : data.Triangles)
			{
				vData->Indices.push_back(t.one);
				vData->Indices.push_back(t.two);
				vData->Indices.push_back(t.three);
			}

			CalculateNormals(*vData);

			setProgress(1.0);

			return vData;
		}

		std::shared_ptr<DxVertexData> RayTraceStepStretch(const TriangleData& data, const std::function<void(double)>& setProgress)
		{
			std::function<double(const Vector3Double&, const Vector3Double&, Vector3Double&)> marcher = nullptr;

			if (m_traceParams.Bulb.Fractional)
				marcher = [this](const Vector3Double& start, const Vector3Double& direction, Vector3Double& p)
			{
				Vector3Double startD(start);
				Vector3Double directionD(direction);
				Vector3Double pD(p);
				double distance = RayMarchFractional(startD, directionD, pD);
				p = pD.ToVector3();

				return distance;
			};
			else
				marcher = [this](const Vector3Double& start, const Vector3Double& direction, Vector3Double& p)
			{
				Vector3Double startD(start);
				Vector3Double directionD(direction);
				Vector3Double pD(p);
				double distance = RayMarch(startD, directionD, pD);
				p = pD.ToVector3();

				return distance;
			};

			// new part for estimating distance
			auto setLocalProgress = setProgress;

			if (m_traceParams.Stretch.EstimateMinMax)
			{
				setLocalProgress = [&](double progress)
				{
					setProgress(0.25 + 3.0 * progress / 4.0);
				};

				m_traceParams.Stretch = EstimateStepStretchRange(marcher, data, [&](double progress)
					{
						setProgress(progress / 4.0);
					}, m_traceParams, m_modelData);
			}

			// new part 2
			std::shared_ptr<DxVertexData> vData = std::make_shared<DxVertexData>();
			vData->StretchParams = m_traceParams.Stretch;

			double total = data.Vertices.size() + 2.0;
			int progress = 1;

			for (const XMFLOAT3& v : data.Vertices)
			{
				auto origin = m_traceParams.Bulb.Origin;
				if (m_modelData.OriginTargetType == OriginTarget::Parallel)
				{
					origin = v;
					origin.z = 0.0f;
				}

				XMFLOAT3 pt;
				if (m_traceParams.Fractal.Mandlelbrot)
					pt = MakeStartingPoint(m_traceParams.Bulb.Distance, origin, v, m_traceParams.Fractal.ConstantC);
				else
					pt = MakeStartingPoint(m_traceParams.Bulb.Distance, origin, v);

				Vector3Double direction = -1.0 * Vector3Double(v - origin);
				Vector3Double p;
				double distance = CalculateStepStretchDistance(marcher, pt, direction, p, m_traceParams.Stretch);

				auto normal = CalculateNormal(p);

				vData->Vertices.emplace_back(p.ToVector3(), normal, Vector2(static_cast<float>(distance), 0.0f));

				++progress;
				if (progress % 20 == 0)
					setLocalProgress(progress / total);
			}

			for (const Triangle& t : data.Triangles)
			{
				vData->Indices.push_back(t.one);
				vData->Indices.push_back(t.two);
				vData->Indices.push_back(t.three);
			}

			CalculateNormals(*vData);

			setProgress(1.0);

			return vData;
		}


	public:
		InternalDoubleRayTracer(const std::function<double(Vector3Double&)>& estimateDistance,
			const TraceParams& traceParams, const ModelData& modelData)
			: m_estimateDistance(estimateDistance)
			, m_traceParams(traceParams)
			, m_modelData(modelData)
		{}

		virtual ~InternalDoubleRayTracer()
		{}

		std::shared_ptr<DxVertexData> RayTrace(const TriangleData& data, const std::function<void(double)>& setProgress) override
		{
			if (m_traceParams.Stretch.Stretch)
				return RayTraceStepStretch(data, setProgress);

			return RayTraceNormal(data, setProgress);
		}


		std::shared_ptr<DxVertexData> RayTraceStretch(const TriangleData& data, const std::function<void(double)>& setProgress) override
		{
			auto setLocalProgress = setProgress;

			if (m_traceParams.Stretch.EstimateMinMax)
			{
				setLocalProgress = [&](double progress)
				{
					setProgress(0.25 + 3.0 * progress / 4.0);
				};

				m_traceParams.Stretch = EstimateStretchRange(data, [&](double progress)
					{
						setProgress(progress / 4.0);
					});
			}

			std::shared_ptr<DxVertexData> vData = std::make_shared<DxVertexData>();
			vData->StretchParams = m_traceParams.Stretch;

			double total = data.Vertices.size() + 2.0;
			int progress = 1;

			for (const XMFLOAT3& v : data.Vertices)
			{
				auto origin = m_traceParams.Bulb.Origin;
				if (m_modelData.OriginTargetType == OriginTarget::Parallel)
				{
					origin = v;
					origin.z = 0.0f;
				}

				XMFLOAT3 pt;
				if (m_traceParams.Fractal.Mandlelbrot)
					pt = MakeStartingPoint(m_traceParams.Bulb.Distance, origin, v, m_traceParams.Fractal.ConstantC);
				else
					pt = MakeStartingPoint(m_traceParams.Bulb.Distance, origin, v);

				Vector3Double direction = -1.0f * Vector3Double(v);
				Vector3Double p;
				double distance = CalculateStretchDistance(pt, direction, p, m_traceParams.Stretch);

				auto normal = CalculateNormal(p);

				vData->Vertices.emplace_back(p.ToVector3(), normal, Vector2(static_cast<float>(distance), 0.0f));

				++progress;
				if (progress % 20 == 0)
					setLocalProgress(progress / total);
			}

			for (const Triangle& t : data.Triangles)
			{
				vData->Indices.push_back(t.one);
				vData->Indices.push_back(t.two);
				vData->Indices.push_back(t.three);
			}

			CalculateNormals(*vData);

			setProgress(1.0);

			return vData;
		}
	};

	std::unique_ptr<IRayTracer> CreateInternalDoubleRayTracer(const TraceParams& traceParams, const ModelData& modelData,
		const std::function<double(Vector3Double&)>& estimateDistance)
	{
		return std::make_unique<InternalDoubleRayTracer>(estimateDistance, traceParams, modelData);
	}
}