
// FractalXView.h : interface of the CFractalXView class
//

#pragma once

class CFractalXViewImp;

class CFractalXView : public CView
{
protected:
	CFractalXView();

	DECLARE_DYNCREATE(CFractalXView)

	virtual ~CFractalXView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	std::unique_ptr<CFractalXViewImp> m_impl;

public:
	CFractalXDoc* GetDocument() const;

	void OnInitialUpdate() override;
	void OnDraw(CDC* pDC) override; 

	// ext should be "bmp", "jpg" or "png" 
	void SaveImage(const CString& filename, const CString& ext);

protected:

	void OnUpdate(CView* sender, LPARAM lHint, CObject* pHint) override;

	void ShowContextMenu(CWnd* /* pWnd */, CPoint point);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
//	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	afx_msg void OnEditSetModel();
	afx_msg void OnUpdateEditSetModel(CCmdUI* pCmdUI);
	afx_msg void OnEditSetBackground();
	afx_msg void OnUpdateEditSetBackground(CCmdUI* pCmdUI);
	afx_msg void OnEditAnglePosition();
	afx_msg void OnUpdateEditAnglePosition(CCmdUI* pCmdUI);
	afx_msg void OnEditLights();
	afx_msg void OnUpdateEditLights(CCmdUI* pCmdUI);
	afx_msg void OnEditUpdateRotation();
	afx_msg void OnUpdateEditRotation(CCmdUI *pCmdUI);
	afx_msg void OnEditStopRotation();
	afx_msg void OnUpdateEditStopRotation(CCmdUI* pCmdUI);
	afx_msg void OnEditPerspective();
	afx_msg void OnUpdateEditSetPerspective(CCmdUI *pCmdUI);
	afx_msg void OnEditView();
	afx_msg void OnUpdateEditSetView(CCmdUI *pCmdUI);
	afx_msg void OnEditBackgroundColor();
	afx_msg void OnUpdateEditBackgroundColor(CCmdUI* pCmdUI);
	afx_msg void OnEditPalette();
	afx_msg void OnUpdateEditPalette(CCmdUI* pCmdUI);
	afx_msg void OnEditEffectColors();
	afx_msg void OnUpdateEditEffectColors(CCmdUI* pCmdUI);
	afx_msg void OnEditSelectPalette();
	afx_msg void OnUpdateEditSelectPalette(CCmdUI* pCmdUI);
	afx_msg void OnSaveSelectedVertices();
	afx_msg void OnUpdateSaveSelectedVertices(CCmdUI* pCmdUI);
	afx_msg void OnEditSetWorldScale();
	afx_msg void OnUpdateEditSetWorldScale(CCmdUI* pCmdUI);
	afx_msg void OnEditImageSize();
	afx_msg void OnUpdateEditImageSize(CCmdUI* pCmdUI);

	// temp
	afx_msg void OnExportTriangles();

	afx_msg LRESULT OnUpdateProgress(WPARAM, LPARAM);
	afx_msg LRESULT OnCalcFinished(WPARAM, LPARAM);
	afx_msg LRESULT OnKillPaletteView(WPARAM, LPARAM);
	afx_msg LRESULT OnKillPositionAngleDlg(WPARAM wparam, LPARAM);
	afx_msg LRESULT OnModelPositionChanged(WPARAM, LPARAM);
	afx_msg LRESULT OnModelAngleChanged(WPARAM, LPARAM);
	afx_msg LRESULT OnRender(WPARAM, LPARAM);
};

#ifndef _DEBUG  // debug version in FractalXView.cpp
inline CFractalXDoc* CFractalXView::GetDocument() const
   { return reinterpret_cast<CFractalXDoc*>(m_pDocument); }
#endif

