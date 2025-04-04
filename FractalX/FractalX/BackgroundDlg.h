#pragma once

#include "ColorArgb.h"
#include "ShadowWorldType.h"
#include <Vertex.h>

class CBackgroundDlg : public CDialogEx
{
public:
	virtual ~CBackgroundDlg() = default;

	virtual void SetFilename(const std::wstring& filename) = 0;

	virtual std::wstring GetFilename() const = 0;

	virtual void SetShowBackground(bool show) = 0;

	virtual bool GetShowBackground() const = 0;

	virtual void SetModelVertices(const std::vector<DXF::Vertex<float>>& model) = 0;

	virtual std::vector<DXF::Vertex<float>> GetModelVertices() const = 0;

	virtual void SetShowShadow(bool showShadow) = 0;

	virtual bool GetShowShadow() const = 0;

	virtual void SetShadowWorldType(DXF::ShadowWorldType worldType) = 0;

	virtual DXF::ShadowWorldType GetShadowWorldType() const = 0;

	virtual DxColor::ColorArgb GetShadowColor() const = 0;

	virtual void SetShadowColor(DxColor::ColorArgb color) = 0;

	static std::shared_ptr<CBackgroundDlg> CreateBackgroundDlg(CWnd* pParent = nullptr);

protected:
	CBackgroundDlg(UINT nIDTemplate, CWnd* pParent)
		: CDialogEx(nIDTemplate, pParent)
	{}
};