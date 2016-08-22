// fb-dump.c
   
#include <getopt.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <ApplicationServices/ApplicationServices.h>
   
#define PROGNAME          "fb-dump"
#define DUMPFILE_TMPDIR   "/tmp/"
#define DUMPFILE_TEMPLATE "fb-dump.XXXXXX"
   
...
   
int
main(int argc, char * argv[])
{
    int      i, saveFD = -1;
    char     template[] = DUMPFILE_TMPDIR DUMPFILE_TEMPLATE;
    uint32_t width, height, rowBytes, rowUInt32s, *screen;
   
    CGDirectDisplayID targetDisplay = 0;
   
    // populate targetDisplay as in Figure 10–23
    // use listDisplays() from Figure 10–23
    ...
   
    screen = (uint32_t *)CGDisplayBaseAddress(targetDisplay);
    rowBytes = CGDisplayBytesPerRow(targetDisplay);
    rowUInt32s = rowBytes / 4;
    width = CGDisplayPixelsWide(targetDisplay); 
    height = CGDisplayPixelsHigh(targetDisplay);
    
    if ((saveFD = mkstemp(template)) < 0) {
        perror("mkstemps");
        exit(1);
    }
   
    for (i = 0; i < height; i++)
        write(saveFD, screen + i * rowUInt32s, width * sizeof(uint32_t));
   
    close(saveFD);
   
    exit(0);
}
