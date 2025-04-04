#pragma once

#include <SimpleMath.h>
#include "TraceParams.h"

class CPositionRangePage : public CMFCPropertyPage
{
	DECLARE_DYNAMIC(CPositionRangePage)

public:
	CPositionRangePage();
	virtual ~CPositionRangePage();

	enum { IDD = IDD_POSITION_RANGE_PAGE };

	void SetOrigin(const DirectX::SimpleMath::Vector3 origin);

	DirectX::SimpleMath::Vector3 GetOrigin() const;

	void SetDistance(float distance);

	float GetDistance() const;

	void SetStretchParams(const DXF::StretchDistanceParams& stretchParams);

	DXF::StretchDistanceParams GetStretchParams() const;

	void SetBlockDistanceIncrease(bool blockIncrease);

	bool GetBlockDistanceIncrease() const;

protected:
	DECLARE_MESSAGE_MAP()

	virtual void DoDataExchange(CDataExchange* pDX);

	virtual BOOL OnSetActive();

	void OnOK() override;

	void OnKillfocusEdit();

	void OnStretchDistanceClicked();

	void OnStretchClicked();

	void EnableStretchCtrls();

private:

	DirectX::SimpleMath::Vector3 m_origin;
	float m_distance = 0.0;
	DXF::StretchDistanceParams m_stretchParams;
	bool m_blockDistanceIncrease = true;
};
