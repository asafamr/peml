//this file code takes care of icon changing in windows
#include "../include/peml_internal.h"

using namespace pemetalib;
//the structure found in ICO files
#pragma pack(push, 2)
typedef struct
{
	BYTE        bWidth;          // Width, in pixels, of the image
	BYTE        bHeight;         // Height, in pixels, of the image
	BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
	BYTE        bReserved;       // Reserved ( must be 0)
	WORD        wPlanes;         // Color Planes
	WORD        wBitCount;       // Bits per pixel
	DWORD       dwBytesInRes;    // How many bytes in this resource?
	DWORD       dwImageOffset;   // Where in the file is this image? - in resources this is resource ID instead
} ICONDIRENTRY, *LPICONDIRENTRY;
#pragma pack(pop)

#pragma pack(push, 2)
typedef struct {
	WORD           idReserved;   // Reserved (must be 0)
	WORD           idType;       // Resource Type (1 for icons)
	WORD           idCount;      // How many images?
	ICONDIRENTRY   idEntries[1]; // An entry for each image (idCount of 'em)
}ICONDIR, *LPICONDIR;
#pragma pack(pop)

//the structure used in resource icon group (RT_GROUP_ICON)
#pragma pack(push, 2)
typedef struct {
	BYTE        bWidth;          // Width, in pixels, of the image
	BYTE        bHeight;         // Height, in pixels, of the image
	BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
	BYTE        bReserved;       // Reserved ( must be 0)
	WORD        wPlanes;         // Color Planes
	WORD        wBitCount;       // Bits per pixel
	DWORD       dwBytesInRes;    // How many bytes in this resource?
	WORD		ResourceID;      // resource ID
} GROUPICON;
#pragma pack(pop)


bool pemetalib::UpdatePEIcon(HANDLE hExtractorExe, wstring & iconPath )
{
	HANDLE	hFile = CreateFileW (iconPath.c_str(),
		GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		LOG_ERROR(L"Could not open icon file - %s ",GetLastErrorStdStrW().c_str());
		return false;
	}

	LPICONDIR pIconDir;
	int baseResId=200;
	DWORD dwBytesRead =0;

	// We need an ICONDIR to hold the data
	pIconDir = (LPICONDIR)malloc( sizeof( ICONDIR ) );//not added to sa because of realloc later on
	ReadFile( hFile, &(pIconDir->idReserved), sizeof( WORD ), &dwBytesRead, NULL );
	ReadFile( hFile, &(pIconDir->idType), sizeof( WORD ), &dwBytesRead, NULL );
	if(pIconDir->idType!= 1 || pIconDir->idReserved !=0)
	{
		LOG_ERROR(L"Could not load icon file");
		return false;
	}
	ReadFile( hFile, &(pIconDir->idCount), sizeof( WORD ), &dwBytesRead, NULL );
	pIconDir = (LPICONDIR)safeAllocator.addMalloc(realloc( pIconDir, ( sizeof( WORD ) * 3 ) +
		( sizeof( ICONDIRENTRY ) * pIconDir->idCount ) ));

	ReadFile( hFile, pIconDir->idEntries, pIconDir->idCount * sizeof(ICONDIRENTRY),
		&dwBytesRead, NULL );
	char** pIconImages= (char**)safeAllocator.addMalloc(malloc(  sizeof(char*)*pIconDir->idCount ));

	// Loop through and read in each image
	for(int i=0;i<pIconDir->idCount;i++)
	{
		pIconImages[i] = (char*)safeAllocator.addMalloc(malloc( pIconDir->idEntries[i].dwBytesInRes ));
		SetFilePointer( hFile, pIconDir->idEntries[i].dwImageOffset, 
			NULL, FILE_BEGIN );
		ReadFile( hFile, pIconImages[i], pIconDir->idEntries[i].dwBytesInRes,
			&dwBytesRead, NULL );
	}

	//done with the .ico file - all in memory now
	CloseHandle(hFile);

	int resIconDirSize=( sizeof( WORD ) * 3 ) +
		( sizeof( GROUPICON ) * pIconDir->idCount );
	LPICONDIR pResourceIconDir = (LPICONDIR)safeAllocator.addMalloc(malloc( resIconDirSize ));
	{
		pResourceIconDir->idCount=pIconDir->idCount;
		pResourceIconDir->idReserved=pIconDir->idReserved;
		pResourceIconDir->idType=pIconDir->idType;
	}
	for(int i=0;i<pIconDir->idCount;i++)
	{
		((GROUPICON*)pResourceIconDir->idEntries)[i].bHeight=pIconDir->idEntries[i].bHeight;
		((GROUPICON*)pResourceIconDir->idEntries)[i].bWidth=pIconDir->idEntries[i].bWidth;
		((GROUPICON*)pResourceIconDir->idEntries)[i].bColorCount=pIconDir->idEntries[i].bColorCount;
		((GROUPICON*)pResourceIconDir->idEntries)[i].bReserved=pIconDir->idEntries[i].bReserved;
		((GROUPICON*)pResourceIconDir->idEntries)[i].wPlanes=pIconDir->idEntries[i].wPlanes;
		((GROUPICON*)pResourceIconDir->idEntries)[i].wBitCount=pIconDir->idEntries[i].wBitCount;
		((GROUPICON*)pResourceIconDir->idEntries)[i].dwBytesInRes=pIconDir->idEntries[i].dwBytesInRes;
		((GROUPICON*)pResourceIconDir->idEntries)[i].ResourceID=baseResId+i+1;
	}

	//create a resource for each of the bitmaps
	for(int i=0;i<pResourceIconDir->idCount;i++)
	{
		UpdateResource(hExtractorExe,RT_ICON,MAKEINTRESOURCE(baseResId+i+1),
			MAKELANGID(LANG_ENGLISH,SUBLANG_DEFAULT),pIconImages[i], 
			((GROUPICON*)pResourceIconDir->idEntries)[i].dwBytesInRes);
	}

	//create a resource that groups all the bitmaps to one icon
	UpdateResource(hExtractorExe,RT_GROUP_ICON,L"MAINICON",MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
		pResourceIconDir,resIconDirSize);

	return true;

}