// frr.c
   
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
   
// Constant on the PowerPC 
#define BYTES_PER_INSTRUCTION 4
   
// Branch instruction's major opcode
#define BRANCH_MOPCODE 0x12
   
// Large enough size for a function stub
#define DEFAULT_STUBSZ 128
   
// Atomic update function
//
int hw_compare_and_store(u_int32_t  old,
                         u_int32_t  new,
                         u_int32_t *address,
                         u_int32_t *dummy_address);
   
// Structure corresponding to a branch instruction
//
typedef struct branch_s {
    u_int32_t OP: 6;  // bits 0 - 5, primary opcode
    u_int32_t LI: 24; // bits 6 - 29, LI
    u_int32_t AA: 1;  // bit 30, absolute address
    u_int32_t LK: 1;  // bit 31, link or not
} branch_t;
   
// Each instance of rerouting has the following data structure associated with
// it. A pointer to a frr_data_t is returned by the "install" function. The
// "remove" function takes the same pointer as argument.
//
typedef struct frr_data_s {
    void *f_orig; // "original" function
    void *f_new;  // user-provided "new" function
    void *f_stub; // stub to call "original" inside "new"
    char  f_bytes[BYTES_PER_INSTRUCTION]; // bytes from f_orig
} frr_data_t;
   
// Given an "original" function and a "new" function, frr_install() reroutes
// so that anybody calling "original" will actually be calling "new". Inside
// "new", it is possible to call "original" through a stub.
//
frr_data_t *
frr_install(void *original, void *new)
{
    int         ret = -1;
    branch_t    branch;
    frr_data_t *FRR = (frr_data_t *)0;
    u_int32_t   target_address, dummy_address;
   
    // Check new's address
    if ((u_int32_t)new >> 25) {
        fprintf(stderr, "This demo is out of luck. \"new\" too far.\n");
        goto ERROR;
    } else
        printf("    FRR: \"new\" is at address %#x.\n", (u_int32_t)new);
   
    // Allocate space for FRR metadata
    FRR = (frr_data_t *)malloc(sizeof(frr_data_t));
    if (!FRR)
        return FRR;
   
    FRR->f_orig = original;
    FRR->f_new = new;
   
    // Allocate space for the stub to call the original function
    FRR->f_stub = (char *)malloc(DEFAULT_STUBSZ);
    if (!FRR->f_stub) {
        free(FRR);
        FRR = (frr_data_t *)0;
        return FRR;
    }
   
    // Prepare to write to the first 4 bytes of "original"
    ret = mprotect(FRR->f_orig, 4, PROT_READ|PROT_WRITE|PROT_EXEC);
    if (ret != 0)
        goto ERROR;
   
    // Prepare to populate the stub and make it executable
    ret = mprotect(FRR->f_stub, DEFAULT_STUBSZ, PROT_READ|PROT_WRITE|PROT_EXEC);
    if (ret != 0)
        goto ERROR;
   
    memcpy(FRR->f_bytes, (char *)FRR->f_orig, BYTES_PER_INSTRUCTION);
   
    // Unconditional branch (relative)
    branch.OP = BRANCH_MOPCODE;
    branch.AA = 1;
    branch.LK = 0;
   
    // Create unconditional branch from "stub" to "original"
    target_address = (u_int32_t)(FRR->f_orig + 4) >> 2;
    if (target_address >> 25) {
        fprintf(stderr, "This demo is out of luck. Target address too far.\n");
        goto ERROR;
    } else
        printf("    FRR: target_address for stub -> original is %#x.\n",
               target_address);
    branch.LI = target_address;
    memcpy((char *)FRR->f_stub, (char *)FRR->f_bytes, BYTES_PER_INSTRUCTION);
    memcpy((char *)FRR->f_stub + BYTES_PER_INSTRUCTION, (char *)&branch, 4);
   
    // Create unconditional branch from "original" to "new"
    target_address = (u_int32_t)FRR->f_new >> 2;
    if (target_address >> 25) {
        fprintf(stderr, "This demo is out of luck. Target address too far.\n");
        goto ERROR;
    } else
        printf("    FRR: target_address for original -> new is %#x.\n",
               target_address);
    branch.LI = target_address;
    ret = hw_compare_and_store(*((u_int32_t *)FRR->f_orig),
                               *((u_int32_t *)&branch),
                               (u_int32_t *)FRR->f_orig,
                               &dummy_address);
    if (ret != 1) {
        fprintf(stderr, "Atomic store failed.\n");
        goto ERROR;
    } else
        printf("    FRR: Atomically updated instruction.\n");
   
    return FRR;
   
    ERROR:
    if (FRR && FRR->f_stub)
        free(FRR->f_stub);
    if (FRR)
        free(FRR);
    return FRR;
}
   
int
frr_remove(frr_data_t *FRR)
{
    int       ret;
    u_int32_t dummy_address;
   
    if (!FRR)
        return 0;
   
    ret = mprotect(FRR->f_orig, 4, PROT_READ|PROT_WRITE|PROT_EXEC);
    if (ret != 0)
        return -1;
   
    ret = hw_compare_and_store(*((u_int32_t *)FRR->f_orig),
                               *((u_int32_t *)FRR->f_bytes),
                               (u_int32_t *)FRR->f_orig,
                               &dummy_address);
   
    if (FRR && FRR->f_stub)
        free(FRR->f_stub);
   
    if (FRR)
        free(FRR);
   
    FRR = (frr_data_t *)0;
   
    return 0;
}
   
int
function(int i, char *s)
{
    int   ret;
    char *m = s;
   
    if (!s)
        m = "(null)";
   
    printf(" CALLED: function(%d, %s).\n", i, m);
    ret = i + 1;
    printf(" RETURN: %d = function(%d, %s).\n", ret, i, m);
   
    return ret;
}
   
int (* function_stub)(int, char *);
   
int
function_new(int i, char *s)
{
    int   ret = -1;
    char *m = s;
   
    if (!s)
        m = "(null)";
   
    printf(" CALLED: function_new(%d, %s).\n", i, m);
   
    if (function_stub) {
        printf("CALLING: function_new() --> function_stub().\n");
        ret = function_stub(i, s);
    } else {
        printf("function_new(): function_stub missing.\n");
    }
   
    printf(" RETURN: %d = function_new(%d, %s).\n", ret, i, m);
   
    return ret;
}
   
int
main(int argc, char **argv)
{
    int         ret;
    int         arg_i = 2;
    char       *arg_s = "Hello, World!";
    frr_data_t *FRR;
   
    function_stub = (int(*)(int, char *))0;
   
    printf("[Clean State]\n");
        printf("CALLING: main() --> function().\n");
    ret = function(arg_i, arg_s);
   
    printf("\n[Installing Rerouting]\n");
    printf("Maximum branch target address is %#x (32MB).\n", (1 << 25));
    FRR = frr_install(function, function_new);
    if (FRR)
        function_stub = FRR->f_stub;
    else {
        fprintf(stderr, "main(): frr_install failed.\n");
        return 1;
    }
   
    printf("\n[Rerouting installed]\n");
    printf("CALLING: main() --> function().\n");
    ret = function(arg_i, arg_s);
   
    ret = frr_remove(FRR);
    if (ret != 0) {
        fprintf(stderr, "main(): frr_remove failed.\n");
        return 1;
    }
   
    printf("\n[Rerouting removed]\n");
    printf("CALLING: main() --> function().\n");
    ret = function(arg_i, arg_s);
   
    return 0;
}
