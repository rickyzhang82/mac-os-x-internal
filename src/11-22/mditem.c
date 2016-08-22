// mditem.c
   
#include <getopt.h>
#include <CoreServices/CoreServices.h>
   
#define PROGNAME "mditem"
#define RELEASE_IF_NOT_NULL(ref) { if (ref) { CFRelease(ref); } }
#define EXIT_ON_NULL(ref)        { if (!ref) { goto out; } }
   
void MDItemSetAttribute(MDItemRef item, CFStringRef name, CFTypeRef value);
   
usage(void)
{
    fprintf(stderr, "Set or get metadata. Usage:\n\n\
    %s -g <attribute-name> <filename>                   # get\n\
    %s -s <attribute-name>=<attribute-value> <filename> # set\n",
    PROGNAME, PROGNAME);
}
   
int
main(int argc, char **argv)
{
    int               ch, ret = -1;
    MDItemRef         item = NULL;
    CFStringRef       filePath = NULL, attrName = NULL;
    CFTypeRef         attrValue = NULL;
    char             *valuep;
    CFStringEncoding  encoding = CFStringGetSystemEncoding();
   
    if (argc != 4) {
        usage();
        goto out;
    }
   
    filePath = CFStringCreateWithCString(kCFAllocatorDefault,
                                        argv[argc - 1], encoding);
    EXIT_ON_NULL(filePath);
    argc--;
   
    item = MDItemCreate(kCFAllocatorDefault, filePath);
    EXIT_ON_NULL(item);
   
    while ((ch = getopt(argc, argv, "g:s:")) != -1) {
        switch (ch) {
        case 'g':
            attrName = CFStringCreateWithCString(kCFAllocatorDefault,
                                                 optarg, encoding);
            EXIT_ON_NULL(attrName);
            attrValue = MDItemCopyAttribute(item, attrName);
            EXIT_ON_NULL(attrValue);
            CFShow(attrValue);
            break;
   
        case 's':
            if (!(valuep = strchr(argv[optind - 1], '='))) {
                usage();
                goto out;
            }
                
            *valuep++ = '\0';
            attrName = CFStringCreateWithCString(kCFAllocatorDefault,
                                                 optarg, encoding);
            EXIT_ON_NULL(attrName);
            attrValue = CFStringCreateWithCString(kCFAllocatorDefault,
                                                  valuep, encoding);
            EXIT_ON_NULL(attrValue);
            (void)MDItemSetAttribute(item, attrName, attrValue);
            break;
   
        default:
            usage();
            break;
        }
    }
   
out:
    RELEASE_IF_NOT_NULL(attrName);
    RELEASE_IF_NOT_NULL(attrValue);
    RELEASE_IF_NOT_NULL(filePath);
    RELEASE_IF_NOT_NULL(item);
   
    exit(ret);
}
