#pragma once
#include "Resource.h"
#include "Vertex.h"

class CBackgroundPage : public CMFCPropertyPage
{
	DECLARE_DYNAMIC(CBackgroundPage)

public:
	CBackgroundPage();
	virtual ~CBackgroundPage() = default;

	enum { IDD = IDD_BACKGROUND_PAGE };

	void SetFilename(const std::wstring& filename);

	std::wstring GetFilename() const;

	void SetShowBackground(std::shared_ptr<bool>& isBackground);

	bool GetShowBackground() const;

	void SetModelVertices(const std::vector<DXF::Vertex<float>>& model);

	std::vector<DXF::Vertex<float>> GetModelVertices() const;

protected:
	DECLARE_MESSAGE_MAP()

	void DoDataExchange(CDataExchange* pDX) override;

	BOOL OnInitDialog() override;

	BOOL OnSetActive() override;

	BOOL OnKillActive() override;

	void OnOK() override;

	void OnBrowseBut();

	void EnableCtrls();

	bool NeedFileName();

	void EnableOK(bool enable);

	afx_msg void OnShowClicked();

	afx_msg void DefaultModelClicked();

	afx_msg void OnKillFocusImageFile();

private:
	CString m_filename;
	std::shared_ptr<bool> m_showBackground = std::make_shared<bool>(false);
	std::vector<DXF::Vertex<float>> m_backgroundModel;
};