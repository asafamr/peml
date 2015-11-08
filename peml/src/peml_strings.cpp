// this file code is responsible for generating the PE string metadata
// i.e product name, version, trademarks etc which can be viewed in windows explorer

#include "../include/peml_internal.h"

using namespace pemetalib;

LengthedStructField * getStringStructLSField(const wstring& key, const wstring& val)
{
	/*WORD  wLength;
	WORD  wValueLength;
	WORD  wType;
	wchar[] szKey;
	WORD  Padding; to 32bit
	wchar[] Value;*/

	LSFieldString* szKeyField = (LSFieldString*)safeAllocator.addNew((LengthedStructField*)new LSFieldString(key));
	LSFieldString* valField = (LSFieldString*)safeAllocator.addNew((LengthedStructField*)new LSFieldString(val));

	LengthedStruct* thisString = (LengthedStruct*)safeAllocator.addNew((LengthedStructField*)new LengthedStruct());
	thisString->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWordSizeOfOtherField((LengthedStructField*)thisString)));
	thisString->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWordSizeOfOtherField((LengthedStructField*)valField, true)));
	thisString->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWord(1)));
	thisString->addField((LengthedStructField*)szKeyField);
	thisString->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSField32BitPadding()));
	thisString->addField((LengthedStructField*)valField);
	return (LengthedStructField *)thisString;
}

bool addStringToStructIfNotEmpty(LengthedStruct * lsStruct, const wstring& wstr, const wstring& strKey)
{
	if (wstr== L"")
	{
		return false;
	}

	if (wstr.length() > 1024)
	{
		LOG_ERROR(L"%s is too long(%d chars) - not inserting to PE header.", strKey, wstr.length());
		return false;
	}
	//if (!str.empty())
	{
		LengthedStructField* field = getStringStructLSField(strKey, wstr);
		if (field == NULL)
		{
			LOG_ERROR(L"Could not create StringStruct for %s %s", strKey.c_str(), wstr.c_str());
			return false;
		}
		else
		{
			lsStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSField32BitPadding()));
			lsStruct->addField(getStringStructLSField(strKey, wstr));

			return true;
		}
	}
	return false;
}
LengthedStructField * getStringTableStruct(const VersionStrings& versionStrings)
{
	/*struct {
		WORD   wLength;
		WORD   wValueLength;(=0)
		WORD   wType;
		WCHAR  szKey[8];
		WORD   Padding;
		//String Children;
	} ;*/

	LengthedStruct* thisStringTable = (LengthedStruct*)safeAllocator.addNew((LengthedStructField*)new LengthedStruct());
	thisStringTable->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWordSizeOfOtherField((LengthedStructField*)thisStringTable)));
	thisStringTable->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWord(0)));//always 0
	thisStringTable->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWord(1)));//1 for text (0 = binary)
	thisStringTable->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldString(wstring(L"000904b0"))));//concatenated language and codepage 
	thisStringTable->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSField32BitPadding()));


	map<void*, UINT> buffersToAdd;

	int nAdded = 0;
	nAdded += addStringToStructIfNotEmpty(thisStringTable, versionStrings.Comments, L"Comments");



	nAdded += addStringToStructIfNotEmpty(thisStringTable, versionStrings.CompanyName, L"CompanyName");
	nAdded += addStringToStructIfNotEmpty(thisStringTable, versionStrings.FileDescription, L"FileDescription");
	nAdded += addStringToStructIfNotEmpty(thisStringTable, versionStrings.FileVersion, L"FileVersion");
	nAdded += addStringToStructIfNotEmpty(thisStringTable, versionStrings.InternalName, L"InternalName");
	nAdded += addStringToStructIfNotEmpty(thisStringTable, versionStrings.LegalCopyright, L"LegalCopyright");
	nAdded += addStringToStructIfNotEmpty(thisStringTable, versionStrings.LegalTrademarks, L"LegalTrademarks");
	nAdded += addStringToStructIfNotEmpty(thisStringTable, versionStrings.OriginalFilename, L"OriginalFilename");
	nAdded += addStringToStructIfNotEmpty(thisStringTable, versionStrings.ProductName, L"ProductName");
	nAdded += addStringToStructIfNotEmpty(thisStringTable, versionStrings.ProductVersion, L"ProductVersion");



	if (nAdded == 0)return NULL;
	return (LengthedStructField *)thisStringTable;
}
LengthedStructField* getStringStructInfo(const VersionStrings& versionStrings)
{/*
	typedef struct {
		WORD        wLength;
		WORD        wValueLength;
		WORD        wType;
		WCHAR       szKey;
		WORD        Padding;
		StringTable Children;
	} StringFileInfo;*/
	LengthedStruct* thisSSIStruct = (LengthedStruct*)safeAllocator.addNew((LengthedStructField*)new LengthedStruct());
	thisSSIStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWordSizeOfOtherField((LengthedStructField*)thisSSIStruct)));
	thisSSIStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWord(0)));//always zero
	thisSSIStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWord(1)));//1 for text (0 = binary)
	thisSSIStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldString(wstring(L"StringFileInfo"))));
	thisSSIStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSField32BitPadding()));
	thisSSIStruct->addField((LengthedStructField*)getStringTableStruct(versionStrings));
	return (LengthedStructField*)thisSSIStruct;
}

