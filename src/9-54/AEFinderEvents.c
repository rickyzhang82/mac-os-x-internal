// AEFinderEvents.c
   
#include <Carbon/Carbon.h>
   
OSStatus
AEFinderEventBuildAndSend(const char   *path,
                          AEEventClass  eventClass,
                          AEEventID     eventID)
{
    OSStatus     err = noErr;
    FSRef        fsRef;
    AliasHandle  fsAlias;
    AppleEvent   eventToSend = { typeNull, nil };
    AppleEvent   eventReply  = { typeNull, nil };
    AEBuildError eventBuildError;
    const OSType finderSignature = 'MACS';
   
    if ((err = FSPathMakeRef((unsigned char *)path, &fsRef, NULL)) != noErr) {
        fprintf(stderr, "Failed to get FSRef from path (%s)\n", path);
        return err;
    }
   
    if ((err = FSNewAliasMinimal(&fsRef, &fsAlias)) != noErr) {
        fprintf(stderr, "Failed to create alias for path (%s)\n", path);
        return err;
    }
   
    err = AEBuildAppleEvent(
              eventClass,            // Event class for the resulting event
              eventID,               // Event ID for the resulting event
              typeApplSignature,     // Address type for next two parameters
              &finderSignature,      // Finder signature (pointer to address)
              sizeof(OSType),        // Size of Finder signature
              kAutoGenerateReturnID, // Return ID for the created event
              kAnyTransactionID,     // Transaction ID for this event
              &eventToSend,          // Pointer to location for storing result
              &eventBuildError,      // Pointer to error structure
              "'----':alis(@@)",     // AEBuild format string describing the
                                     // AppleEvent record to be created
              fsAlias    
        );
    if (err != noErr) {
        fprintf(stderr, "Failed to build Apple Event (error %d)\n", (int)err);
        return err;
    }
   
    err = AESend(&eventToSend,
                 &eventReply,
                 kAEWaitReply,      // Send mode (wait for reply)
                 kAENormalPriority,
                 kNoTimeOut,
                 nil,               // No pointer to idle function
                 nil);              // No pointer to filter function
    
    if (err != noErr)
        fprintf(stderr, "Failed to send Apple Event (error %d)\n", (int)err);
   
    // Dispose of the send/reply descs
    AEDisposeDesc(&eventToSend);
    AEDisposeDesc(&eventReply);
   
    return err;
}
   
int
main(int argc, char **argv)
{
    switch (argc) {
    case 2:
        (void)AEFinderEventBuildAndSend(argv[1], kCoreEventClass,
                                        kAEOpenDocuments);
        break;
   
    case 3:
        (void)AEFinderEventBuildAndSend(argv[2], kAEMiscStandards,
                                        kAEMakeObjectsVisible);
        break;
   
    default:
        fprintf(stderr, "usage: %s [-r] <path>\n", argv[0]);
        exit(1);
        break;
    }
   
    exit(0);
}
