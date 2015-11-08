#include "../include/peml_internal.h"

using namespace pemetalib;



LogCallback logCallback;
bool pemetalib::UpdateExeResources( const ExeResources & resources,const wstring & exePath)
{
	LOG_DEBUG(L"Updating resources");
	//start editing the written extractor PE header
	HANDLE hExtractorExe = BeginUpdateResourceW(exePath.c_str(), TRUE);
	if (!hExtractorExe)
	{
		LOG_ERROR(L"BeginUpdateResource failed - %s", GetLastErrorStdStrW().c_str());
		return false;
	}
	if (resources.manifest!=L"")
	{
		string narrow(resources.manifest.begin(), resources.manifest.end());//todo: support wide chars
		UpdatePEManifest(hExtractorExe, narrow);
	}
	if (resources.iconPath != L"")
	{
		UpdatePEIcon(hExtractorExe, wstring(resources.iconPath));
	}
	UpdatePEStrings(hExtractorExe, resources.versionBytes, resources.versionstrings);
	EndUpdateResource(hExtractorExe, FALSE);
	return true;
}

void pemetalib::SetLogCallback(LogCallback&  logcb)
{
	logCallback = logcb;
}


void pemetalib::Log(bool onlyVerbose, const wchar_t* format, ...)
{
	//if (!loggerOn || !outLog)return;
	//if (onlyVerbose &!verbose)return;
	static wchar_t buffer[2048];
	va_list argptr;
	va_start(argptr, format);
	vswprintf(buffer, 2048, format, argptr);
	va_end(argptr);
	wstring msg(buffer);
	(*logCallback)(msg, onlyVerbose ? 0 : 1,0);
	//(*outLog) << buffer << L"\n";
	//outLog->flush();
};

class SafeAllocatorImpl : public SafeAllocator
{
private:
	vector<void*> malloced;
	vector<LengthedStructField *> newed;
	void deleteLsFields();
	//vector<void*> arrayNewed;
public:
	virtual void* addMalloc(void* o) { malloced.push_back(o); return o; };
	virtual void* addNew(LengthedStructField* o) { newed.push_back(o); return o; };
	//void* addNewArray(void* o){arrayNewed.push_back(o); return o;};
	virtual ~SafeAllocatorImpl()
	{
		for (vector<void*>::iterator it = malloced.begin(); it != malloced.end(); ++it)
			free(*it);
		deleteLsFields();
		//for (vector<void*>::iterator it = arrayNewed.begin() ; it != arrayNewed.end(); ++it)
		//	delete [] (*it);
	}

};

void SafeAllocatorImpl::deleteLsFields()
{
	for (vector<LengthedStructField*>::iterator it = newed.begin(); it != newed.end(); ++it)
		delete (*it);
}


wstring  pemetalib::GetLastErrorStdStrW()
{
	DWORD error = GetLastError();
	wchar_t errNumBuff[20] = { 0 };
	_itow_s(error, errNumBuff, 20, 10);
	if (error)
	{
		LPVOID lpMsgBuf;
		DWORD bufLen = FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);
		if (bufLen)
		{
			LPCWSTR lpMsgStr = (LPCWSTR)lpMsgBuf;
			wstring result(lpMsgStr, lpMsgStr + bufLen);

			LocalFree(lpMsgBuf);
			result = wstring(errNumBuff) + L": " + result;
			return result;
		}
	}
	return wstring();
}

bool pemetalib::UpdatePEManifest(HANDLE hExtractorExe, string& manifest)
{
	LOG_DEBUG(L"Adding manifest");
	if (!UpdateResource(hExtractorExe, RT_MANIFEST, MAKEINTRESOURCE(VS_VERSION_INFO),
		MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (void*)manifest.c_str(), manifest.length() - 1))
	{
		LOG_ERROR(L"Could not update manifest - %s", GetLastErrorStdStrW().c_str());
		return false;
	}
	return true;

}
int pemetalib::LSFieldStaticBuff::writeToStream(ostream & stream)
{
	stream.write((char*)buff, length);
	return length;
}

int pemetalib::LSField32BitPadding::writeToStream(ostream & stream)
{
	size_t nZeros = getSizeInBytes(stream.tellp());
	if (nZeros)for (int i = 0; i < nZeros; i++)stream.write("", 1);//write nZeros zeros to buffer(use the null terminate of "")
	return nZeros;
};
int pemetalib::LSField32BitPadding::getSizeInBytes(int sizeSoFar)
{
	//bytes needed to make the size divided by 4 [0-3]
	return (4 - (sizeSoFar) % 4) % 4;//%8;
}

int pemetalib::LSFieldWordSizeOfOtherField::writeToStream(ostream & stream)
{
	WORD sizeOfOther = other->getSizeInBytes(0);
	if (inWords)sizeOfOther /= 2;
	stream.write((char*)&sizeOfOther, sizeof(WORD));
	return sizeof(WORD);
}
int pemetalib::LSFieldWordSizeOfOtherField::getSizeInBytes(int sizeSoFar)
{
	return sizeof(WORD);
}


int pemetalib::LengthedStruct::getSizeInBytes(int sizeSoFar)
{
	UINT sum = 0;
	for (vector<LengthedStructField*>::iterator it = fields.begin(); it != fields.end(); ++it)
	{
		sum += (*it)->getSizeInBytes(sizeSoFar + sum);
	}
	return sum;
}

int pemetalib::LengthedStruct::writeToStream(ostream & stream)
{
	UINT sum = 0;
	for (vector<LengthedStructField*>::iterator it = fields.begin(); it != fields.end(); ++it)
	{
		sum += (*it)->writeToStream(stream);
	}
	return sum;
}

SafeAllocatorImpl safeAllocatorImpl;
pemetalib::SafeAllocator &  pemetalib::safeAllocator = safeAllocatorImpl;