// CarbonProcessManager.c
   
#include <Carbon/Carbon.h>
   
#define PROGNAME "cpmtest"
   
int
main(int argc, char **argv)
{
    OSErr               err;
    Str255              path;
    FSSpec              spec;
    LaunchParamBlockRec launchParams;
   
    if (argc != 2) {
        printf("usage: %s <full application path>\n", PROGNAME);
        exit(1);
    }
   
    c2pstrcpy(path, argv[1]);
    err = FSMakeFSSpec(0, // use the default volume
                       0, // parent directory -- determine from filename
                       path, &spec);
    if (err != noErr) {
        printf("failed to make FS spec for application (error %d).\n", err);
        exit(1);
    }
   
    // the extendedBlock constant specifies that we are using the fields that
    // follow this field in the structure
    launchParams.launchBlockID = extendedBlock;
   
    // length of the fields following this field (again, use a constant)
    launchParams.launchEPBLength = extendedBlockLen;
   
    // launch control flags
    // we want the existing program to continue, and not terminate
    // moreover, we want the function to determine the Finder flags itself
    launchParams.launchControlFlags = launchContinue + launchNoFileFlags;
   
    // FSSpec for the application to launch
    launchParams.launchAppSpec = &spec;
   
    // no parameters
    launchParams.launchAppParameters = NULL;
   
    err = LaunchApplication(&launchParams);
   
    if (err != noErr) {
        printf("failed to launch application (error %d).\n", err);
        exit(1);
    }
   
    printf("main: launched application, PSN = %lu_%lu\n",
           launchParams.launchProcessSN.highLongOfPSN,
           launchParams.launchProcessSN.lowLongOfPSN);
    printf("main: continuing\n");
   
    exit(0);
}
