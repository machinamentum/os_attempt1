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

#define HSF_DEFAULT_PRIMARY_VOLUME_NAME "CD_IMAGE"

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
    u8 years; //since 1900
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;
    u8 gmt_offset; //15 min intervals
} HSF_PACKED Hsf_Time_Stamp;

typedef struct
{
    char year[4];
    char month[2];
    char day[2];
    char hour[2];
    char minute[2];
    char second[2];
    char hundreths_of_second[2];
    u8 gmt_offset;
} HSF_PACKED Hsf_Date;

#define HSF_FILE_FLAG_HIDDEN          (1 << 0)
#define HSF_FILE_FLAG_IS_DIR          (1 << 1)
#define HSF_FILE_FLAG_ASSOCIATED_FILE (1 << 2)
#define HSF_FILE_FLAG_EAR             (1 << 3)
#define HSF_FILE_FLAG_EAR_PERMISSIONS (1 << 4)
#define HSF_FILE_FLAG_NOT_FINAL_DIR   (1 << 7)

typedef struct
{
    u8 length;
    u8 ear_length;
    u32 data_location_le;
    u32 data_location_be;
    u32 data_length_le;
    u32 data_length_be;
    hsf_time_stamp time_stamp;
    u8 file_flags;
    u8 file_unit_size_interleaved;
    u8 interleave_gap;
    u16 volume_sequence_number_le;
    u16 volume_sequence_number_be;
    u8 file_name_length;
    strD file_name[1];
} HSF_PACKED Hsf_Directory_Entry;

typedef struct
{
    u8 type;
    strA id[5];
    u8 version;
    u8 data[0];
} HSF_PACKED Hsf_Volume_Descriptor;

typedef struct
{
    u8 type;
    strA id[5];
    u8 version;
    
    u8 unused0;
    char system_identifier[32];
    char volume_identifier[32];
    u8 unused1[8];
    u32 volume_space_size_le;
    u32 volume_space_size_be;
    u8 unused2[32];
    u16 volume_set_size_le;
    u16 volume_set_size_be;
    u16 volume_sequence_number_le;
    u16 volume_sequence_number_be;
    u16 logical_block_size_le;
    u16 logical_block_size_be;
    u32 path_table_size_le;
    u32 path_table_size_be;
    u32 path_table_location_le;
    u32 optional_path_table_location_le;
    u32 path_table_location_be;
    u32 optional_path_table_location_be;
    Hsf_Directory_Entry root_directory_entry;
    char volume_set_identifier[128];
    char publisher_identifier[128];
    char data_preparer_identifier[128];
    char application_identifier[128];
    char copyright_file_identifier[38];
    char abstrace_file_identifier[36];
    char bibliographic_file_identifier[37];
    Hsf_Date volume_creation_date;
    Hsf_Date volume_modification_date;
    Hsf_Date volume_expiration_date;
    Hsf_Date volume_effective_date;
    u8 file_structure_version;
    u8 unused3;
    u8 application_used[512];
    u8 reserved[653];
} HSF_PACKED Hsf_Primary_Volume_Descriptor;

typedef struct
{
    u8 identifier_length;
    u8 ear_length;
    u32 extent_location;
    u16 parent_directory_index;
    strD identifier[1];
} HSF_PACKED Hsf_Path_Table_Entry;

#ifdef __GNUC__
#else
#pragma pack(pop)
#endif

#undef HSF_PACKED

typedef int (*hsf_read_sector_callback)(void *payload, void *buffer, u32 sector_start, u32 sector_count);

typedef int (*hsf_write_sector_callback)(void *payload, void *buffer, u32 sector_start, u32 sector_count);


#define HSF_IO_READ_ONLY  0
#define HSF_IO_READ_WRITE 1

typedef struct
{
    void *UserPayload;
    hsf_read_sector_callback read_sector_cb;
    hsf_write_sector_callback write_sector_cb;
    hsf_primary_volume_descriptor *PVD;
    
    int io_mode;
} Hsf_Context;

typedef struct
{
    hsf *HSF;
    hsf_directory_entry *DirectoryEntry;
    u32 SeekPosition;
} Hsf_File;

