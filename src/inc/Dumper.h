#ifndef _DUMPER_H_
#define _DUMPER_H_

#define MM_DUMPER_FILE_NAME_LENGTH_W 1024

typedef struct _MM_DUMPER{
    HANDLE                hFileHandle;
    OBJECT_ATTRIBUTES     FileObjAttr;

    WCHAR                 FileName[MM_DUMPER_FILE_NAME_LENGTH_W];
}MM_DUMPER, *PMM_DUMPER, **PPMM_DUMPER;


//////////////////////////////////////////////////////////////////////////

NTSTATUS
MmDumperCreateDumpFile(PMM_DUMPER pMmDumper);

NTSTATUS
MmDumperDumpMem(PMM_DUMPER pMmDumper, PUCHAR pMemVa, ULONG Size);

//////////////////////////////////////////////////////////////////////////

NTSTATUS
MmDumperOpenDumpFile(PMM_DUMPER pMemLoader, PWCHAR pFileName);

NTSTATUS
MmDumperLoadFileToMemory(PMM_DUMPER pMemLoader, PUCHAR pMemVa, ULONG MemSize, PULONG pReadedSize);

//////////////////////////////////////////////////////////////////////////

NTSTATUS
MmDumperCloseDumpFile(PMM_DUMPER pMmDumper);


#endif