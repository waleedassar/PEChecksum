// PEChecksum.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stddef.h"
#include "stdio.h"
#include "windows.h"
#include "Header.h"



ulong  ChkSum(ulong LastChecksum, unsigned char* ImageBase, ulong NumberOfWords)
{
	if(NumberOfWords)
	{
		do
		{
			ulong X = *(ushort*)ImageBase;
			ImageBase+=2;

			LastChecksum += X;

			ulong LastChecksumX = LastChecksum;

			LastChecksum = (ushort)(LastChecksum);

			LastChecksumX >>= 0x10;

			LastChecksum += LastChecksumX;

			NumberOfWords--;
		}while(NumberOfWords);
	}

	ulong ret = LastChecksum;
	ret >>= 0x10;
	ret += LastChecksum;

	return ret;
}


int MapDllX(wchar_t* pFileName,HANDLE* phFile,HANDLE* phSection,void** pBaseAddress,ulonglong* pViewSize)
{
	if(!pFileName||!phFile||!phSection||!pBaseAddress||!pViewSize) return -1;

	HANDLE hFile = 0;

	_UNICODE_STRING Uni={0};
	Uni.Buffer = pFileName;
	Uni.Length = wcslen(pFileName) * 2;
	Uni.MaxLength = Uni.Length + 2;


	_OBJECT_ATTRIBUTES ObjAttr = {sizeof(ObjAttr)};
	ObjAttr.ObjectName = & Uni;
	ObjAttr.Attributes = OBJ_CASE_INSENSITIVE;

	_IO_STATUS_BLOCK IOB={0};

	int ret = ZwOpenFile(&hFile,
		0x100020,
		&ObjAttr,
		&IOB,
		1,
		0);

	printf("ZwOpenFile, ret: %X, hFile: %I64X\r\n",ret,hFile);

	if(ret < 0)
	{
		if(hFile && hFile!=INVALID_HANDLE_VALUE) ZwClose(hFile);
		return ret;
	}

	*phFile = hFile;


	_OBJECT_ATTRIBUTES ObjAttrSec = {0};
	ObjAttrSec.Length = sizeof(ObjAttrSec);
	ObjAttrSec.Attributes = 0x240;

	HANDLE hSection = 0;
	ret = ZwCreateSection(&hSection,
		0x8 /*DesiredAccess*/,
		&ObjAttrSec /*ObjAttr*/,
		0 /* MaximumSize*/,
		0x10 /*PAGE_EXECUTE*/,
		0x8000000 /*section Never Relocate*/, 
		hFile);

	printf("ZwCreateSection, ret: %X, hSection: %I64X\r\n",ret,hSection);
	

	if(ret < 0)
	{
		ZwClose(hFile);
		return ret;
	}

	*phSection = hSection;


	void* BaseAddress = 0;
	ulonglong ViewSize = 0;
	ret = ZwMapViewOfSection(hSection,
		GetCurrentProcess(),
		&BaseAddress,
		0,
		0,
		0,
		&ViewSize,
		ViewShare,
		0,
		PAGE_EXECUTE);

	printf("ZwMapViewOfSection, ret: %X, BaseAddress: %I64X\r\n",ret,BaseAddress);


	if(ret >= 0)
	{
		*pBaseAddress = BaseAddress;
		*pViewSize = ViewSize;
	}
	else
	{
		ZwClose(hSection);
		ZwClose(hFile);
	}
	return ret;
}

int UnmapDllX(void* BaseAddress, HANDLE hSection,HANDLE hFile)
{
	int ret = ZwUnmapViewOfSection(GetCurrentProcess(),BaseAddress);
	if(hSection != INVALID_HANDLE_VALUE) ZwClose(hSection);
	if(hFile != INVALID_HANDLE_VALUE) ZwClose(hFile);

	return ret;
}