#ifdef __cplusplus
extern "C" {
#endif
    void hsf_create_context(Hsf_Context *ctx, void *callback_payload, hsf_read_sector_callback read_cb, hsf_write_sector_callback write_cb, int io_mode);
    
#ifdef HSF_INCLUDE_STDIO
    void hsf_create_from_fopen(Hsf_Context *ctx, const char *file_name);
    void hsf_destruct_with_fclose(Hsf_Context *ctx);
#endif
    
    void *hsf_get_sector(Hsf_Context *ctx, u32 Sector);
    Hsf_Primary_Volume_Descriptor *hsf_get_primary_volume_descriptor(Hsf_Context *ctx);
    Hsf_Directory_Entry *hsf_get_directory_entry(Hsf_Context *ctx, const char *file_name);
    
#define HSF_SEEK_SET 0
#define HSF_SEEK_CUR 1
#define HSF_SEEK_END 2
    
    Hsf_File *hsf_file_open(Hsf_Context *ctx, const char *file_name);
    void hsf_file_close(Hsf_File *file);
    void hsf_file_seek(Hsf_File *file, u32 offset, int seek_type);
    u32  hsf_file_tell(Hsf_File *file);
    int hsf_file_read(void *buffer, u64 count_bytes, Hsf_File *file);
    
    
    typedef void (*hsf_visitor_callback)(Hsf_Context *ctx, const char *dir_path, Hsf_Directory_Entry *entry, void *user_payload);
    void hsf_visit_directory(Hsf_Context *ctx, const char *dir_path, hsf_visitor_callback visitor_cb, void *user_payload);
    
#ifdef __cplusplus
} // extern "C"
#endif

#endif


#ifdef HSF_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {
#endif
    
    void hsf_create_context(Hsf_Context *ctx, void *callback_payload, hsf_read_sector_callback read_cb, hsf_write_sector_callback write_cb, int io_mode) {
        ctx->user_payload = callback_payload;
        ctx->read_sector_cb = read_cb;
        ctx->write_sector_cb = write_cb;
        ctx->pvd = hsf_get_primary_volume_descriptor(ctx);
        ctx->io_mode = io_mode;
    }
    
    void hsf_destroy_context(Hsf_Context *ctx) {
        if (ctx->pvd) HSF_FREE(ctx->pvd);
        __hsf_zero_memory(ctx, sizeof(Hsf_Context));
    }
    
#ifdef HSF_INCLUDE_STDIO
#include <stdio.h>
    
    int __stdio_read_sector(void *payload, void *buffer, u32 sector, u32 sector_count) {
        int result = fseek((FILE *)payload, sector * HSF_SECTOR_SIZE, SEEK_SET);
        if (result != 0) return -1;
        
        u32 total = 0;
        for (;;) {
            result = fread(buffer, HSF_SECTOR_SIZE, sector_count-total, (FILE *)payload);
            if (result != (sector_count-total)) {
                // check ferror
            }
            
            total += result;
            
            if (total == sector_count) break;
        }
        
        return 0;
    }
    
    int __stdio_write_sector(void *payload, void *buffer, u32 sector, u32 sector_count) {
        int result = fseek((FILE *)payload, sector * HSF_SECTOR_SIZE, SEEK_SET);
        if (result != 0) return -1;
        
        u32 total = 0;
        for (;;) {
            result = fwrite(buffer, HSF_SECTOR_SIZE, sector_count-total, (FILE *)payload);
            if (result != (sector_count-total)) {
                // check ferror
            }
            
            total += result;
            
            if (total == sector_count) break;
        }
        
        return 0;
    }
    
    void hsf_create_from_fopen(Hsf_Context *ctx, const char *file_name) {
        FILE *file = fopen(file_name, "r+");
        if (!file) {
            // file doesnt exist so open it in create-mode
            file = fopen(file_name, "w+");
        }
        
        hsf_create_context(ctx, file, __stdio_read_sector, __stdio_write_sector, HSF_IO_READ_ONLY);
    }
    
    void hsf_destruct_with_fclose(Hsf_Context *ctx)
    {
        fclose((FILE *)ctx->user_payload);
        hsf_destroy_context(ctx);
    }
