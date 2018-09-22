/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 94):
 * <joshuahuelsman@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Josh Huelsman
 * ----------------------------------------------------------------------------
 */
#ifndef HSF_H
#define HSF_H

#include <stdint.h>
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef char strA;
typedef char strD;

#define HSF_PATH_SEPARATOR '/'

#define HSF_SECTOR_SIZE 2048

#define HSF_VD_ID ("CD001")

#define HSF_VD_TYPE_BR      (0)
#define HSF_VD_TYPE_PVD     (1)
#define HSF_VD_TYPE_SVD     (2)
#define HSF_VD_TYPE_VPD     (3)
#define HSF_VD_TYPE_VDST  (255)

#ifdef __GNUC__
#define HSF_PACKED __attribute__((__packed__))
#else
#define HSF_PACKED
#pragma pack(push, 1)
#endif

typedef struct
{
    u8 Years; //since 1900
    u8 Month;
    u8 Day;
    u8 Hour;
    u8 Minute;
    u8 Second;
    u8 GMTOffset; //15 min intervals
} HSF_PACKED hsf_time_stamp;

typedef struct
{
    char Year[4];
    char Month[2];
    char Day[2];
    char Hour[2];
    char Minute[2];
    char Second[2];
    char HundrethsOfSecond[2];
    u8 GMTOffset;
} HSF_PACKED hsf_date;

#define HSF_FILE_FLAG_HIDDEN          (1 << 0)
#define HSF_FILE_FLAG_IS_DIR          (1 << 1)
#define HSF_FILE_FLAG_ASSOCIATED_FILE (1 << 2)
#define HSF_FILE_FLAG_EAR             (1 << 3)
#define HSF_FILE_FLAG_EAR_PERMISSIONS (1 << 4)
#define HSF_FILE_FLAG_NOT_FINAL_DIR   (1 << 7)

typedef struct
{
    u8 Length;
    u8 EARLength;
    u32 DataLocationLE;
    u32 DataLocationBE;
    u32 DataLengthLE;
    u32 DataLengthBE;
    hsf_time_stamp TimeStamp;
    u8 FileFlags;
    u8 FileUnitSizeInterleaved;
    u8 InterleaveGap;
    u16 VolumeSequenceNumberLE;
    u16 VolumeSequenceNumberBE;
    u8 FileNameLength;
    strD FileName[1];
} HSF_PACKED hsf_directory_entry;

typedef struct
{
    u8 Type;
    strA Id[5];
    u8 Version;
    u8 Data[0];
} HSF_PACKED hsf_volume_descriptor;

typedef struct
{
    u8 Type;
    strA Id[5];
    u8 Version;

    u8 Unused0;
    char SystemIdentifier[32];
    char VolumeIdentifier[32];
    u8 Unused1[8];
    u32 VolumeSpaceSizeLE;
    u32 VolumeSpaceSizeBE;
    u8 Unused2[32];
    u16 VolumeSetSizeLE;
    u16 VolumeSetSizeBE;
    u16 VolumeSequenceNumberLE;
    u16 VolumeSequenceNumberBE;
    u16 LogicalBlockSizeLE;
    u16 LogicalBlockSizeBE;
    u32 PathTableSizeLE;
    u32 PathTableSizeBE;
    u32 PathTableLocationLE;
    u32 OptionalPathTableLocationLE;
    u32 PathTableLocationBE;
    u32 OptionalPathTableLocationBE;
    hsf_directory_entry RootDirectoryEntry;
    char VolumeSetIdentifier[128];
    char PublisherIdentifier[128];
    char DataPreparerIdentifier[128];
    char ApplicationIdentifier[128];
    char CopyrightFileIdentifier[38];
    char AbstractFileIdentifier[36];
    char BibliographicFileIdentifier[37];
    hsf_date VolumeCreationDate;
    hsf_date VolumeModificationDate;
    hsf_date VolumeExpirationDate;
    hsf_date VolumeEffectiveDate;
    u8 FileStructureVersion;
    u8 Unused3;
    u8 ApplicationUsed[512];
    u8 Reserved[653];
} HSF_PACKED hsf_primary_volume_descriptor;