int UpdateChecksum(wchar_t* pFileName,ulong NewCheckSum,ulong OldChecksum)
{
	if(!pFileName) return -1;

	HANDLE hFile = CreateFile(pFileName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		printf("Error opening file for write, err: %X\r\n",GetLastError());
		return -2;
	}

	_IMAGE_DOS_HEADER Dos={0};
	ulong NumRead = 0;
	if(!ReadFile(hFile,&Dos,sizeof(Dos),&NumRead,0))
	{
		printf("Error reading from file\r\n");
		CloseHandle(hFile);
		return -3;
	}

	if(SetFilePointer(hFile,Dos.e_lfanew,0,FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		printf("Error SetFilePointer\r\n");
		CloseHandle(hFile);
		return -4;
	}


	_IMAGE_NT_HEADERS NT={0};
	if(!ReadFile(hFile,&NT,sizeof(NT),&NumRead,0))
	{
		printf("Error reading from file\r\n");
		CloseHandle(hFile);
		return -5;
	}

	if(OldChecksum)
	{
		if(OldChecksum != NT.OptionalHeader.CheckSum)
		{
			printf("Waring: file mismatch\r\n");
		}
	}

	NT.OptionalHeader.CheckSum = NewCheckSum;

	if(SetFilePointer(hFile,Dos.e_lfanew,0,FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		printf("Error SetFilePointer\r\n");
		CloseHandle(hFile);
		return -6;
	}

	if(!WriteFile(hFile,&NT,sizeof(NT),&NumRead,0))
	{
		printf("Error writing file\r\n");
		CloseHandle(hFile);
		return -7;
	}

	printf("Done\r\n");



	CloseHandle(hFile);
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if(argc != 2)
	{
		printf("Usage: PEChecksum.exe c:\\www.exe\r\n");
		return 0;
	}

	//wchar_t* pDllName = L"\\??\\D:\\vbox\\Patching_System\\ntdll.dll";
	wchar_t* pDllName = argv[1];

	wchar_t inBuffer[MAX_PATH+1]={0};
	wcscat(inBuffer,L"\\??\\");
	wcscat(inBuffer,pDllName);

	
	wprintf(L"Calculating checksum: %s\r\n",inBuffer);

	HANDLE hFile = 0;
	HANDLE hSection = 0;
	void* BaseAddress = 0;
	ulonglong ViewSize = 0;

	int ret = MapDllX(inBuffer,&hFile,&hSection,&BaseAddress,&ViewSize);
	if(ret < 0)
	{
		printf("Can't map input file, status: %X\r\n",ret);
		return ret;
	}

	unsigned long FileSize = GetFileSize(hFile,0);



	printf("FileSize: %X\r\n",FileSize);
	printf("BaseAddress: %I64X\r\n",BaseAddress);
	printf("ViewSize: %I64X\r\n",ViewSize);


	_IMAGE_NT_HEADERS* pNt = 
	RtlImageNtHeader(BaseAddress);

	ulong OriginalCheckSum = pNt->OptionalHeader.CheckSum;
	printf("Checksum from PE header: %X\r\n",OriginalCheckSum);

	ulonglong offset_checksum = 4 + sizeof(_IMAGE_FILE_HEADER)+(offsetof(_IMAGE_OPTIONAL_HEADER64,CheckSum));
	//offset_checksum = 4 + sizeof(_IMAGE_FILE_HEADER)+(offsetof(_IMAGE_OPTIONAL_HEADER,CheckSum)); //Same result
	//printf("offset_checksum: %X\r\n",offset_checksum);

	offset_checksum += (((ulonglong)pNt) - ((ulonglong)BaseAddress));
	//printf("offset_checksum: %X\r\n",offset_checksum);



	ulong NumberOfWords = offset_checksum/2;
	ulong LastCheckSum = 0;

	printf("=>LastCheckSum: %X\r\n",LastCheckSum);
	printf("=>BaseAddress: %X\r\n",BaseAddress);
	printf("=>NumberOfWords: %X\r\n",NumberOfWords);

	LastCheckSum =(ushort) ChkSum(LastCheckSum,(unsigned char*)BaseAddress,NumberOfWords);
	printf("LastCheckSum: %X\r\n",LastCheckSum);

	NumberOfWords = ViewSize - offset_checksum;
	NumberOfWords -= 4;
	NumberOfWords /= 2;


	ulonglong offset_subsystem = 4 + sizeof(_IMAGE_FILE_HEADER)+(offsetof(_IMAGE_OPTIONAL_HEADER64,Subsystem));
	offset_subsystem += (((ulonglong)pNt) - ((ulonglong)BaseAddress));
	//printf("offset_subsystem: %X\r\n",offset_subsystem);



	printf("===>LastCheckSum: %X\r\n",LastCheckSum);
	printf("===>BaseAddress: %X\r\n",((unsigned char*)BaseAddress)+offset_subsystem);
	printf("===>NumberOfWords: %X\r\n",NumberOfWords);

	LastCheckSum =(ushort) ChkSum(LastCheckSum,((unsigned char*)BaseAddress)+offset_subsystem,NumberOfWords);
	printf("LastCheckSum: %X\r\n",LastCheckSum);


	ulong CalculatedChecksum = FileSize+LastCheckSum;

	printf("Calculatd checksum: %X\r\n",CalculatedChecksum);


	ret = UnmapDllX(BaseAddress,hSection,hFile);
	//printf("ret: %X\r\n",ret);


	if(OriginalCheckSum != CalculatedChecksum)
	{
		printf("Checksum mismatch, press any key to update it\r\n");
		getchar();
		int retX = UpdateChecksum(pDllName,CalculatedChecksum,OriginalCheckSum);
		printf("retX: %X\r\n",retX);
	}


	
	return 0;
}

