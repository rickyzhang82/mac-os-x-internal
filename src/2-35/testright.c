// testright.c
   
#include <stdio.h>
#include <stdlib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Authorization.h>
#include <Security/AuthorizationDB.h>
   
const char kTestActionRightName[] = "com.osxbook.Test.DoSomething";
   
int
main(int argc, char **argv)
{
    OSStatus            err;
    AuthorizationRef    authRef;
    AuthorizationItem   authorization = { 0, 0, 0, 0 };
    AuthorizationRights rights = { 1, &authorization };
    AuthorizationFlags  flags = kAuthorizationFlagInteractionAllowed |\
                                kAuthorizationFlagExtendRights
   
    // Create a new authorization reference
    err = AuthorizationCreate(NULL, NULL, 0, &authRef);
    if (err != noErr) {
        fprintf(stderr, "failed to connect to Authorization Services\n");
        return err;
    }
   
    // Check if the right is defined
    err = AuthorizationRightGet(kTestActionRightName, NULL);
    if (err != noErr) {
        if (err == errAuthorizationDenied) {
            // Create right in the policy database
            err = AuthorizationRightSet(
                      authRef,
                      kTestActionRightName, 
                      CFSTR(kAuthorizationRuleAuthenticateAsSessionUser),
                      CFSTR("You must be authorized to perform DoSomething."),
                      NULL,
                      NULL
                  );
            if (err != noErr) {
                fprintf(stderr, "failed to set up right\n");
                return err;
            }
        }
        else {
            // Give up
            fprintf(stderr, "failed to check right definition (%ld)\n", err);
            return err;
        }
    }
   
    // Authorize right
    authorization.name = kTestActionRightName;
    err = AuthorizationCopyRights(authRef, &rights, NULL, flags, NULL);
    if (err != noErr)
        fprintf(stderr, "failed to acquire right (%s)\n", kTestActionRightName);
    else
        fprintf(stderr, "right acquired (%s)\n", kTestActionRightName);
   
    // Free the memory associated with the authorization reference
    AuthorizationFree(authRef, kAuthorizationFlagDefaults);
   
    exit(0);
}
