#ifndef UTILS_H
#define UTILS_H

#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WORKING_DIR "/3ds/FormatMii/backup"

Result AM_DeleteAllTwlUserPrograms(void);
void BackupCtrFs(void);
void HWCALDelete(void);

Result init_services(void);
void deinit_services(void);

#endif
