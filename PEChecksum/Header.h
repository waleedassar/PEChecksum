#pragma once

#define ulong unsigned long
#define ulonglong unsigned long long
#define ULONG unsigned long
#define ULONGLONG unsigned long long
#define ushort unsigned short
#define USHORT unsigned short


#define SEC_IMAGE 0x01000000
#define OBJ_CASE_INSENSITIVE 0x40




#define 	FILE_SUPERSEDE   0x00000000
#define 	FILE_OPEN   0x00000001
#define 	FILE_CREATE   0x00000002
#define 	FILE_OPEN_IF   0x00000003
#define 	FILE_OVERWRITE   0x00000004
#define 	FILE_OVERWRITE_IF   0x00000005
#define 	FILE_MAXIMUM_DISPOSITION   0x00000005
#define 	FILE_DIRECTORY_FILE   0x00000001
#define 	FILE_WRITE_THROUGH   0x00000002
#define 	FILE_SEQUENTIAL_ONLY   0x00000004
#define 	FILE_NO_INTERMEDIATE_BUFFERING   0x00000008
#define 	FILE_SYNCHRONOUS_IO_ALERT   0x00000010
#define 	FILE_SYNCHRONOUS_IO_NONALERT   0x00000020
#define 	FILE_NON_DIRECTORY_FILE   0x00000040
#define 	FILE_CREATE_TREE_CONNECTION   0x00000080
#define 	FILE_COMPLETE_IF_OPLOCKED   0x00000100
#define 	FILE_NO_EA_KNOWLEDGE   0x00000200
#define 	FILE_OPEN_FOR_RECOVERY   0x00000400
#define 	FILE_RANDOM_ACCESS   0x00000800
#define 	FILE_DELETE_ON_CLOSE   0x00001000
#define 	FILE_OPEN_BY_FILE_ID   0x00002000
#define 	FILE_OPEN_FOR_BACKUP_INTENT   0x00004000
#define 	FILE_NO_COMPRESSION   0x00008000
#define 	FILE_OPEN_REQUIRING_OPLOCK   0x00010000
#define 	FILE_DISALLOW_EXCLUSIVE   0x00020000
#define 	FILE_SESSION_AWARE   0x00040000
#define 	FILE_RESERVE_OPFILTER   0x00100000
#define 	FILE_OPEN_REPARSE_POINT   0x00200000
#define 	FILE_OPEN_NO_RECALL   0x00400000
#define 	FILE_OPEN_FOR_FREE_SPACE_QUERY   0x00800000

#define SECTION_QUERY                0x0001
#define SECTION_MAP_WRITE            0x0002
#define SECTION_MAP_READ             0x0004
#define SECTION_MAP_EXECUTE          0x0008
#define SECTION_EXTEND_SIZE          0x0010
#define SECTION_MAP_EXECUTE_EXPLICIT 0x0020

struct _UNICODE_STRING
{
	unsigned short Length;
	unsigned short MaxLength;
	unsigned long Pad;
	wchar_t* Buffer;
};
typedef struct _OBJECT_ATTRIBUTES {
  ULONGLONG           Length;
  HANDLE          RootDirectory;
  _UNICODE_STRING* ObjectName;
  ULONGLONG           Attributes;
  PVOID           SecurityDescriptor;
  PVOID           SecurityQualityOfService;
} OBJECT_ATTRIBUTES;


typedef struct _IO_STATUS_BLOCK {
  union {
    NTSTATUS Status;
    void*    Pointer;
  } DUMMYUNIONNAME;
  ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;


struct _CLIENT_ID
{
	unsigned long long UniqueProcess;
	unsigned long long UniqueThread;
};


typedef enum _SECTION_INHERIT {
    ViewShare=1,
    ViewUnmap=2
} SECTION_INHERIT, *PSECTION_INHERIT;



extern "C"
{
		int ZwCreateSection(HANDLE* SectionHandle,
						ACCESS_MASK DesiredAccess,
						_OBJECT_ATTRIBUTES* ObjectAttributes,
						_LARGE_INTEGER* MaximumSize,
						ulonglong SectionPageProtection,
						ulonglong AllocationAttributes,
						HANDLE FileHandle);


		int ZwOpenFile(HANDLE* FileHandle,
			ACCESS_MASK DesiredAccess, 
			_OBJECT_ATTRIBUTES* ObjectAttributes, 
			_IO_STATUS_BLOCK* IoStatusBlock, 
			ulonglong ShareAccess,
			ulonglong OpenOptions);


		 
		int ZwMapViewOfSection(HANDLE SectionHandle,
			HANDLE ProcessHandle, 
			void* *BaseAddress,
			ulonglong ZeroBits,
			ulonglong CommitSize, 
			_LARGE_INTEGER* SectionOffset, 
			ulonglong* ViewSize,
			SECTION_INHERIT InheritDisposition,
			ulonglong AllocationType,
			ulonglong Win32Protect);
 
		int ZwClose(HANDLE Handle);

		int ZwUnmapViewOfSection(HANDLE ProcessHandle,void* BaseAddress ) ;


		_IMAGE_NT_HEADERS* RtlImageNtHeader(void* ImageBase);
}