#endif
    
    int __hsf_is_dchar_set(char C) {
        if ( (C >= 'A') || (C <= 'Z') ) return 1;
        if ( (C >= '0') || (C <= '9') ) return 1;
        if (C == '_') return 1;
        
        
        return 0;
    }
    
    int __hsf_is_achar_set(char C) {
        if (__hsf_is_dchar_set(C)) return 1;
        if (   C == '!' || C == '\"' || C == '%'
            || C == '&' || C == '\'' || C == '('
            || C == ')' || C == '*' || C == '+'
            || C == ',' || C == '-' || C == '.'
            || C == '/' || C == ':' || C == ';'
            || C == '<' || C == '=' || C == '>'
            || C == '?') return 1;
        
        return 0;
    }
    
    int __hsf_strncmp(const char *str0, const char *str1, u32 length) {
        if (!str0 && str1) return -1;
        if (str0 && !str1) return 1;
        if (!str0 && !str1) return 0;
        
        for (u32 i = 0; i < length; ++i) {
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
    
    u32 __hsf_strlen(const char *path) {
        const char *end = path;
        while (*end) end++;
        return (u32)(end-path);
    }
    
    void __hsf_memcpy(void *_dst, const void *_src, u32 size) {
        u8 *dst = (u8 *)_dst;
        u8 *src = (u8 *)_src;
        
        while (size) {
            *dst = *src;
            dst++;
            src++;
            size--;
        }
    }
    
    void __hsf_memset(void *buffer, u8 value, u64 bytes) {
        u8 *data = (u8 *)buffer;
        for (u64  i = 0; i < bytes; ++i) {
            data[i] = value;
        }
    }
    
    void __hsf_zero_memory(void *buffer, u64 bytes) {
        char *data = (char *)buffer;
        for (u64  i = 0; i < bytes; ++i) {
            data[i] = 0;
        }
    }
    
    int __hsf_read_sectors(Hsf_Context *ctx, u32 sector, u32 sector_count, void *buffer)
    {
        return ctx->read_sector_cb(ctx->user_payload, buffer, sector, sector_count);
    }
    
    int __hsf_write_sectors(Hsf_Context *ctx, u32 sector, u32 sector_count, void *buffer) {
        if (ctx->io_mode == HSF_IO_READ_WRITE) {
            return ctx->write_sector_cb(ctx->user_payload, buffer, sector, sector_count); 
        }
        
        return -1;
    }
    
    /*
    typedef struct
    {
        u8 Type;
        strA Id[5];
        u8 Version;
        
        u8 Unused0;
        strA SystemIdentifier[32];
        strD VolumeIdentifier[32];
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
    */
    
    u32 __hsf_max_u32(u32 a, u32 b) {
        return (a > b) ? a : b;
    }
    
    int hsf_format_image(Hsf_Context *ctx, const char *primary_volume_name, u64 total_disc_size_sectors) {
        if (ctx->io_mode == HSF_IO_READ_ONLY) return -1;
        
        if (ctx->pvd) {
            HSF_FREE(ctx->pvd);
            ctx->pvd = 0;
        }
        
        // zero all the disc sectors first, this makes it easy to be compatible with standard
        // fwrite APIs for ensuring the file size without the user worrying about first making sure
        // the file is the correct size. However, this probably shouldn't be used on a direct file
        // handle to a CD-R device since it may make the entire disc read-only
        void *zero_mem = HSF_ALLOC(HSF_SECTOR_SIZE);
        __hsf_zero_memory(zero_mem, HSF_SECTOR_SIZE);
        
        for (u64 i = 0; i < total_disc_size_sectors; ++i) {
            int result = __hsf_write_sectors(ctx, i, 1, zero_mem);
            if (result != 0) {
                HSF_FREE(zero_mem);
                return -1;
            }
        }
        
        if (!primary_volume_name) primary_volume_name = HSF_DEFAULT_PRIMARY_VOLUME_NAME;
        
        Hsf_Primary_Volume_Descriptor *pvd = (Hsf_Primary_Volume_Descriptor *)HSF_ALLOC(HSF_SECTOR_SIZE);
        __hsf_zero_memory(pvd, HSF_SECTOR_SIZE);
        
        pvd->type = HSF_VD_TYPE_PVD;
        __hsf_memcpy(&pvd->id[0], HSF_VD_ID, 5);
        pvd->version = 1;
        __hsf_memcpy(&pvd->system_identifier[0], "", 0); // @TODO see what grub expects here
        __hsf_memcpy(&PVD->volume_identifier[0], primary_volume_name, __hsf_max_u32(__hsf_strlen(primary_volume_name), 32));
        
        
        HSF_FREE(zero_mem);
        return 0;
    }
    
    int hsf_file_read(void *buffer, u64 count_bytes, Hsf_File *file) {
        Hsf_Context *ctx = file->ctx;
        
        u32 sector_start = file->directory_entry->data_location_le + (file->seek_position / HSF_SECTOR_SIZE);
        u32 num_sectors = (count_bytes / HSF_SECTOR_SIZE) + ((count_bytes % HSF_SECTOR_SIZE) ? 1 : 0);
        
        void *temp = HSF_ALLOC(HSF_SECTOR_SIZE * num_sectors);
        int result = __hsf_read_sectors(ctx, sector_start, num_sectors, temp);
        if (result != 0) {
            HSF_FREE(temp);
            return -1;
        }
        
        u32 start = file->seek_position % HSF_SECTOR_SIZE;
        __hsf_memcpy(buffer, ((char *)temp) + start, count_bytes);
        
        file->seek_position += count_bytes;
        HSF_FREE(temp);
        
        return 0;
    }
    
    void *hsf_get_sector(Hsf_Context *ctx, u32 sector) {
        void *buffer = HSF_ALLOC(HSF_SECTOR_SIZE);
        int result = __hsf_read_sectors(ctx, sector, 1, buffer);
        if (result != 0) {
            HSF_FREE(buffer);
            return 0;
        }
        return buffer;
    }
    
    Hsf_Primary_Volume_Descriptor *hsf_get_primary_volume_descriptor(Hsf_Context *ctx) {
        void *buffer = hsf_get_sector(ctx, 0x10);
        return (Hsf_Primary_Volume_Descriptor *)buffer;
    }
    
    int __HsfParseNextPathIdentifier(const char *Path, int StartOffset) {
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
    
    int __HsfIsValidPath(const char *FileName) {
        if (!FileName) return -1;
        
        while (*FileName) {
            char C = *FileName;
            if (!(C == HSF_PATH_SEPARATOR || __HsfIsInDCharSet(C)) ) return -1;
            FileName++;
        }
        
        return 0;
    }
    
    u32 __HsfGetFileNameLength(hsf_directory_entry *Entry) {
        u32 MaxLength = Entry->FileNameLength;
        
        for (u32 i = 0; i <= MaxLength; ++i) {
            char C = Entry->FileName[i];
            
            // @FixMe -1 hwew is potentially very bad if i == 0, but maybe that never happens, i dunno man.
            if (C == ';') return i-1;
        }
        
        return MaxLength;
    }
    
    hsf_directory_entry *HsfGetDirectoryEntry(hsf *HSF, const char *FileName) {
        if (__HsfIsValidPath(FileName) == -1) return 0;
        
        int Offset = __HsfParseNextPathIdentifier(FileName, 0);
        if (Offset == -1) return 0;
        
        int NameEnd = __HsfParseNextPathIdentifier(FileName, Offset + 2);
        
        void *Buffer = HsfGetSector(HSF, HSF->PVD->RootDirectoryEntry.DataLocationLE);
        if (!Buffer) return 0;
        
        hsf_directory_entry *RE = (hsf_directory_entry *)Buffer;
        
        if (NameEnd == 1) {
            return RE;
        }
        
        Offset = 1;
        
        int IndexCurrent = 0;
        // @TODO handle the case that you need > 2048 bytes
        int IndexMax = RE->DataLengthLE;
        
        while (IndexCurrent < IndexMax) {
            if (RE->Length == 0) break;
            if (NameEnd==Offset) break;
            
            u32 Length = NameEnd-Offset;
            u32 FileNameLength = __HsfGetFileNameLength(RE); 
            
            // @FixMe this doesnt work for files because for some stupid reason file names end with ;<number>
            if (Length == FileNameLength) {
                if (__HsfStrNCmp(&RE->FileName[0], FileName+Offset, Length) == 0) {
                    if (RE->FileFlags & HSF_FILE_FLAG_IS_DIR) {
                        Offset = NameEnd+1;
                        NameEnd = __HsfParseNextPathIdentifier(FileName, Offset);
                        
                        void *NewSector = HsfGetSector(HSF, RE->DataLocationLE);
                        HSF_FREE(Buffer);
                        if (!NewSector) return 0;
                        
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
    
    hsf_file *HsfFileOpen(hsf *HSF, const char *FileName)
    {
        hsf_file *File = (hsf_file *)HSF_ALLOC(sizeof(hsf_file));
        File->HSF = HSF;
        File->DirectoryEntry = HsfGetDirectoryEntry(HSF, FileName);
        File->SeekPosition = 0;
        
        // File not found
        if (!File->DirectoryEntry)  {
            HSF_FREE(File);
            return 0;
        }
        
        return File;
    }
    
    void HsfFileClose(hsf_file *File)
    {
        HSF_FREE(File->DirectoryEntry);
        HSF_FREE(File);
    }
    
    void HsfFileSeek(hsf_file *File, u32 Offset, int SeekType)
    {
        // @FixMe this is unsafe because we dont range-check Offset
        if (SeekType == HSF_SEEK_SET) {
            File->SeekPosition = Offset;
        } else if (SeekType == HSF_SEEK_CUR) {
            File->SeekPosition += Offset;
        } else if (SeekType == HSF_SEEK_END) {
            File->SeekPosition = File->DirectoryEntry->DataLengthLE - Offset;
        }
    }
    
    u32 HsfFileTell(hsf_file *File)
    {
        return File->SeekPosition;
    }
    
    void HsfVisitDirectory(hsf *HSF, const char *DirPath, hsf_visitor_callback VisitorCallback, void *UserPayload) {
        hsf_directory_entry *Entry = HsfGetDirectoryEntry(HSF, DirPath);
        
        if (Entry) {
            hsf_directory_entry *Entry_Memory = Entry;
            int IndexCurrent = 0;
            int IndexMax = Entry->DataLengthLE;
            
            while (IndexCurrent < IndexMax) {
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
