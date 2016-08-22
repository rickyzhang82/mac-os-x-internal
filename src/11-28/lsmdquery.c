// lsmdquery.c
   
#include <unistd.h>
#include <sys/stat.h>
#include <CoreServices/CoreServices.h>
   
#define PROGNAME "lsmdquery"
   
void
exit_usage(void)
{
    fprintf(stderr, "usage: %s -f <smart folder path>\n"
                    "       %s -q <query string>\n", PROGNAME, PROGNAME);
    exit(1);
}
   
void
printDictionaryAsXML(CFDictionaryRef dict)
{
    CFDataRef xml = CFPropertyListCreateXMLData(kCFAllocatorDefault,
                                                (CFPropertyListRef)dict);
    if (!xml)
        return;
   
    write(STDOUT_FILENO, CFDataGetBytePtr(xml), (size_t)CFDataGetLength(xml));
    CFRelease(xml);
}
   
void
notificationCallback(CFNotificationCenterRef  center,
                     void                    *observer,
                     CFStringRef              name,
                     const void              *object,
                     CFDictionaryRef          userInfo)
{
    CFDictionaryRef attributes;
    CFArrayRef      attributeNames;
    CFIndex         idx, count;
    MDItemRef       itemRef = NULL;
    MDQueryRef      queryRef = (MDQueryRef)object;
   
    if (CFStringCompare(name, kMDQueryDidFinishNotification, 0)
           == kCFCompareEqualTo) { // gathered results
        // disable updates, process results, and reenable updates
        MDQueryDisableUpdates(queryRef);
        count = MDQueryGetResultCount(queryRef);
        if (count > 0) {
            for (idx = 0; idx < count; idx++) {
                itemRef = (MDItemRef)MDQueryGetResultAtIndex(queryRef, idx);
                attributeNames = MDItemCopyAttributeNames(itemRef);
                attributes = MDItemCopyAttributes(itemRef, attributeNames);
                printDictionaryAsXML(attributes);
                CFRelease(attributes);
                CFRelease(attributeNames);
            }
            printf("\n%ld results total\n", count);
        }
        MDQueryEnableUpdates(queryRef);
     } else if (CFStringCompare(name, kMDQueryDidUpdateNotification, 0)
                   == kCFCompareEqualTo) { // live update
         CFShow(name), CFShow(object), CFShow(userInfo);
     }
     // ignore kMDQueryProgressNotification
}
   
CFStringRef
ExtractRawQueryFromSmartFolder(const char *folderpath)
{
    int                fd, ret;
    struct stat        sb;
    UInt8             *bufp;
    CFMutableDataRef   xmlData  = NULL;
    CFPropertyListRef  pList    = NULL;
    CFStringRef        rawQuery = NULL, errorString = NULL;
   
    if ((fd = open(folderpath, O_RDONLY)) < 0) {
        perror("open");
        return NULL;
    }
   
    if ((ret = fstat(fd, &sb)) < 0) {
        perror("fstat");
        goto out;
    }
   
    if (sb.st_size <= 0) {
        fprintf(stderr, "no data in smart folder (%s)?\n", folderpath);
        goto out;
    }
   
    xmlData = CFDataCreateMutable(kCFAllocatorDefault, (CFIndex)sb.st_size);
    if (xmlData == NULL) {
        fprintf(stderr, "CFDataCreateMutable() failed\n");
        goto out;
    }
    CFDataIncreaseLength(xmlData, (CFIndex)sb.st_size);
   
    bufp = CFDataGetMutableBytePtr(xmlData);
    if (bufp == NULL) {
        fprintf(stderr, "CFDataGetMutableBytePtr() failed\n");
        goto out;
    }
    ret = read(fd, (void *)bufp, (size_t)sb.st_size);
   
    pList = CFPropertyListCreateFromXMLData(kCFAllocatorDefault,
                                            xmlData,
                                            kCFPropertyListImmutable,
                                            &errorString);
    if (pList == NULL) {
        fprintf(stderr, "CFPropertyListCreateFromXMLData() failed (%s)\n",
                CFStringGetCStringPtr(errorString, kCFStringEncodingASCII));
        CFRelease(errorString);
        goto out;
    }
   
    rawQuery = CFDictionaryGetValue(pList, CFSTR("RawQuery"));
    CFRetain(rawQuery);
    if (rawQuery == NULL) {
        fprintf(stderr, "failed to retrieve query from smart folder\n");
        goto out;
    }
   
out:
    close(fd);
   
    if (pList)
        CFRelease(pList);
    if (xmlData)
        CFRelease(xmlData);
   
    return rawQuery;
}
   
int
main(int argc, char **argv)
{
    int                     i;
    CFStringRef             rawQuery = NULL;
    MDQueryRef              queryRef;
    Boolean                 result;
    CFNotificationCenterRef localCenter;
    MDQueryBatchingParams   batchingParams;
   
    while ((i = getopt(argc, argv, "f:q:")) != -1) {
        switch (i) {
        case 'f':
            rawQuery = ExtractRawQueryFromSmartFolder(optarg);
            break;
        case 'q':
            rawQuery = CFStringCreateWithCString(kCFAllocatorDefault, optarg,
                                                 CFStringGetSystemEncoding());
            break;
   
        default:
            exit_usage();
            break;
        }
    }
   
    if (!rawQuery)
        exit_usage();
   
    queryRef = MDQueryCreate(kCFAllocatorDefault, rawQuery, NULL, NULL);
    if (queryRef == NULL)
        goto out;
   
    if (!(localCenter = CFNotificationCenterGetLocalCenter())) {
        fprintf(stderr, "failed to access local notification center\n");
        goto out;
    }
   
    CFNotificationCenterAddObserver(
        localCenter,          // process-local center
        NULL,                 // observer
        notificationCallback, // to process query finish/update notifications
        NULL,                 // observe all notifications
        (void *)queryRef,     // observe notifications for this object
        CFNotificationSuspensionBehaviorDeliverImmediately);
   
    // maximum number of results that can accumulate and the maximum number
    // of milliseconds that can pass before various notifications are sent
    batchingParams.first_max_num    = 1000; // first progress notification
    batchingParams.first_max_ms     = 1000;
    batchingParams.progress_max_num = 1000; // additional progress notifications
    batchingParams.progress_max_ms  = 1000;
    batchingParams.update_max_num   = 1;    // update notification
    batchingParams.update_max_ms    = 1000;
    MDQuerySetBatchingParameters(queryRef, batchingParams);
   
    // go execute the query
    if ((result = MDQueryExecute(queryRef, kMDQueryWantsUpdates)) == TRUE)
        CFRunLoopRun();
   
out:
    CFRelease(rawQuery);
    if (queryRef)
        CFRelease(queryRef);
   
    exit(0);
}
