#include "stdafx.h"
#include "BackgroundDlg.h"

#include "DefaultFields.h"
#include "NarrowCast.h"
#include "Resource.h"
#include "UiUtilities.h"

using namespace DXF;
using namespace std;

class CBackgroundDlgImp : public CBackgroundDlg
{
private:
	CString m_filename;
	bool m_showBackground = false;
	std::vector<DXF::Vertex<float>> m_backgroundModel;
	bool m_showShadow = false;
	int m_worldTypeIndex;
	DxColor::ColorArgb m_shadowColor = DXF::GetDefaultShadowColor();
	CComboBox m_worldTypeCombo;

public:
	CBackgroundDlgImp(CWnd* pParent)
		: CBackgroundDlg(IDD_BACKGROUND_SHADOW_DLG, pParent)
		, m_worldTypeIndex(ShadowWorldTypeToInt(ShadowWorldType::MainModel) - 1)
		, m_backgroundModel(4)
	{}

	virtual ~CBackgroundDlgImp()
	{}

	void SetFilename(const std::wstring& filename) override
	{
		m_filename = filename.c_str();
	}

	std::wstring GetFilename() const override
	{
		std::wstring str(m_filename.GetString());
		return str;
	}

	void SetShowBackground(bool show) override
	{
		m_showBackground = show;
	}

	bool GetShowBackground() const override
	{
		return m_showBackground;
	}

	void SetModelVertices(const std::vector<DXF::Vertex<float>>& model) override
	{
		if (model.size() != 4)
		{
#ifdef DEBUG
			assert(false);
#endif
			return;
		}
		m_backgroundModel = model;
	}

	std::vector<DXF::Vertex<float>> GetModelVertices() const override
	{
		return m_backgroundModel;
	}

	void SetShowShadow(bool showShadow) override
	{
		m_showShadow = showShadow;
	}

	bool GetShowShadow() const override
	{
		return m_showShadow;
	}

	void SetShadowWorldType(DXF::ShadowWorldType worldType) override
	{
		m_worldTypeIndex = ShadowWorldTypeToInt(worldType) - 1;
	}

	DXF::ShadowWorldType GetShadowWorldType() const override
	{
		return ShadowWorldTypeFromInt(m_worldTypeIndex + 1);
	}

	DxColor::ColorArgb GetShadowColor() const override
	{
		return m_shadowColor;
	}

	void SetShadowColor(DxColor::ColorArgb color) override
	{
		m_shadowColor = color;
	}

