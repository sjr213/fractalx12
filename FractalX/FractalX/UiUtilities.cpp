#include "stdafx.h"
#include "UiUtilities.h"

#include <filesystem>

namespace UiUtilities
{
	void SafeEnable(CWnd* pWnd, UINT ctrl, bool enable)
	{
		if (!pWnd)
		{
			ASSERT(false);
			return;
		}
		CWnd* pCtrl = pWnd->GetDlgItem(ctrl);
		if (pCtrl)
			pCtrl->EnableWindow(enable);
	}

	void SafeCheck(CWnd* pWnd, UINT ctrl, bool check)
	{
		if (!pWnd)
		{
			ASSERT(false);
			return;
		}

		CButton* pBut = (CButton*)pWnd->GetDlgItem(ctrl);
		if (pBut)
			pBut->SetCheck(check);
	}

	bool FileExists(CString fileName)
	{
		auto attr = GetFileAttributes(fileName); 
		if (INVALID_FILE_ATTRIBUTES == attr && GetLastError() == ERROR_FILE_NOT_FOUND)
			return false;

		return true;
	}

	void SafeShow(CWnd* pWnd, UINT ctrl, int show)
	{
		{
			if (!pWnd)
			{
				ASSERT(false);
				return;
			}
			CWnd* pCtrl = pWnd->GetDlgItem(ctrl);
			if (pCtrl)
				pCtrl->ShowWindow(show);
		}
	}

	bool IsValidFile(const std::wstring& filename, const std::wstring& expectedExt)
	{
		using std::filesystem::exists;

		if (filename.empty())
			return false;

		CString ext = PathFindExtension(filename.c_str());
		if (ext.IsEmpty())
			return false;

		CString fileExt = expectedExt.c_str();

		if (ext.MakeUpper() != fileExt.MakeUpper())
			return false;

		return exists(filename);
	}
}