typedef struct
{
    u8 IdentifierLength;
    u8 EARLength;
    u32 ExtentLocation;
    u16 ParentDirectoryIndex;
    strD Identifier[1];
} HSF_PACKED hsf_path_table_entry;

typedef struct
{
    FILE *FileStream;
    hsf_primary_volume_descriptor *PVD;
} HSF_PACKED hsf;

typedef struct
{
    hsf *HSF;
    hsf_directory_entry *DirectoryEntry;
    u32 SeekPosition;
} HSF_PACKED hsf_file;

#ifdef __GNUC__
#else
#pragma pack(pop)
#endif

#undef HSF_PACKED

#ifdef __cplusplus
extern "C" {
#endif

void HsfOpen(hsf *HSF, const char *FileName);
void HsfClose(hsf *HSF);

void *HsfGetSector(hsf *HSF, u32 Sector);
hsf_primary_volume_descriptor *HsfGetPrimaryVolumeDescriptor(hsf *HSF);
hsf_directory_entry *HsfGetDirectoryEntry(hsf *HSF, const char *FileName);

#define HSF_SEEK_SET 0
#define HSF_SEEK_CUR 1
#define HSF_SEEK_END 2

hsf_file *HsfFileOpen(hsf *HSF, const char *FileName);
void HsfFileClose(hsf_file *File);
void HsfFileSeek(hsf_file *File, u32 Offset, int SeekType);
u32  HsfFileTell(hsf_file *File);
void HsfFileRead(void *Buffer, size_t size, size_t count, hsf_file *File);


typedef void (*hsf_visitor_callback)(hsf *HSF, const char *DirPath, hsf_directory_entry *Entry, void *UserPayload);
void HsfVisitDirectory(hsf *HSF, const char *DirPath, hsf_visitor_callback VisitorCallback, void *UserPayload);

#ifdef __cplusplus
} // extern "C"
#endif


#ifdef HSF_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {
#endif


void
HsfOpen(hsf *HSF, const char *FileName)
{
    HSF->FileStream = fopen(FileName, "rb");
    HSF->PVD = HsfGetPrimaryVolumeDescriptor(HSF);
}

void
HsfClose(hsf *HSF)
{
    fclose(HSF->FileStream);
    HSF_FREE(HSF->PVD);
}

void
__HsfGetSector(hsf *HSF, u32 Sector, void *Buffer)
{
    fseek(HSF->FileStream, Sector * HSF_SECTOR_SIZE, SEEK_SET);
    fread(Buffer, HSF_SECTOR_SIZE, 1, HSF->FileStream);
}

void
HsfFileRead(void *Buffer, size_t Size, size_t Count, hsf_file *File)
{
    size_t FinalOffset = File->SeekPosition + File->DirectoryEntry->DataLocationLE * HSF_SECTOR_SIZE;
    fseek(File->HSF->FileStream, FinalOffset, SEEK_SET);
    fread(Buffer, Size, Count, File->HSF->FileStream);
    File->SeekPosition += Size * Count;
}

void *
HsfGetSector(hsf *HSF, u32 Sector)
{
    void *Buffer = HSF_ALLOC(HSF_SECTOR_SIZE);
    __HsfGetSector(HSF, Sector, Buffer);
    return Buffer;
}

hsf_primary_volume_descriptor *
HsfGetPrimaryVolumeDescriptor(hsf *HSF)
{
    void *Buffer = HSF_ALLOC(HSF_SECTOR_SIZE);
    __HsfGetSector(HSF, 0x10, Buffer);
    return (hsf_primary_volume_descriptor *)Buffer;
}

