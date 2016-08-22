// cnid2path.c
   
#include <stdio.h>
#include <sys/param.h>
#include <Carbon/Carbon.h>
   
typedef struct { // this is returned by PBResolveFileIDRefSync()
    unsigned char length;
    unsigned char characters[255];
} HFSStr255;
   
int
main(int argc, char **argv)
{
    FIDParam   pb;
    OSStatus   result;
    long       tmpSrcDirID;
    int        len = MAXPATHLEN - 1;
    char       path[MAXPATHLEN] = { '\0' };
    char      *cursor = (char *)(path + (MAXPATHLEN - 1));
    char      *upath;
    HFSStr255 *p, pbuf;
   
    if (argc != 2) {
        fprintf(stderr, "usage: %s <CNID>\n", argv[0]);
        exit(1);
    }
   
    tmpSrcDirID = atoi(argv[1]);
   
    pb.ioVRefNum = 0;   // no volume reference number -- use default
    pb.ioSrcDirID = -1; // parent directory ID -- we don't know it yet
   
    while (1) {
   
        pb.ioNamePtr = (StringPtr)&pbuf; // a pointer to a pathname
        pb.ioFileID = tmpSrcDirID;       // the given CNID
        if ((result = PBResolveFileIDRefSync((HParmBlkPtr)&pb)) < 0)
            return result;
   
        if ((pb.ioSrcDirID == tmpSrcDirID) || (len <= 0)) {
            cursor++;
            break;
        }
   
        p = (HFSStr255 *)&pbuf;
        cursor -= (p->length);
        memcpy(cursor, p->characters, p->length);
        *--cursor = '/';
        len -= (1 + p->length);
   
        tmpSrcDirID = pb.ioSrcDirID;
    }
   
    if ((upath = strchr(cursor, '/')) != NULL) {
        *upath = '\0';
        upath++;
    } else
        upath = "";
   
    printf("%s:/%s\n", cursor, upath);
   
    return 0;
}
