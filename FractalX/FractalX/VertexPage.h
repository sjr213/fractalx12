#pragma once

#include "OriginTarget.h"
#include "SeedTriangles.h"
#include "Vertex.h"
#include "VertexSource.h"

// CVertexPage

struct VertexRect
{
	DXF::Vertex<float> TL;
	DXF::Vertex<float> TR;
	DXF::Vertex<float> BL;
	DXF::Vertex<float> BR;
};

class CVertexPage : public CMFCPropertyPage
{
	DECLARE_DYNAMIC(CVertexPage)

public:
	CVertexPage();
	virtual ~CVertexPage();

	enum { IDD = IDD_VERTEX_PAGE };

	void SetVertexIterations(int iterations);

	int GetVertexIterations() const;

	void SetSeedTriangles(DXF::SeedTriangles seedTriangles);

	DXF::SeedTriangles GetSeedTriangles() const;

	void SetVertexSource(DXF::VertexSource vertexSource);

	DXF::VertexSource GetVertexSource() const;

	void SetVertices(const VertexRect& vRect);

	VertexRect GetVertices() const;

	void SetAutoZoom(bool autoZoom);

	bool GetAutoZoom() const;

	void SetAutoTranslate(bool autoTranslate);

	bool GetAutoTranslate() const;

	bool GetDefaultTargetAndWorkScale() const;

	void SetRectVerticesWide(int width);

	int GetRectVerticesWide() const;

	void SetRectVerticesHigh(int height);

	int GetRectVerticesHigh() const;

	void SetRectHalfWidth(double halfWidth);

	double GetRectHalfWidth() const;

	void SetRectHalfHeight(double halfHeight);

	double GetRectHalfHeight() const;

	void SetOriginTarget(DXF::OriginTarget originTarget);

	DXF::OriginTarget GetOriginTarget() const;

protected:
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);

	virtual BOOL OnSetActive();

	void OnOK() override;

	void CalculateStatistics();

	void PopulateSeedTriangleCombo();

	void PopulateOriginTargetCombo();

	void EnableCtrls();

	bool VerticesChanged();

	bool AreVerticesUnique();

	void UpdateAutoZoomCtrls();

	afx_msg void OnKillfocusIterationEdit();

	afx_msg void OnCbnSelchangeSeedTrianglesCombo();

	afx_msg void OnCbnSelchangeOriginTargetCombo();

	afx_msg void OnBnClickSphericalVertexRad();

	afx_msg void OnBnClickCustomVertexRad();

	afx_msg void OnBnClickRectangularVertexRad();

	afx_msg void OnKillFocusVertex();

	CComboBox m_SeedTrianglesCombo;

	// Data

	int m_vertexIterations  = 3;

	DXF::SeedTriangles m_seedTriangles =  DXF::SeedTriangles::Eight;

	int m_vertexNumber = 0;

	int m_triangleNumber = 0;

	DXF::VertexSource m_vertexSource = DXF::VertexSource::Spherical;

	DXF::Vertex<float> m_vertexTL_original;
	DXF::Vertex<float> m_vertexTR_original;
	DXF::Vertex<float> m_vertexBL_original;
	DXF::Vertex<float> m_vertexBR_original;

	DXF::Vertex<float> m_vertexTL;
	DXF::Vertex<float> m_vertexTR;
	DXF::Vertex<float> m_vertexBL;
	DXF::Vertex<float> m_vertexBR;

	BOOL m_autoZoom = FALSE;
	BOOL m_autoTranslate = FALSE;
	bool m_enableCustomVertexes = false;
	BOOL m_defaultTargetAndWorldScale = FALSE;

	int m_rectVerticesWide = 1000;
	int m_rectVerticesHigh = 1000;
	DXF::OriginTarget m_originTarget = DXF::OriginTarget::Point;
	double m_rectHalfWidth = 1.0;
	double m_rectHalfHeight = 1.0;

	CComboBox m_originTargetCombo;
};