	void DoDataExchange(CDataExchange* pDX)
	{
		CDialogEx::DoDataExchange(pDX);

		DDX_Text(pDX, IDC_IMAGE_FILE_EDIT, m_filename);

		BOOL show = m_showBackground;
		DDX_Check(pDX, IDC_SHOW_BACKGROUND_CHECK, show);
		m_showBackground = show;

		DDX_Text(pDX, IDC_V1_X_EDIT, m_backgroundModel.at(0).X);
		DDX_Text(pDX, IDC_V1_Y_EDIT, m_backgroundModel.at(0).Y);
		DDX_Text(pDX, IDC_V1_Z_EDIT, m_backgroundModel.at(0).Z);

		DDX_Text(pDX, IDC_V2_X_EDIT, m_backgroundModel.at(1).X);
		DDX_Text(pDX, IDC_V2_Y_EDIT, m_backgroundModel.at(1).Y);
		DDX_Text(pDX, IDC_V2_Z_EDIT, m_backgroundModel.at(1).Z);

		DDX_Text(pDX, IDC_V3_X_EDIT, m_backgroundModel.at(2).X);
		DDX_Text(pDX, IDC_V3_Y_EDIT, m_backgroundModel.at(2).Y);
		DDX_Text(pDX, IDC_V3_Z_EDIT, m_backgroundModel.at(2).Z);

		DDX_Text(pDX, IDC_V4_X_EDIT, m_backgroundModel.at(3).X);
		DDX_Text(pDX, IDC_V4_Y_EDIT, m_backgroundModel.at(3).Y);
		DDX_Text(pDX, IDC_V4_Z_EDIT, m_backgroundModel.at(3).Z);

		BOOL showShadow = m_showShadow;
		DDX_Check(pDX, IDC_SHOW_SHADOW_CHECK, showShadow);
		m_showShadow = showShadow;

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

	void InitializeWorldTypeCombo()
	{
		m_worldTypeCombo.InsertString(0, ShadowWorldTypeString(ShadowWorldType::MainModel));
		m_worldTypeCombo.InsertString(1, ShadowWorldTypeString(ShadowWorldType::Background));

		m_worldTypeCombo.SetCurSel(m_worldTypeIndex);
	}

	BOOL OnInitDialog()
	{
		EnableWindow(TRUE);
		ShowWindow(SW_SHOW);

		CDialogEx::OnInitDialog();

		UiUtilities::SafeShow(this, IDOK, SW_SHOW);
		UiUtilities::SafeEnable(this, IDOK, true);
		UiUtilities::SafeShow(this, IDCANCEL, SW_SHOW);
		UiUtilities::SafeEnable(this, IDCANCEL, true);

		InitializeWorldTypeCombo();

		EnableCtrls();

		return TRUE;
	}

	void OnBrowseBut()
	{
		CString fileName = m_filename;
		CString fileType = _T("DDS Files (*.dds)|*.dds|All Files (*.*)|*.*||");
		CString fileExt = _T(".dds");

		// Open a Save As dialog to get a name
		CFileDialog MyOpenDlg(TRUE, fileExt, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, fileType);
		if (MyOpenDlg.DoModal() != IDOK)
			return;

		m_filename = MyOpenDlg.GetPathName();
		UpdateData(FALSE);
	}

	void OnOK() override
	{
		UpdateData(TRUE);
		if (m_showBackground && (m_filename.IsEmpty() || !UiUtilities::FileExists(m_filename)))
		{
			AfxMessageBox(_T("Current file is not valid!"));
			return;
		}

		CDialogEx::OnOK();
	}

protected:
	DECLARE_MESSAGE_MAP()

	void EnableCtrls()
	{
		UiUtilities::SafeEnable(this, IDC_BROWSE_BUT, m_showBackground);
		UiUtilities::SafeEnable(this, IDC_IMAGE_FILE_EDIT, m_showBackground);

		UiUtilities::SafeEnable(this, IDC_SHOW_SHADOW_CHECK, m_showBackground);
		UiUtilities::SafeEnable(this, IDC_SHADOW_WORLD_COMBO, m_showShadow && m_showBackground);
		UiUtilities::SafeEnable(this, IDC_ALPHA_EDIT, m_showShadow && m_showBackground);
		UiUtilities::SafeEnable(this, IDC_RED_EDIT, m_showShadow && m_showBackground);
		UiUtilities::SafeEnable(this, IDC_GREEN_EDIT, m_showShadow && m_showBackground);
		UiUtilities::SafeEnable(this, IDC_BLUE_EDIT, m_showShadow && m_showBackground);
	}

	void OnShowClicked()
	{
		UpdateData(TRUE);
		EnableCtrls();
	}

	void DefaultModelClicked()
	{
		m_backgroundModel = GetDefaultBackgroundModel();
		UpdateData(FALSE);
	}
};

BEGIN_MESSAGE_MAP(CBackgroundDlgImp, CBackgroundDlg)
	ON_BN_CLICKED(IDC_BROWSE_BUT, &CBackgroundDlgImp::OnBrowseBut)
	ON_BN_CLICKED(IDC_SHOW_BACKGROUND_CHECK, &CBackgroundDlgImp::OnShowClicked)
	ON_BN_CLICKED(IDC_DEFAULT_BUT, &CBackgroundDlgImp::DefaultModelClicked)
	ON_BN_CLICKED(IDC_SHOW_SHADOW_CHECK, &CBackgroundDlgImp::OnShowClicked)
END_MESSAGE_MAP()


shared_ptr<CBackgroundDlg> CBackgroundDlg::CreateBackgroundDlg(CWnd* pParent)
{
	return make_shared<CBackgroundDlgImp>(pParent);
}