/**
	gets a VarFileInfo structure - tells which languages are supported

	BASE struct:
		typedef struct {
	  WORD  wLength;
	  WORD  wValueLength;
	  WORD  wType;
	  WCHAR szKey;
	  WORD  Padding;
	  Var   Children;  <-----array of Vars
	} VarFileInfo;

	var(chil) structs:
	typedef struct {
	WORD  wLength;
	WORD  wValueLength;
	WORD  wType;
	WCHAR szKey;
	WORD  Padding;
	DWORD Value;
	} Var;


**/
LengthedStructField* getVarFileInfo()
{
	LengthedStruct* thisVFIStruct = (LengthedStruct*)safeAllocator.addNew((LengthedStructField*)new LengthedStruct());
	thisVFIStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWordSizeOfOtherField((LengthedStructField*)thisVFIStruct)));
	thisVFIStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWord(0)));//always zero
	thisVFIStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWord(1)));//1 for text (0 = binary)
	thisVFIStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldString(wstring(L"VarFileInfo"))));
	thisVFIStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSField32BitPadding()));

	//this is a English VAR struct for supported languages (only English for now)
	LengthedStruct* englishVarStruct = (LengthedStruct*)safeAllocator.addNew((LengthedStructField*)new LengthedStruct());
	englishVarStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWordSizeOfOtherField((LengthedStructField*)englishVarStruct)));
	void* varVal = safeAllocator.addMalloc(malloc(sizeof(DWORD)));
	((DWORD*)varVal)[0] = 0x04b00009;
	LengthedStructField* valField = (LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldStaticBuff(varVal, sizeof(DWORD)));
	englishVarStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWordSizeOfOtherField(valField)));
	englishVarStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWord(0)));//1 for text (0 = binary)
	englishVarStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldString(wstring(L"Translation"))));
	englishVarStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSField32BitPadding()));
	englishVarStruct->addField(valField);

	//add it to the end of the VarFileInfo struct
	thisVFIStruct->addField((LengthedStructField*)englishVarStruct);

	return (LengthedStructField*)thisVFIStruct;
}


