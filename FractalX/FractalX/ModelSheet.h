#pragma once

#include "BackgroundPage.h"
#include "FractalParamsPage.h"
#include "ModelData.h"
#include "PositionRangePage.h"
#include "ShadowPage.h"
#include "TraceParams.h"
#include "TraceParamsPage.h"
#include "VertexPage.h"


class CModelSheet : public CMFCPropertySheet
{
	DECLARE_DYNAMIC(CModelSheet)

public:
	CModelSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CModelSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CModelSheet();

	void SetModelData(const DXF::ModelData& data);

	void SetTraceParams(std::shared_ptr<const DXF::TraceParams> traceParams);

	DXF::ModelData GetModelData() const;

	std::shared_ptr<DXF::TraceParams> GetTraceParams() const;

	bool GetAutoZoom() const;

	bool GetAutoTranslate() const;

	bool GetDefaultTargetAndWorkScale() const;

	void OnOk();

protected:
	void AddPages();

	DECLARE_MESSAGE_MAP()

	CVertexPage	m_vertexPage;

	CFractalParamsPage m_fractalParamsPage;

	CTraceParamsPage m_traceParamsPage;

	CPositionRangePage m_positionRangePage;

	CBackgroundPage m_backgroundPage;

	CShadowPage m_shadowPage;

	DXF::ModelData m_data;

	std::shared_ptr<DXF::TraceParams> m_traceParams;

	bool m_autoZoom = false;

	bool m_autoTranslate = false;

	bool m_defaultTargetAndWorldScale = false;
};