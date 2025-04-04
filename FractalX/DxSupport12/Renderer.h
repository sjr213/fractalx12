
#pragma once

#include "ModelData.h"
#include <optional>
#include "Perspective.h"
#include <string>
#include <tuple>
#include "Vertex.h"
#include "VertexData.h"

class CDC;

namespace DXF
{
	struct DxEffectColors;
	struct RotationGroup;
	struct DxLights;
	struct DxBackgroundVertexData;

	class Renderer
	{
	public:
		virtual ~Renderer()
		{
		}

		// Initialization and management
		virtual void Initialize(HWND window, int width, int height) = 0;

		virtual bool WasInitialized() const = 0;

		virtual void OnDeviceLost() = 0;

		// Basic game loop
		virtual void Tick() = 0;

		virtual void RefreshRender(float time) = 0;

		// Messages
		virtual void OnWindowSizeChanged(int width, int height) = 0;

		// Properties

		virtual void SetModel(const DxVertexData& vertexData) = 0;

		virtual void SetModel2(const DxBackgroundVertexData& bkgndVertexData, const BackgroundImageParams& bkgndParams) = 0;

		virtual void ResetModel2() = 0;

		virtual void SetRotationGroup(const RotationGroup& rg) = 0;

		virtual RotationGroup GetRotationGroup() = 0;

		virtual void SetTextureColors(std::vector<uint32_t> colors) = 0;

		virtual void ReloadTextures() = 0;

		virtual void SetPerspective(const DxPerspective& perspective) = 0;

		virtual DxPerspective GetPerspective() const = 0;

		virtual void SetView(const Vertex<float>& camera, const Vertex<float>& target, const Vertex<float>& targetBackgnd) = 0;

		virtual void SetTarget(const Vertex<float>& target) = 0;

		virtual void SetTargetBackground(const Vertex<float>& targetBackgnd) = 0;

		virtual Vertex<float> GetCamera() const = 0;

		virtual Vertex<float> GetTarget() const = 0;

		virtual Vertex<float> GetTargetBackground() const = 0;

		virtual bool IsReady() = 0;

		virtual CSize GetScreenSize() const = 0;

		virtual bool DrawImage(const CString& filename, const CString& ext) = 0;

		virtual void SetBackgroundColor(DirectX::SimpleMath::Color bkColor) = 0;

		virtual void SetEffectColors(DxEffectColors effectColors) = 0;

		virtual void SetLights(DxLights lights) = 0;

		virtual std::optional<DXF::Vertex<float>> Map2Dto3D(int x, int y) = 0;

		virtual void SetWorldScale(const DXF::Vertex<float>& scale) = 0;

		virtual Vertex<float> GetWorldScale() const = 0;

		virtual void SetBackgroundScale(const DXF::Vertex<float>& scale) = 0;

		virtual Vertex<float> GetBackgroundScale() const = 0;

		virtual void RebuildModelsAndTexture() = 0;

		virtual void SetSelectedRect(const CRect& client, CPoint topLeft, CPoint bottomRight, bool draw) = 0;

	protected:
		Renderer() {}
	};
}
