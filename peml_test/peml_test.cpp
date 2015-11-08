#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.h"
#include "peml_internal.h"
#include "../peml_utils/peml_utils.h"
#include "boost/filesystem.hpp"
using namespace pemetalib;
using namespace peutils;

std::string ReplaceAllStr(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

TEST_CASE("LengthedStruct size", "[LengthedStruct]") {
	LengthedStruct ls;
	LSFieldWord lsfw(45);
	LSField32BitPadding lsp;
	ls.addField((LengthedStructField*)&lsfw);
	ls.addField((LengthedStructField*)&lsp);
	REQUIRE(ls.getSizeInBytes(0) == 4);
}
TEST_CASE("parse command arguments", "[utils]")
{
	pemetalib::ExeResources res;
	peutils::Utils utils;

	HMODULE hModule = GetModuleHandleW(NULL);
	WCHAR path[MAX_PATH];//will hold a path we know exists
	GetModuleFileNameW(hModule, path, MAX_PATH);
	wstring exePath;
	wstring iconPath;
	wchar_t* commandLine[] = { L"prog", L"--exe", path, L"--icon", path ,
		L"--version",L"1.2.4.56",
		L"--comment",L"some comment",
		L"--desc",L"some desc",
		L"--copyright",L"copy right?",
		L"--trademarks",L"a tm",
		L"--internalname",L"bob",
		L"--originalname",L"davis&*!",
		L"--company",L"acme",
		L"--productname",L"best prod"
	};
	int nArgs = sizeof(commandLine) / sizeof(commandLine[0]);
	bool ret = utils.parseCommandLineArgs(nArgs, commandLine, res, 
	vector<AdditionalArgument>{AdditionalArgument("exe","",&exePath,AdditionalArgument::ArgType::EXISITNG_FILE,true)});
	REQUIRE(ret == true);
	REQUIRE(((DWORD*)&res.versionBytes)[0]== 0x01020438);
	REQUIRE(res.versionstrings.FileVersion == L"1.2.4.56");
	REQUIRE(res.versionstrings.ProductVersion == L"1.2.4.56");
	REQUIRE(res.versionstrings.CompanyName == L"acme");
	REQUIRE(res.versionstrings.Comments == L"some comment");
	REQUIRE(res.versionstrings.FileDescription == L"some desc");
	REQUIRE(res.versionstrings.LegalCopyright == L"copy right?");
	REQUIRE(res.versionstrings.LegalTrademarks == L"a tm");
	REQUIRE(res.versionstrings.InternalName == L"bob");
	REQUIRE(res.versionstrings.OriginalFilename == L"davis&*!");
	REQUIRE(res.versionstrings.ProductName == L"best prod");
	
	wchar_t* commandLine2[] = { L"prog", L"--exe", path, L"--icon", path ,
		L"--version",L"1.2.4.56",
	};
	pemetalib::ExeResources res2;
	int nArgs2 = sizeof(commandLine2) / sizeof(commandLine2[0]);
	ret = utils.parseCommandLineArgs(nArgs2, commandLine2, res2,
		vector<AdditionalArgument>{AdditionalArgument("exe", "", &exePath, AdditionalArgument::ArgType::EXISITNG_FILE, true)});
	REQUIRE(ret == true);
	REQUIRE(res2.versionstrings.LegalCopyright == L"");
	REQUIRE(res2.versionstrings.LegalTrademarks == L"");
	REQUIRE(res2.versionstrings.Comments == L"");
	REQUIRE(res2.versionstrings.InternalName == L"");
	REQUIRE(res2.versionstrings.OriginalFilename == L"");
	REQUIRE(res2.versionstrings.ProductName == L"");

	wchar_t* commandLine3[] = { L"prog", L"--exe", path, L"--ico", path ,
		L"--help",
		L"--version",L"1.2.456",
		L"--comment",L"some comment",
		L"--desc",L"some desc",
		L"--copyright",L"copy right?",
		L"--trademarks",L"a tm",
		L"--internalname",L"bob",
		L"--originalname",L"davis&*!",
		L"--productname",L"best prod"
		
	};
	pemetalib::ExeResources res3;
	int nArgs3 = sizeof(commandLine3) / sizeof(commandLine3[0]);
	ret = utils.parseCommandLineArgs(nArgs3, commandLine3, res3,
		vector<AdditionalArgument>{AdditionalArgument("exe", "", &exePath, AdditionalArgument::ArgType::EXISITNG_FILE, true)});
	REQUIRE(ret == false);
	wstring err = utils.getParseError();
	REQUIRE(err.find(L"Usage")!=err.npos);

}



TEST_CASE("end to end", "[e2e]")
{
	//more of a smoke test
	HMODULE hModule = GetModuleHandleW(NULL);
	CHAR path[MAX_PATH];//will hold a path to exe we know exists -this module
	GetModuleFileNameA(hModule, path, MAX_PATH);


	
	const char * PATH_TO_CONSOLE = "C:\\ws\\DuckPack\\vs2015\\Debug\\peml_console.exe";
	const char * PATH_TO_TESTOUT = "..\\peml_test\\test_workspace\\teste2e.exe";
	const char * PATH_TO_ICON = "..\\peml_test\\test_workspace\\icon.ico";
	const char * PATH_TO_MANIFEST = "..\\peml_test\\test_workspace\\manifest.txt";

	boost::filesystem::copy_file(path, PATH_TO_TESTOUT, boost::filesystem::copy_option::overwrite_if_exists);


	const char* commandLine[] = { PATH_TO_CONSOLE, 
		"--exe", PATH_TO_TESTOUT,
		"--ico", PATH_TO_ICON,
		"--manifest", PATH_TO_MANIFEST,
		"--version","1.2.4.56",
		"--comment","some comment",
		"--desc","some desc",
		"--copyright","copy right",
		"--trademarks","a tm",
		"--internalname","bob",
		"--originalname","davis",
		"--company","ACME LTD",
		"--productname","best prod"
	};
	int nArgs = sizeof(commandLine) / sizeof(commandLine[0]);
	string command(commandLine[0]);
	for (int i = 1; i < nArgs; i++)
	{
		command += " \"";
		command += commandLine[i];
		command += "\"";
	}
	

	std::system(command.c_str());
	char buffer[1024];
	GetFullPathNameA(PATH_TO_TESTOUT, 1024, buffer, NULL);
	string duobleBackslash = ReplaceAllStr(string(buffer), "\\", "\\\\");
	command = "wmic datafile where Name='" + duobleBackslash + "'";
	FILE *lsofFile_p = _popen(command.c_str(), "r");
	REQUIRE(lsofFile_p != NULL);
	


	char *line_p = fgets(buffer, sizeof(buffer), lsofFile_p);
	line_p = fgets(buffer, sizeof(buffer), lsofFile_p);
	_pclose(lsofFile_p);

	string  strMetaData(buffer);

	REQUIRE((int)strMetaData.find("ACME LTD") != (int)strMetaData.npos);
	REQUIRE((int)strMetaData.find("1.2.4.56") != (int)strMetaData.npos);



}