#pragma once
#ifndef PEML_PE_DS_H
#define PEML_PE_DS_H

#include <sstream>
/**
Data structers used by windows PE format.
Their complexity: datastructers need to "know" the size of themselves
(size in bytes of the datastructure and children)
**/

using namespace std;
namespace pemetalib
{
	/**
	Base class for all PE structs
	**/
	class LengthedStructField
	{
	public:
		virtual int writeToStream(ostream & stream) = 0; ///write struct including children to stream
		virtual int getSizeInBytes(int sizeSoFar) = 0; ///size in bytes of struct including children
	};

	/**
	PE struct for one WORD
	**/
	class LSFieldWord :LengthedStructField
	{
	private:
		WORD word;
	public:
		LSFieldWord(WORD w) :word(w) {};
		virtual int writeToStream(ostream & stream) { stream.write((char*)&word, getSizeInBytes(0)); return getSizeInBytes(0); };
		virtual int getSizeInBytes(int sizeSoFar) { return 2; };
	};

	/**
	PE struct for a string (could be null terminated or not)
	**/
	class LSFieldString :LengthedStructField
	{
	private:
		wstring string;
		bool nullTerminated;
	public:
		LSFieldString(const wstring& s, bool nullTerm = true) :string(s), nullTerminated(nullTerm) {};
		virtual int writeToStream(ostream & stream) { stream.write((char*)(string.c_str()), getSizeInBytes(0)); return getSizeInBytes(0); };
		virtual int getSizeInBytes(int sizeSoFar) { return sizeof(wchar_t)*(string.length() + (nullTerminated ? 1 : 0)); };
	};

	/**
	PE struct for an arbitrary buffer
	**/
	class LSFieldStaticBuff :LengthedStructField
	{
	private:
		void* buff;
		int length;
	public:
		LSFieldStaticBuff(void* b, int len) :buff(b), length(len) {};
		virtual int writeToStream(ostream & stream);
		virtual int getSizeInBytes(int sizeSoFar) { return length; };
	};

	/**
	PE struct for padding the output buffer to be at pos mod 4 = 0 for next field
	**/
	class LSField32BitPadding :LengthedStructField
	{
	public:
		LSField32BitPadding() {};
		virtual int writeToStream(ostream & stream);
		virtual int getSizeInBytes(int sizeSoFar);
	};

	class LSFieldWordSizeOfOtherField :LengthedStructField
	{
	private:
		LengthedStructField * other;
		bool inWords;
	public:
		LSFieldWordSizeOfOtherField(LengthedStructField * f, bool isInWords = false) :other(f), inWords(isInWords) {};
		virtual int writeToStream(ostream & stream);
		virtual int getSizeInBytes(int sizeSoFar);
	};

	class LengthedStruct :LengthedStructField
	{
	private:
		vector<LengthedStructField*> fields;
	public:
		LengthedStruct() {};
		void addField(LengthedStructField* field) { fields.push_back(field); }
		virtual int writeToStream(ostream & stream);
		virtual int getSizeInBytes(int sizeSoFar);
	};
}

#endif 