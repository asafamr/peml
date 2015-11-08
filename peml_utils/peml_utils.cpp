
#include "peml_utils.h"
#include <fstream>
#include <sstream>
#include <vector>
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"
using namespace std;
namespace po = boost::program_options;



namespace peutils
{

	
	struct OutArgStrs;
	class UtilsImpl
	{
	protected:


		wstring lastError;
	public:
		po::options_description getArgDesc(pemetalib::ExeResources & outStrs, vector<AdditionalArgument> &additional);
		bool parseVersion(wstring & ver, unsigned char out[4]);
		wstring getParseError() { return lastError; }
		bool parseCommandLineArgs(int argc, wchar_t ** argv, pemetalib::ExeResources &outRes, vector<AdditionalArgument> &additional);
		wstring getFileContent(const wstring &filePath);
		vector<wstring> &UtilsImpl::split(const wstring &s, wchar_t delim, vector<wstring> &elems);
		vector<wstring> splitStr(const wstring &s, wchar_t delim);
		bool isFileExisting(wstring & path);
		bool isDirExisting(wstring & path);
		wstring narrowToWide(string & str);

	};
	wstring Utils::narrowToWide(string & str)
	{
		return utilsImpl->narrowToWide(str);
	}
	wstring Utils::getParseError()
	{
		return utilsImpl->getParseError();
	}
	bool Utils::parseCommandLineArgs(int argc, wchar_t** argv, pemetalib::ExeResources &outRes, vector<AdditionalArgument> &additional)
	{
		return utilsImpl->parseCommandLineArgs(argc, argv, outRes,additional);
	}

	bool Utils::isFileExisting(wstring & path)
	{
		return utilsImpl->isFileExisting(path);
	}
	bool Utils::isDirExisting(wstring & path)
	{
		return utilsImpl->isDirExisting(path);
	}
	Utils::Utils()
	{
		utilsImpl = new UtilsImpl();
	}
	Utils::~Utils()
	{
		delete utilsImpl;
	}
	wstring Utils::getFileContent(const wstring &filePath)
	{
		return utilsImpl->getFileContent(filePath);
	}

	vector<wstring> Utils::splitStr(const wstring & s, wchar_t delim)
	{
		//some copying but nothing to worry 'bout
		return utilsImpl->splitStr(s, delim);
	}







	wstring UtilsImpl::getFileContent(const wstring &filePath)
	{
		ifstream ifs(filePath.c_str(), ios::in | ios::binary | ios::ate);

		ifstream::pos_type fileSize = ifs.tellg();
		ifs.seekg(0, ios::beg);

		vector<char> bytes(fileSize);
		ifs.read(&bytes[0], fileSize);
		string s(&bytes[0], fileSize);
		return narrowToWide(s);
	}
	struct OutArgStrs
	{
		OutArgStrs() :isHelp(false) {};
		wstring iconPath;
		wstring manifestPath;
		wstring exePath;
		wstring version;
		wstring Comments;
		wstring FileDescription;
		wstring LegalCopyright;
		wstring LegalTrademarks;
		wstring InternalName;
		wstring OriginalFilename;
		wstring ProductName;

		bool isHelp;

	};
	/**
	This boost program options description will fill the ExeResources struct., 
	leaving not set fields as peutils::NOT_SET_TOKEN
	some crude fixes notes:
	version only goes to fileVersion - we deal with this later on
	res.manifest gets the path now and later get its content
	we check for change by comparing strings to peutils::NOT_SET_TOKEN

	all of these can be fixed with notifier functions but let it be like this now
	**/
	po::options_description UtilsImpl::getArgDesc(pemetalib::ExeResources & res,vector<AdditionalArgument> &additional)
	{
		po::options_description desc("Program Usage", 1024, 512);
		po::options_description_easy_init init = desc.add_options()
			("help", "produce help message");

		for (int i = 0; i < additional.size(); i++)
		{
			po::value_semantic * vs;
			if (additional[i].mtype == AdditionalArgument::ArgType::BOOL)
			{
				vs = po::bool_switch((bool*)&additional[i].mToSet);
			}
			else
			{
				((wstring*)additional[i].mToSet)->operator=(L"");
				vs = po::wvalue<wstring>((wstring*)additional[i].mToSet);
			}
			if (additional[i].mIsRequired)vs->is_required();
			init = init(additional[i].mArg, vs, additional[i].mDescription);
		}
		po::options_description strDesc("Meta Strings");
		strDesc.add_options()
			("version,v", po::wvalue<wstring>(&res.versionstrings.FileVersion)->required(), "version in <BYTE>.<BYTE>.<BYTE>.<BYTE> form e.g. 1.23.45.67 ")
			("comments", po::wvalue<wstring>(&res.versionstrings.Comments), "Comments")
			("company", po::wvalue<wstring>(&res.versionstrings.CompanyName), "Company name")
			("description", po::wvalue<wstring>(&res.versionstrings.FileDescription), "File description")
			("copyright", po::wvalue<wstring>(&res.versionstrings.LegalCopyright), "Legal copyright")
			("trademarks", po::wvalue<wstring>(&res.versionstrings.LegalTrademarks), "Legal trademarks")
			("internalname", po::wvalue<wstring>(&res.versionstrings.InternalName), "Internal name")
			("originalname", po::wvalue<wstring>(&res.versionstrings.OriginalFilename), "Original file name")
			("productname", po::wvalue<wstring>(&res.versionstrings.ProductName), "Product name");

		po::options_description all("Allowed options");
		all.add(desc).add(strDesc);
		return all;
	}


