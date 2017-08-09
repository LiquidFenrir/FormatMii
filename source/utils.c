#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

FS_Archive sdmcArchive;
FS_Archive ctrfsArchive;

void init_services(void)
{
    fsInit();
    sdmcInit();
    amInit();
    aptInit();
    ptmSysmInit();
    FSUSER_OpenArchive(&sdmcArchive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
    if (R_SUMMARY(FSUSER_OpenDirectory(NULL, sdmcArchive, fsMakePath(PATH_ASCII, "/FormatMii")) == RS_NOTFOUND))
        FSUSER_CreateDirectory(sdmcArchive, fsMakePath(PATH_ASCII, "/FormatMii"), FS_ATTRIBUTE_DIRECTORY);
    FSUSER_OpenArchive(&ctrfsArchive, ARCHIVE_NAND_CTR_FS, fsMakePath(PATH_EMPTY, ""));
}

void deinit_services(void)
{
    amExit();
    aptExit();
    fsExit();
    sdmcExit();
    gfxExit();
    ptmSysmExit();
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

void BackupCtrFs(void) 
{
    if (R_SUMMARY(FSUSER_OpenDirectory(NULL, sdmcArchive, fsMakePath(PATH_ASCII, "/FormatMii/backup")) == RS_NOTFOUND))
        FSUSER_CreateDirectory(sdmcArchive, fsMakePath(PATH_ASCII, "/FormatMii/backup"), FS_ATTRIBUTE_DIRECTORY);
}