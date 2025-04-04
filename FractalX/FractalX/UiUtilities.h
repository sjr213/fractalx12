#pragma once

namespace UiUtilities
{
	void SafeEnable(CWnd* pWnd, UINT ctrl, bool enable);

	void SafeCheck(CWnd* pWnd, UINT ctrl, bool check);

	bool FileExists(CString fileName);

	void SafeShow(CWnd* pWnd, UINT ctrl, int show);

	// Need to include . in expectedExt i.e. ".dds"
	bool IsValidFile(const std::wstring& filename, const std::wstring& expectedExt);
}