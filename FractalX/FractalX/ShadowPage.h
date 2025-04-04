#pragma once

#include "ColorArgb.h"
#include "Resource.h"
#include "ShadowWorldType.h"

class CShadowPage : public CMFCPropertyPage
{
	DECLARE_DYNAMIC(CShadowPage)

public:
	CShadowPage();
	virtual ~CShadowPage() = default;

	enum { IDD = IDD_SHADOW_PAGE };

	void SetShowShadow(bool show);

	bool GetShowShadow() const;

	void SetShadowWorldType(DXF::ShadowWorldType worldType);

	DXF::ShadowWorldType GetShadowWorldType() const;

	void SetShadowColor(const DxColor::ColorArgb& color);

	DxColor::ColorArgb GetShadowColor() const;

	void SetIsBackground(std::shared_ptr<bool>& isBackground);

protected:
	DECLARE_MESSAGE_MAP()

	void DoDataExchange(CDataExchange* pDX) override;

	BOOL OnInitDialog() override;

	BOOL OnSetActive() override;

	BOOL OnKillActive() override;

	void OnOK() override;

	void InitializeWorldTypeCombo();

	void EnableCtrls();

	afx_msg void OnShowClicked();

private:
	std::shared_ptr<bool> m_isBackground = std::make_shared<bool>(false);
	bool m_showShadow = false;
	int m_worldTypeIndex;
	DxColor::ColorArgb m_shadowColor;
	CComboBox m_worldTypeCombo;
};