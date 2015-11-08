#pragma once
#include <string>
#include <Windows.h>
#include <vector>

#include <map>
#include "peml.h"
#include "peml_pe_ds.h"

using namespace std;

namespace pemetalib
{


class SafeAllocator
{
public:
	virtual void* addMalloc(void* o)=0;
	virtual void* addNew(LengthedStructField* o) = 0;
	virtual ~SafeAllocator() {};
};



#define LOG_ERROR(...) Log( false , __VA_ARGS__ )
#define LOG_DEBUG(...) Log( true , __VA_ARGS__ )

extern void Log(bool onlyVerbose, const wchar_t* format, ...);


 wstring GetLastErrorStdStrW();


extern SafeAllocator & safeAllocator;

bool UpdatePEManifest(HANDLE hExtractorExe, string& manifest);
bool UpdatePEIcon(HANDLE hExtractorExe, wstring & iconPath);
bool UpdatePEStrings(HANDLE hExtractorExe, const BYTE versionBytes[4], const VersionStrings& versionStrings);
}//namespace winreslib