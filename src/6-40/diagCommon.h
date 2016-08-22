// diagCommon.h
   
#ifndef _DIAG_COMMON_H_
#define _DIAG_COMMON_H_
   
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ppc/types.h>
#define _POSIX_C_SOURCE
#include <stdlib.h>
#include <unistd.h>
   
struct savearea;
   
// These headers are not available outside of the kernel source tree
#define KERNEL_PRIVATE
#include <ppc/Diagnostics.h>
#include <console/video_console.h>
#undef KERNEL_PRIVATE
   
// The diagCall() prototype in Diagnostics.h is from the kernel's standpoint
// -- having only one argument: a pointer to the caller's save area. Our user-
// space call takes a variable number of arguments.
//
// Note that diagCall() does have a stub in libSystem.
//
// Here we declare a prototype with a variable number of arguments, define
// a function pointer per that prototype, and point it to the library stub.
typedef int (*diagCall_t)(int op, ...);
diagCall_t diagCall_ = (diagCall_t)diagCall;
   
// Defined in osfmk/vm/pmap.h, which may not be included from user space
#define cppvPsrc        2
#define cppvNoRefSrc    32
   
// Arbitrary upper limit on the number of bytes of memory we will handle
#define MAXBYTES        (8 * 1024 * 1024)
   
#endif // _DIAG_COMMON_H_