int
__HsfIsInDCharSet(char C) {
    if ( (C >= 'A') || (C <= 'Z') ) return 1;
    if ( (C >= '0') || (C <= '9') ) return 1;
    if (C == '_') return 1;
    

    return 0;
}

int
__HsfIsInACharSet(char C) {
    if (__HsfIsInDCharSet(C)) return 1;
    if (   C == '!' || C == '\"' || C == '%'
        || C == '&' || C == '\'' || C == '('
        || C == ')' || C == '*' || C == '+'
        || C == ',' || C == '-' || C == '.'
        || C == '/' || C == ':' || C == ';'
        || C == '<' || C == '=' || C == '>'
        || C == '?') return 1;

    return 0;
}

int
__HsfStrNCmp(const char *str0, const char *str1, u32 Length) {
    if (!str0 && str1) return -1;
    if (str0 && !str1) return 1;
    if (!str0 && !str1) return 0;

    for (u32 i = 0; i < Length; ++i) {
        int c0 = str0[i];
        int c1 = str1[i];

        int result = c0-c1;
        if (c0 == 0 && c1) return result;
        if (c1 == 0 && c0) return result;        
        if (c0 == 0 && c1 == 0) break;

        if (result == 0) continue;
        return result;
    }

    return 0;
}

u32
__HsfStrLen(const char *Path) {
    const char *End = Path;
    while (*End) End++;
    return (u32)(End-Path);
}

void
__HsfMemCpy(void *_dst, void *_src, u32 size) {
    u8 *dst = (u8 *)_dst;
    u8 *src = (u8 *)_src;

    while (size) {
        *dst = *src;
        dst++;
        src++;
        size--;
    }
}

int
__HsfParseNextPathIdentifier(const char *Path, int StartOffset) {
    u32 PathLength = __HsfStrLen(Path);
    if (StartOffset >= PathLength) return PathLength;

    const char *Start = Path + StartOffset;
    u32 Length = __HsfStrLen(Start);

    for (u32 i = 0; i < Length; ++i) {
        char C = Start[i];

        if (C == HSF_PATH_SEPARATOR) {
            return StartOffset + i;
        } else if (__HsfIsInDCharSet(C)) {
            continue;
        } else {
            return -1;
        }
    }

    return StartOffset+Length;
}

int
__HsfIsValidPath(const char *FileName) {
    if (!FileName) return -1;

    while (*FileName) {
        char C = *FileName;
        if (!(C == HSF_PATH_SEPARATOR || __HsfIsInDCharSet(C)) ) return -1;
        FileName++;
    }

    return 0;
}

u32
__HsfGetFileNameLength(hsf_directory_entry *Entry) {
    u32 MaxLength = Entry->FileNameLength;

    for (u32 i = 0; i <= MaxLength; ++i) {
        char C = Entry->FileName[i];

        // @FixMe -1 hwew is potentially very bad if i == 0, but maybe that never happens, i dunno man.
        if (C == ';') return i-1;
    }

    return MaxLength;
}

