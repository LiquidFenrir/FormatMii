#include "utils.h"

#define CONFIRM_KEYCOMBO (KEY_L | KEY_R | KEY_X)

int main(int argc, char **argv)
{
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    printf("FormattMii\nby Yuuki Aiko\n\n");
    printf("Initializing necessary services...\n");
    
    u32 kDown = 0;
    u32 kHeld = 0;
    Result ret = init_services();
    
    if (R_FAILED(ret))
    {
        printf("It seems you tried to launch this application as a 3dsx.\n");
        printf("Unfortunately, you need to quit, activate the sm patch in the rosalina menu, then launch this again for it to work.\n");
        printf("Press START to quit.\n");
        
        while (aptMainLoop()) 
        {
            hidScanInput();
            kDown = hidKeysDown();
            kHeld = hidKeysHeld();
            
            if (kDown & KEY_START) break;
            
            gfxFlushBuffers();
            gfxSwapBuffers();
            gspWaitForVBlank();
        }
    }
    else
    {
        printf("Press A to begin the System Format.\n");
        printf("Press START to exit.\n");
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
                    if ((kHeld & CONFIRM_KEYCOMBO) == CONFIRM_KEYCOMBO)
                    {
                        printf("Backing up files that will be deleted to /FormatMii/backup...\n");
                        BackupCtrFs();
                        /*
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
                        */
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
            
            gfxFlushBuffers();
            gfxSwapBuffers();
            gspWaitForVBlank();
        }
        deinit_services();
    }
    
    gfxExit();
    return 0;
}
