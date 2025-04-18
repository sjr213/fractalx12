
// FractalXDoc.h : interface of the CFractalXDoc class
//

#pragma once

#include "Vertex.h"

struct FractalXDocImpl;
class CFractalXView;

namespace DXF {
	struct ModelData;
	struct DxBackgroundVertexData;
	struct DxVertexData;
	struct TraceParams;
	struct RotationGroup;
	struct DxPerspective;
}

namespace DxColor
{
	struct ColorArgb;
	struct PinPalette;
	struct ColorContrast;
	struct EffectColors;
	struct Lights;
}

class CFractalXDoc : public CDocument
{
protected: // create from serialization only
	CFractalXDoc();
	DECLARE_DYNCREATE(CFractalXDoc)

// Attributes
public:
	const DXF::ModelData& GetModelData() const;
	void SetModelData(const DXF::ModelData& modelData);

	const std::shared_ptr<DXF::DxVertexData> GetVertexData() const;
	void SetVertexData(std::shared_ptr<DXF::DxVertexData>& vertexData);

	const std::shared_ptr<DXF::DxBackgroundVertexData> GetBackgroundVertexData() const;
	void SetBackgroundVertexData(std::shared_ptr<DXF::DxBackgroundVertexData>& bkgndVertexData);

	std::shared_ptr<const DXF::TraceParams> GetTraceParams() const;
	void SetTraceParams(std::shared_ptr<DXF::TraceParams> params);

	DxColor::PinPalette GetPalette() const;
	void SetPalette(const DxColor::PinPalette& palette);

	DxColor::ColorContrast GetContrast() const;
	void SetContrast(const DxColor::ColorContrast& contrast);

	DXF::RotationGroup GetRotationGroup() const;
	void SetRotationGroup(const DXF::RotationGroup& rotationGroup);

	CSize GetSize() const;
	void SetSize(CSize size);

	DXF::Vertex<float> GetCamera() const;
	void SetCamera(const DXF::Vertex<float>& camera);

	DXF::Vertex<float> GetTarget() const;
	void SetTarget(const DXF::Vertex<float>& target);

	DXF::Vertex<float> GetTargetBackground() const;
	void SetTargetBackground(const DXF::Vertex<float>& target);

	DXF::DxPerspective GetPerspective() const;
	void SetPerspective(const DXF::DxPerspective& perspective);

	DxColor::ColorArgb GetBackgroundColor() const;
	void SetBackgroundColor(DxColor::ColorArgb bkColor);

	DxColor::EffectColors GetEffectColors() const;
	void SetEffectColors(const DxColor::EffectColors& effectColors);

	DxColor::Lights GetLights() const;
	void SetLights(const DxColor::Lights lights);

	DXF::Vertex<float> GetWorldScale() const;
	void SetWorldScale(const DXF::Vertex<float>& worldScale);

	DXF::Vertex<float> GetBackgroundScale() const;
	void SetBackgroundScale(const DXF::Vertex<float>& backgroundScale);

// Operations
public:

// Overrides
public:
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CFractalXDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	CFractalXView* GetView();

// Generated message map functions

	DECLARE_MESSAGE_MAP()

	std::unique_ptr<FractalXDocImpl> m_impl;

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS

	afx_msg void OnFileSaveAs();
	afx_msg void OnUpdateFileSaveAs(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileSave(CCmdUI *pCmdUI);
};