	bool UtilsImpl::parseVersion(wstring & ver, unsigned char out[4])
	{
		//version should be <BYTE>.<BYTE>.<BYTE>.<BYTE> e.g. 1.9.1234
		if (ver.find_first_not_of(L"1234567890.") != string::npos)
		{
			return false;//contains illigal chars - fail
		}
		if (count(ver.begin(), ver.end(), '.') != 3)
		{
			return false;//contains wrong number of dots
		}
		vector<wstring> tokens = splitStr(ver, L'.');
		long t1 = stol(tokens[0]);
		long t2 = stol(tokens[1]);
		long t3 = stol(tokens[2]);
		long t4 = stol(tokens[3]);
		if (t1 < 0 || t1> 255 ||
			t2 < 0 || t2> 255 ||
			t3 < 0 || t3> 255 ||
			t4 < 0 || t4> 255)
		{
			return false;
		}
		out[3] = (BYTE)t1;
		out[2] = (BYTE)t2;
		out[1] = (BYTE)t3;
		out[0] = (BYTE)t4;
		return true;
	}


	vector<wstring> &UtilsImpl::split(const wstring &s, wchar_t delim, vector<wstring> &elems) {
		wstringstream ss(s);
		wstring item;
		while (getline(ss, item, delim)) {
			elems.push_back(item);
		}
		return elems;
	}


	vector<wstring> UtilsImpl::splitStr(const wstring &s, wchar_t delim) {
		vector<wstring> elems;
		split(s, delim, elems);
		return elems;
	}

	bool UtilsImpl::isFileExisting(wstring & path)
	{
		return boost::filesystem::exists(path.c_str()) && boost::filesystem::is_regular_file(path.c_str());
	}
	bool UtilsImpl::isDirExisting(wstring & path)
	{
		return boost::filesystem::exists(path.c_str()) && boost::filesystem::is_directory(path.c_str());
	}

	wstring UtilsImpl::narrowToWide(string & path)
	{
		return wstring(path.begin(),path.end());//stupidly convert to wide chars TODO:unstupidify - use locale to back and forth convert
	}

	bool UtilsImpl::parseCommandLineArgs(int argc, wchar_t** argv, pemetalib::ExeResources &outRes, vector<AdditionalArgument> &additional)
	{
		

		try
		{
			vector<AdditionalArgument> additionalFull(additional);
			additionalFull.push_back(AdditionalArgument("icon", "", &outRes.iconPath, AdditionalArgument::ArgType::EXISITNG_FILE, false));
			additionalFull.push_back(AdditionalArgument("manifest", "", &outRes.iconPath, AdditionalArgument::ArgType::EXISTING_FILE_LOAD_CONTENT, false));
			//check notes on getArgDesc for crudeness 
			po::options_description desc=getArgDesc(outRes, additionalFull);
			po::variables_map vm;
			po::store(po::parse_command_line(argc, argv, desc), vm);

			if (vm.count("help"))//we do this before notify to avoid any "arg required" in case help was supplied
			{
				stringstream ss;
				ss << desc;
				lastError = narrowToWide(ss.str());
				return false;
			}

			po::notify(vm);//fills outRes and check for required fields

			for (int i = 0; i < additionalFull.size(); i++)
			{
				AdditionalArgument thisArg = additionalFull[i];
				if (thisArg.mtype == AdditionalArgument::ArgType::BOOL)
				{
					continue;
				}
				if (*(wstring*)thisArg.mToSet != L"")
				{
					if (thisArg.mtype == AdditionalArgument::ArgType::EXISITNG_FILE ||
						thisArg.mtype == AdditionalArgument::ArgType::EXISTING_FILE_LOAD_CONTENT)
					{
						if (!isFileExisting(*(wstring*)thisArg.mToSet))
						{
							//file must exist and doesn't
							lastError = L"ERROR: file does not exist. argument " + narrowToWide(string(additional[i].mArg));
							return false;
						}
					}
					if (thisArg.mtype == AdditionalArgument::ArgType::EXISTING_FILE_LOAD_CONTENT)
					{
						(*(wstring*)thisArg.mToSet) = getFileContent(*(wstring*)thisArg.mToSet);
					}
					if (thisArg.mtype == AdditionalArgument::ArgType::EXISTING_DIR)
					{
						if (!isDirExisting(*(wstring*)thisArg.mToSet))
						{
							// dir must exist and doesn't
							lastError = L"ERROR: dir does not exist. argument " + narrowToWide(string(thisArg.mArg));
							return false;
						}

					}
				}
			}

			
			if (!parseVersion(outRes.versionstrings.FileVersion, outRes.versionBytes))
			{
				lastError = L"Version argument invalid. Should be in <BYTE>.<BYTE>.<BYTE>.<BYTE> format. e.g. 1.32.73.68";
				return false;
			}
			outRes.versionstrings.ProductVersion = outRes.versionstrings.FileVersion;
			

		}
		catch (exception& e)
		{
			lastError = L"Error: ";
			lastError += narrowToWide(string(e.what()));
			return false;
		}
		catch (...)
		{
			lastError = L"Unknown error" ;
			return false;
		}

		return true;

	}

}