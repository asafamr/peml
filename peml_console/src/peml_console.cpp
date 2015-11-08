// WinResLibConsole.cpp : Defines the entry point for the console application.
//


#include "../../peml/include/peml.h"
#include "../../peml_utils/peml_utils.h"
#include <string>
#include <iostream>
#include <vector>

#include "boost/filesystem.hpp"


using namespace std;
using namespace pemetalib;
using namespace peutils;









void log(wstring& msg, int level, int code)
{
	wcout << L"Log:" << level <<L" " << code << L" " << msg.c_str() << endl;
}
int wmain(int argc, wchar_t**argv)
{
	pemetalib::ExeResources res;
	peutils::Utils utils;
	wstring exePath;
	wstring manifest;
	wstring iconPath;

	bool ret = utils.parseCommandLineArgs(argc, argv, res,
		vector<AdditionalArgument>({
		AdditionalArgument("exe", "Exe file to update(required)", &exePath, AdditionalArgument::ArgType::EXISITNG_FILE, true)}));
	if (ret == false)
	{
		wcout << utils.getParseError()<<endl;
		return -1;
	}
	LogCallback logcb;
	logcb = &log;
	pemetalib::SetLogCallback(logcb);
	pemetalib::UpdateExeResources(res, exePath
		);

	
	
}

