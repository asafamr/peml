#pragma once
#ifndef PEML_UTILS_H
#define PEML_UTILS_H

#include "../peml/include/peml.h"
#include <string>
#include <vector>

using namespace std;


namespace peutils
{
	class AdditionalArgument
	{
	public:
		enum ArgType{EXISITNG_FILE, EXISTING_FILE_LOAD_CONTENT, FILE,EXISTING_DIR,BOOL};
		AdditionalArgument(char *arg, char *description, void *toSet, ArgType type,bool isRequired) :
			mArg(arg),mDescription(description), mToSet(toSet), mtype(type),mIsRequired(isRequired){};
		char * mArg;//"exe,e" for example for --exe -e
		char * mDescription;//"exe,e" for example for --exe -e
		void *mToSet;
		ArgType mtype;
		bool mIsRequired;
	};
	class UtilsImpl;
	class Utils
	{

	public:
		wstring getParseError();
		bool parseCommandLineArgs(int argc, wchar_t** argv, pemetalib::ExeResources &outRes,vector<AdditionalArgument> &additional);
		wstring getFileContent(const wstring &filePath);
		vector<wstring> splitStr(const wstring &s, wchar_t delim);
		bool isFileExisting(wstring & path);
		bool isDirExisting(wstring & path);
		wstring narrowToWide(string & str);
		Utils();
		~Utils();
	protected:
		UtilsImpl *utilsImpl;
	};
}

#endif