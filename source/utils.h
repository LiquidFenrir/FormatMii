#ifndef UTILS_H
#define UTILS_H
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>

FS_Archive sdmcArchive;
FS_Archive ctrfsArchive;
Result AM_DeleteAllTwlUserPrograms(void);
void BackupCtrFs(void);
void HWCALDelete(void);
void init_services(void);
void deinit_services(void);

#endif
