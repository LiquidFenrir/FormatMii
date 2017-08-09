#include "utils.h"

static FS_Archive sdmcArchive;
static FS_Archive ctrfsArchive;
static char * backupPath;

Result init_services(void)
{
    Result ret = amInit();
    if (R_FAILED(ret)) return ret;
    ptmSysmInit();
    fsInit();
    
    FSUSER_OpenArchive(&sdmcArchive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
    FSUSER_OpenArchive(&ctrfsArchive, ARCHIVE_NAND_CTR_FS, fsMakePath(PATH_EMPTY, ""));
    
    return 0;
}

void deinit_services(void)
{
    fsExit();
    ptmSysmExit();
    amExit();
}

Result AM_DeleteAllTwlUserPrograms(void)
{
    Handle amHandle = *amGetSessionHandle();
    u32 *cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = IPC_MakeHeader(0x1D,0,0); // 0x001D0000
    Result ret = 0;
    if(R_FAILED(ret = svcSendSyncRequest(amHandle)))
        return ret;
    return cmdbuf[1];
}

void HWCALDelete(void) {
    FSUSER_DeleteFile(ctrfsArchive, fsMakePath(PATH_ASCII, "/ro/sys/HWCAL0.dat"));
    FSUSER_DeleteFile(ctrfsArchive, fsMakePath(PATH_ASCII, "/ro/sys/HWCAL1.dat"));
}

static void makeDirs(const char * fullPath)
{
    Result ret = 0;
    
    char * path = strdup(fullPath);
    
    for (char * slashpos = strchr(path+1, '/'); slashpos != NULL; slashpos = strchr(slashpos+1, '/')) {
        char bak = *(slashpos);
        *(slashpos) = '\0';
        FS_Path dirpath = fsMakePath(PATH_ASCII, path);
        Handle dirHandle;
        ret = FSUSER_OpenDirectory(&dirHandle, sdmcArchive, dirpath);
        if(ret == 0) FSDIR_Close(dirHandle);
        else {
            printf("Creating dir: %s\n", path);
            ret = FSUSER_CreateDirectory(sdmcArchive, dirpath, FS_ATTRIBUTE_DIRECTORY);
            if (ret != 0) printf("Error in:\nFSUSER_CreateDirectory\nError: 0x%08x\n", (unsigned int)ret);
        }
        *(slashpos) = bak;
    }
    free(path);
}

static void openSDFile(const char * path, Handle * fileHandle)
{    
    makeDirs(path);
    Result ret = 0;
    ret = FSUSER_OpenFile(fileHandle, sdmcArchive, fsMakePath(PATH_EMPTY, path), (FS_OPEN_WRITE | FS_OPEN_CREATE), 0);
    if (ret != 0) printf("FSUSER_OpenFileDirectly: 0x%.8lx\n", ret);
    ret = FSFILE_SetSize(*fileHandle, 0); //truncate the file to remove previous contents before writing
    if (ret != 0) printf("FSFILE_SetSize: 0x%.8lx\n", ret);
}

static void recursiveCopyCtrFs(const char * srcPath)
{
    char * sdmcPath;
    asprintf(&sdmcPath, "%s%s", backupPath, srcPath);
    printf("Copying from\nCTRNAND:%s\n to SD:%s\n", srcPath, sdmcPath);
    
    Handle fileHandle;
    Result ret = FSUSER_OpenFile(&fileHandle, ctrfsArchive, fsMakePath(PATH_ASCII, srcPath), FS_OPEN_READ, 0);
    if (ret)
    {
        //assume it's a dir
        FSFILE_Close(fileHandle);
        printf("FSUSER_OpenFile: %.8lx\n", ret);
        Handle dirHandle;
        FSUSER_OpenDirectory(&dirHandle, ctrfsArchive, fsMakePath(PATH_ASCII, srcPath));
        
        u32 entriesRead = 0;
        do {
            FS_DirectoryEntry entry;
            FSDIR_Read(dirHandle, &entriesRead, 1, &entry);
            char * dirname = calloc(0x106*2, sizeof(char));
            ssize_t name_len = utf16_to_utf8((u8*)dirname, entry.name, 0x106);
            ssize_t after_name_len = strlen(dirname);
            printf("len: %u vs %u\n", name_len, after_name_len);
            char * fullPath;
            asprintf(&fullPath, "%s%s", srcPath, dirname);
            recursiveCopyCtrFs(fullPath);
            free(fullPath);
            free(dirname);
        } while(entriesRead);
        
        FSDIR_Close(dirHandle);
    }
    else 
    {
        //assume it opened as a file correctly
        Handle sdmcFileHandle;
        openSDFile(sdmcPath, &sdmcFileHandle);
        
        u32 bytesRead = 0;
        u32 bytesWritten = 0;
        u64 offset = 0;
        
        u32 toRead = 0x1000;
        u8 * buf = malloc(toRead);
        do {
            FSFILE_Read(fileHandle, &bytesRead, offset, buf, toRead);
            FSFILE_Write(sdmcFileHandle, &bytesWritten, offset, buf, bytesRead, 0);
            offset += bytesRead;
        } while(bytesRead);
        
        FSFILE_Close(sdmcFileHandle);
        FSFILE_Close(fileHandle);
    }
    free(sdmcPath);
}

void BackupCtrFs(void) 
{
    time_t unixTime = time(NULL);
    struct tm* timeStruct = gmtime((const time_t *)&unixTime);
    
    int hours = timeStruct->tm_hour;
    int minutes = timeStruct->tm_min;
    int seconds = timeStruct->tm_sec;
    int day = timeStruct->tm_mday;
    int month = timeStruct->tm_mon;
    int year = timeStruct->tm_year +1900;
    
    asprintf(&backupPath, "%s-%i%i%iT%i%i%i", WORKING_DIR, year, month, day, hours, minutes, seconds);
    
    recursiveCopyCtrFs("/ro/sys/HWCAL0.dat");
    recursiveCopyCtrFs("/ro/sys/HWCAL1.dat");
    recursiveCopyCtrFs("/data");
    recursiveCopyCtrFs("/dbs");
    
    printf("Everything copied to %s on the SD card!\n", backupPath);
    free(backupPath);
}
