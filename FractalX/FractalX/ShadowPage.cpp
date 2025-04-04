#include "stdafx.h"
#include "ShadowPage.h"

#include "DefaultFields.h"
#include "ModelSheet.h"
#include "NarrowCast.h"
#include "UiUtilities.h"

using namespace DXF;

IMPLEMENT_DYNAMIC(CShadowPage, CMFCPropertyPage)

BEGIN_MESSAGE_MAP(CShadowPage, CMFCPropertyPage)
	ON_BN_CLICKED(IDC_SHOW_SHADOW_CHECK, &CShadowPage::OnShowClicked)
END_MESSAGE_MAP()

CShadowPage::CShadowPage()
	: CMFCPropertyPage(CShadowPage::IDD)
	, m_worldTypeIndex(ShadowWorldTypeToInt(ShadowWorldType::MainModel) - 1)
	, m_shadowColor(GetDefaultShadowColor())
{}

void CShadowPage::SetShowShadow(bool show)
{
	m_showShadow = show;
}

bool CShadowPage::GetShowShadow() const
{
	return m_showShadow;
}

void CShadowPage::SetShadowWorldType(DXF::ShadowWorldType worldType)
{
	m_worldTypeIndex = ShadowWorldTypeToInt(worldType) - 1;
}

DXF::ShadowWorldType CShadowPage::GetShadowWorldType() const
{
	return ShadowWorldTypeFromInt(m_worldTypeIndex + 1);
}

void CShadowPage::SetShadowColor(const DxColor::ColorArgb& color)
{
	m_shadowColor = color;
}

DxColor::ColorArgb CShadowPage::GetShadowColor() const
{
	return m_shadowColor;
}

void CShadowPage::SetIsBackground(std::shared_ptr<bool>& isBackground)
{
	m_isBackground = isBackground;
}

void CShadowPage::DoDataExchange(CDataExchange* pDX)
{
	CMFCPropertyPage::DoDataExchange(pDX);

	BOOL show = m_showShadow;
	DDX_Check(pDX, IDC_SHOW_SHADOW_CHECK, show);
	m_showShadow = show;

	DDX_Control(pDX, IDC_SHADOW_WORLD_COMBO, m_worldTypeCombo);

	int alpha = m_shadowColor.A;
	DDX_Text(pDX, IDC_ALPHA_EDIT, alpha);
	DDV_MinMaxInt(pDX, alpha, 0, 255);
	m_shadowColor.A = DxCore::narrow_cast<bite>(alpha);

	int red = m_shadowColor.R;
	DDX_Text(pDX, IDC_RED_EDIT, red);
	DDV_MinMaxInt(pDX, red, 0, 255);
	m_shadowColor.R = DxCore::narrow_cast<bite>(red);

	int green = m_shadowColor.G;
	DDX_Text(pDX, IDC_GREEN_EDIT, green);
	DDV_MinMaxInt(pDX, green, 0, 255);
	m_shadowColor.G = DxCore::narrow_cast<bite>(green);

	int blue = m_shadowColor.B;
	DDX_Text(pDX, IDC_BLUE_EDIT, blue);
	DDV_MinMaxInt(pDX, blue, 0, 255);
	m_shadowColor.B = DxCore::narrow_cast<bite>(blue);
}

BOOL CShadowPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	InitializeWorldTypeCombo();

	EnableCtrls();

	return TRUE;
}

void CShadowPage::InitializeWorldTypeCombo()
{
	m_worldTypeCombo.InsertString(0, ShadowWorldTypeString(ShadowWorldType::MainModel));
	m_worldTypeCombo.InsertString(1, ShadowWorldTypeString(ShadowWorldType::Background));

	m_worldTypeCombo.SetCurSel(m_worldTypeIndex);
}

BOOL CShadowPage::OnSetActive()
{
	EnableCtrls();

	return CPropertyPage::OnSetActive();
}

BOOL CShadowPage::OnKillActive()
{
	UpdateData(TRUE);

	return CMFCPropertyPage::OnKillActive();
}

void CShadowPage::OnOK()
{
	CModelSheet* pModelSheet = dynamic_cast<CModelSheet*>(GetParent());
	if (pModelSheet)
		pModelSheet->OnOk();

	CMFCPropertyPage::OnOK();
}

void CShadowPage::EnableCtrls()
{
	UiUtilities::SafeEnable(this, IDC_SHOW_SHADOW_CHECK, *m_isBackground);
	UiUtilities::SafeEnable(this, IDC_SHADOW_WORLD_COMBO, m_showShadow && *m_isBackground);
	UiUtilities::SafeEnable(this, IDC_ALPHA_EDIT, m_showShadow && *m_isBackground);
	UiUtilities::SafeEnable(this, IDC_RED_EDIT, m_showShadow && *m_isBackground);
	UiUtilities::SafeEnable(this, IDC_GREEN_EDIT, m_showShadow && *m_isBackground);
	UiUtilities::SafeEnable(this, IDC_BLUE_EDIT, m_showShadow && *m_isBackground);
}

void CShadowPage::OnShowClicked()
{
	UpdateData(TRUE);
	EnableCtrls();
}