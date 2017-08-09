#include <3ds.h>
#include <3ds/types.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

int main(int argc, char **argv)
{
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    printf("FormattMii\nby Yuuki Aiko\n\n");
    printf("Initializing necessary services...\n");

    init_services();

    printf("Press A to begin the System Format.\n");
    printf("Press START to exit.\n");
    u32 kDown = 0;
    u32 kHeld = 0;
    while (aptMainLoop()) 
    {
        hidScanInput();
        kDown = hidKeysDown();
        kHeld = hidKeysHeld();
        if (kDown & KEY_A) 
        {
            printf("Your system will be formatted and all data deleted (excluding your SD card)!\n");
            printf("Press L+R+X to confirm. Press START to exit.\n");
            while (!(kDown & KEY_START))
            {
                hidScanInput();
                kDown = hidKeysDown();
                kHeld = hidKeysHeld();
                if ((kHeld & KEY_L) && (kHeld & KEY_R) && (kHeld & KEY_X))
                {
                    printf("Backing up files that will be deleted to /FormatMii/backup...\n");
                    BackupCtrFs();
                    printf("Formatting your system...\n");
                    printf("Deleting HWCAL0.dat and HWCAL1.dat...\n");
                    HWCALDelete();
                    printf("Calling AM_DeleteAllTwlUserPrograms...\n");
                    AM_DeleteAllTwlUserPrograms();
                    printf("Calling FS_InitializeCtrFileSystem...\n");
                    FSUSER_InitializeCtrFileSystem();
                    printf("Done! Rebooting to Initial Setup...\n");
                    svcSleepThread(5e9);
                    PTMSYSM_ShutdownAsync(0);
                    ptmSysmExit();               
                }
            }
            gfxFlushBuffers();
            gfxSwapBuffers();
            gspWaitForVBlank();
        } 

        if (kDown & KEY_START)
        {
            printf("Format aborted, exiting...\n");
            svcSleepThread(1e9);
            break;

        }
    }
    deinit_services();
    return 0;
}
