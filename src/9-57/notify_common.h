// notify_common.h
   
#ifndef _NOTIFY_COMMON_H_
#define _NOTIFY_COMMON_H_
   
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <notify.h>
   
#define PREFIX "com.osxbook.notification."
#define NOTIFICATION_BY_FILE_DESCRIPTOR PREFIX "descriptor"
#define NOTIFICATION_BY_MACH_PORT       PREFIX "mach_port"
#define NOTIFICATION_BY_SIGNAL          PREFIX "signal"
   
#define NOTIFICATION_CANCEL             PREFIX "cancel"
   
#endif