LengthedStructField * getVsVersionInfoStruct(const BYTE versionBytes[4], const VersionStrings& versionStrings)
{
	/*
	struct {
		WORD             wLength;
		WORD             wValueLength;
		WORD             wType;
		WCHAR            szKey;
		WORD             Padding1;
		VS_FIXEDFILEINFO Value;
		WORD             Padding2;
		WORD             Children;
	}vs_version_infoHeader; */
	LengthedStruct* thisVVIStruct = (LengthedStruct*)safeAllocator.addNew((LengthedStructField*)new LengthedStruct());
	thisVVIStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWordSizeOfOtherField((LengthedStructField*)thisVVIStruct)));

	VS_FIXEDFILEINFO* vsFixedFileInfo = (VS_FIXEDFILEINFO*)safeAllocator.addNew((LengthedStructField*)new VS_FIXEDFILEINFO());
	ZeroMemory(vsFixedFileInfo, sizeof(VS_FIXEDFILEINFO));
	LSFieldStaticBuff*  pLsFFI = (LSFieldStaticBuff*)safeAllocator.addNew((LengthedStructField*)new LSFieldStaticBuff(vsFixedFileInfo, sizeof(VS_FIXEDFILEINFO)));
	thisVVIStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWordSizeOfOtherField((LengthedStructField*)pLsFFI)));//always 0
	thisVVIStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldWord(0)));//1 for text (0 = binary)
	thisVVIStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSFieldString(wstring(L"VS_VERSION_INFO"))));
	thisVVIStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSField32BitPadding()));
	thisVVIStruct->addField((LengthedStructField*)pLsFFI);
	//DWORD   dwSignature;            /* e.g. 0xfeef04bd */
	//DWORD   dwStrucVersion;         /* e.g. 0x00000042 = "0.42" */
	//DWORD   dwFileVersionMS;        /* e.g. 0x00030075 = "3.75" */
	//DWORD   dwFileVersionLS;        /* e.g. 0x00000031 = "0.31" */
	//DWORD   dwProductVersionMS;     /* e.g. 0x00030010 = "3.10" */
	//DWORD   dwProductVersionLS;     /* e.g. 0x00000031 = "0.31" */
	//DWORD   dwFileFlagsMask;        /* = 0x3F for version "0.42" */
	//DWORD   dwFileFlags;            /* e.g. VFF_DEBUG | VFF_PRERELEASE */
	//DWORD   dwFileOS;               /* e.g. VOS_DOS_WINDOWS16 */VOS_NT
	//DWORD   dwFileType;             /* e.g. VFT_DRIVER */
	//DWORD   dwFileSubtype;          /* e.g. VFT2_DRV_KEYBOARD */
	//DWORD   dwFileDateMS;           /* e.g. 0 */
	//DWORD   dwFileDateLS;           /* e.g. 0 */
	vsFixedFileInfo->dwSignature = 0xfeef04bd;
	vsFixedFileInfo->dwStrucVersion = 0x00010000;

	vsFixedFileInfo->dwFileFlagsMask = 0x3fL;
	vsFixedFileInfo->dwFileType = VFT_APP;
	vsFixedFileInfo->dwFileSubtype = VFT_UNKNOWN; \
		vsFixedFileInfo->dwFileVersionLS = (DWORD)versionBytes[0] | (versionBytes[1] << 16);
	vsFixedFileInfo->dwFileVersionMS = (DWORD)versionBytes[2] | (versionBytes[3] << 16);
	vsFixedFileInfo->dwProductVersionLS = (DWORD)versionBytes[0] | (versionBytes[1] << 16);
	vsFixedFileInfo->dwProductVersionMS = (DWORD)versionBytes[2] | (versionBytes[3] << 16);

	vsFixedFileInfo->dwFileOS = VOS_NT;

	thisVVIStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSField32BitPadding()));
	thisVVIStruct->addField((LengthedStructField*)getStringStructInfo(versionStrings));
	thisVVIStruct->addField((LengthedStructField*)safeAllocator.addNew((LengthedStructField*)new LSField32BitPadding()));
	thisVVIStruct->addField((LengthedStructField*)getVarFileInfo());



	return (LengthedStructField*)thisVVIStruct;
}
/**
\param hExtractorExe handle to opend exe by winapi's BeginUpdateResource
\param versionBytes version in 4 byte format found in the PE header
\param versionStrings all the meta data strings company name, comments, copyrights...
\return true if succeeded 
**/
bool pemetalib::UpdatePEStrings(HANDLE hExtractorExe, const BYTE versionBytes[4], const VersionStrings& versionStrings)
{
	LOG_DEBUG(L"Updating PE Strings");

	ostringstream oss;
	LengthedStructField* vsVersionInfo = getVsVersionInfoStruct(versionBytes, versionStrings);
	vsVersionInfo->writeToStream(oss);
	size_t sizeWritten = oss.tellp();
	void* toWrite = safeAllocator.addMalloc(malloc(sizeWritten));
	memcpy(toWrite, oss.str().c_str(), sizeWritten);

	//update the version and all the other PE meta data strings 
	if (!UpdateResource(hExtractorExe, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO),
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), toWrite, sizeWritten))
	{
		LOG_ERROR(L"Could not update extractor resources - %s", GetLastErrorStdStrW().c_str());
		return false;
	}
	return true;
}

