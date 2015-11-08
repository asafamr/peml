#pragma once
#include <windows.h>
//#include <tchar.h>
#include <string>




using namespace std;
namespace pemetalib
{
	typedef void(*LogCallback)(wstring& msg, int level, int code);

	

	struct VersionStrings
	{
		wstring CompanyName;
		wstring FileDescription;
		wstring FileVersion;
		wstring InternalName;
		wstring LegalCopyright;
		wstring LegalTrademarks;
		wstring OriginalFilename;
		wstring ProductName;
		wstring ProductVersion;
		wstring Comments;
	};

	struct ExeResources
	{
		ExeResources()
			{}
		VersionStrings versionstrings;
		wstring iconPath;
		wstring manifest;
		//version[3] most significat
		BYTE versionBytes[4];
	};

	bool UpdateExeResources(const ExeResources & resources, const wstring & exePath);

	void SetLogCallback(LogCallback& logcb);

}