hsf_directory_entry *
HsfGetDirectoryEntry(hsf *HSF, const char *FileName)
{
    if (__HsfIsValidPath(FileName) == -1) return 0;

    int Offset = __HsfParseNextPathIdentifier(FileName, 0);
    if (Offset == -1) return 0;

    int NameEnd = __HsfParseNextPathIdentifier(FileName, Offset + 2);

    void *Buffer = HsfGetSector(HSF, HSF->PVD->RootDirectoryEntry.DataLocationLE);
    hsf_directory_entry *RE = (hsf_directory_entry *)Buffer;

    if (NameEnd == 1) {
        return RE;
    }

    Offset = 1;

    int IndexCurrent = 0;
    // @TODO handle the case that you need > 2048 bytes
    int IndexMax = RE->DataLengthLE;

    while (IndexCurrent < IndexMax)
    {
        if (RE->Length == 0) break;
        if (NameEnd==Offset) break;

        u32 Length = NameEnd-Offset;
        u32 FileNameLength = __HsfGetFileNameLength(RE); 

        // @FixMe this doesnt work for files because for some stupid reason file names end with ;<number>
        if (Length == FileNameLength) {
            if (__HsfStrNCmp(&RE->FileName[0], FileName+Offset, Length) == 0)
            {
                if (RE->FileFlags & HSF_FILE_FLAG_IS_DIR) {
                    Offset = NameEnd+1;
                    NameEnd = __HsfParseNextPathIdentifier(FileName, Offset);

                    void *NewSector = HsfGetSector(HSF, RE->DataLocationLE);
                    HSF_FREE(Buffer);
                    Buffer = NewSector;
                    RE = (hsf_directory_entry *)Buffer;
                    IndexCurrent = 0;
                    IndexMax = RE->DataLengthLE;

                    if (NameEnd <= Offset) {
                        return RE;
                    }

                    continue;
                } else {
                    Offset = NameEnd+1;
                    NameEnd = __HsfParseNextPathIdentifier(FileName, Offset);

                    if (NameEnd <= Offset) {
                        hsf_directory_entry *Out = (hsf_directory_entry *)HSF_ALLOC(RE->Length);
                        __HsfMemCpy(Out, RE, RE->Length);
                        HSF_FREE(Buffer);
                        return (hsf_directory_entry *)Out;
                    } else {
                        break; // there's more in the path so the path may be invalid after this point
                    }
                }
            }
        }

        IndexCurrent += RE->Length;
        RE = (hsf_directory_entry *)((u8 *)RE + RE->Length);
    }

    HSF_FREE(Buffer);
    return 0;
}

hsf_file *
HsfFileOpen(hsf *HSF, const char *FileName)
{
    hsf_file *File = (hsf_file *)HSF_ALLOC(sizeof(hsf_file));
    File->HSF = HSF;
    File->DirectoryEntry = HsfGetDirectoryEntry(HSF, FileName);
    File->SeekPosition = 0;

    if (!File->DirectoryEntry) // File not found
    {
        HSF_FREE(File);
        return 0;
    }

    return File;
}

void
HsfFileClose(hsf_file *File)
{
    HSF_FREE(File->DirectoryEntry);
    HSF_FREE(File);
}

void
HsfFileSeek(hsf_file *File, u32 Offset, int SeekType)
{
    // @FixMe this is unsafe because we dont range-check Offset
    if (SeekType == HSF_SEEK_SET)
    {
        File->SeekPosition = Offset;
    }
    else if (SeekType == HSF_SEEK_CUR)
    {
        File->SeekPosition += Offset;
    }
    else if (SeekType == HSF_SEEK_END)
    {
        File->SeekPosition = File->DirectoryEntry->DataLengthLE - Offset;
    }
}

u32
HsfFileTell(hsf_file *File)
{
    return File->SeekPosition;
}

void HsfVisitDirectory(hsf *HSF, const char *DirPath, hsf_visitor_callback VisitorCallback, void *UserPayload) {
    hsf_directory_entry *Entry = HsfGetDirectoryEntry(HSF, DirPath);

    if (Entry) {
        hsf_directory_entry *Entry_Memory = Entry;
        int IndexCurrent = 0;
        int IndexMax = Entry->DataLengthLE;

        while (IndexCurrent < IndexMax)
        {
            if (Entry->Length == 0) break;

            VisitorCallback(HSF, DirPath, Entry, UserPayload);

            IndexCurrent += Entry->Length;
            Entry = (hsf_directory_entry *)((u8 *)Entry + Entry->Length);
        }

        HSF_FREE(Entry_Memory);
    } else {
        // @TODO error
    }
}

#ifdef __cplusplus
} // extern "C"
#endif


#endif // HSF_IMPLEMENTATION

#endif
