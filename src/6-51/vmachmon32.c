// vmachmon32.c
// Mac OS X Virtual Machine Monitor (Vmm) facility demonstration
   
#define PROGNAME "vmachmon32"
   
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <mach/mach.h>
#include <architecture/ppc/cframe.h>
   
#ifndef _VMACHMON32_KLUDGE_
// We need to include xnu/osfmk/ppc/vmachmon.h, which includes several other
// kernel headers and is not really meant for inclusion in user programs.
// We perform the following kludges to include vmachmon.h to be able to
// compile this program:
//
// 1. Provide dummy forward declarations for data types that vmachmon.h
//    needs, but we will not actually use.
// 2. Copy vmachmon.h to the current directory from the kernel source tree.
// 3. Remove or comment out "#include <ppc/exception.h>" from vmachmon.h.
//
struct  savearea;             // kludge #1
typedef int ReturnHandler;    // kludge #1
typedef int pmap_t;           // kludge #1
typedef int facility_context; // kludge #1
#include "vmachmon.h"         // kludge #2
#endif
   
#define OUT_ON_MACH_ERROR(msg, retval) \
    if (kr != KERN_SUCCESS) { mach_error("*** " msg ":" , kr); goto out; }
   
// vmm_dispatch() is a PowerPC-only system call that allows us to invoke
// functions residing in the Vmm dispatch table. In general, Vmm routines
// are available to user space, but the C library (or another library) does
// not contain stubs to call them. Thus, we must go through vmm_dispatch(),
// using the index of the function to call as the first parameter in GPR3.
//
// Since vmachmon.h contains the kernel prototype of vmm_dispatch(), which
// is not what we want, we will declare our own function pointer and set
// it to the stub available in the C library.
//
typedef kern_return_t (* vmm_dispatch_func_t)(int, ...);
vmm_dispatch_func_t my_vmm_dispatch;
   
// Convenience data structure for pretty-printing Vmm features
struct VmmFeature {
    int32_t  mask;
    char    *name;
} VmmFeatures[] = {
    { kVmmFeature_LittleEndian,        "LittleEndian"        },
    { kVmmFeature_Stop,                "Stop"                },
    { kVmmFeature_ExtendedMapping,     "ExtendedMapping"     },
    { kVmmFeature_ListMapping,         "ListMapping"         },
    { kVmmFeature_FastAssist,          "FastAssist"          },
    { kVmmFeature_XA,                  "XA"                  },
    { kVmmFeature_SixtyFourBit,        "SixtyFourBit"        },
    { kVmmFeature_MultAddrSpace,       "MultAddrSpace"       },
    { kVmmFeature_GuestShadowAssist,   "GuestShadowAssist"   },
    { kVmmFeature_GlobalMappingAssist, "GlobalMappingAssist" },
    { kVmmFeature_HostShadowAssist,    "HostShadowAssist"    },
    { kVmmFeature_MultAddrSpaceAssist, "MultAddrSpaceAssist" },
    { -1, NULL },
};
   
// For Vmm messages that we print
#define Printf(fmt, ...) printf("Vmm> " fmt, ## __VA_ARGS__)
   
// PowerPC instruction template: add immediate, D-form
typedef struct I_addi_d_form {
    u_int32_t OP: 6;  // major opcode
    u_int32_t RT: 5;  // target register
    u_int32_t RA: 5;  // register operand
    u_int32_t SI: 16; // immediate operand
} I_addi_d_form;
   
// PowerPC instruction template: unconditional branch, I-form
typedef struct branch_i_form {
    u_int32_t OP: 6;  // major opcode
    u_int32_t LI: 24; // branch target (immediate)
    u_int32_t AA: 1;  // absolute or relative
    u_int32_t LK: 1;  // link or not
} I_branch_i_form;
   
// PowerPC instruction template: add, XO-form
typedef struct I_add_xo_form {
    u_int32_t OP: 6;  // major opcode
    u_int32_t RT: 5;  // target register
    u_int32_t RA: 5;  // register operand A
    u_int32_t RB: 5;  // register operand B
    u_int32_t OE: 1;  // alter SO, OV?
    u_int32_t XO: 9;  // extended opcode
    u_int32_t Rc: 1;  // alter CR0?
} I_add_xo_form;
   
// Print the bits of a 32-bit number
void
prbits32(u_int32_t u)
{
    u_int32_t i = 32;
   
    for (; i > 16 && i--; putchar(u & 1 << i ? '1' : '0'))
        ;
    printf(" ");
    for (; i--; putchar(u & 1 << i ? '1' : '0'))
        ;
    printf("\n");
}
   
// Function to initialize a memory buffer with some machine code
void
initGuestText_Dummy(u_int32_t    *text,
                    vm_address_t  guestTextAddress,
                    vmm_regs32_t *ppcRegs32)
{
    // We will execute a stream of a few instructions in the virtual machine
    // through the Vmm (that is, us). I0 and I1 will load integer values into
    // registers GPR10 and GPR11. I3 will be an illegal instruction. I2 will
    // jump over I3 by unconditionally branching to I4, which will sum GPR10
    // and GPR11, placing their sum in GPR12.
    //
    // We will allow I5 to either be illegal, in which case control will
    // return to the Vmm, or, be a branch to itself: an infinite
    // loop. One Infinite Loop.
    //
    I_addi_d_form   *I0;
    I_addi_d_form   *I1;
    I_branch_i_form *I2;
    // I3 is illegal
    I_add_xo_form   *I4;
    I_branch_i_form *I5;
   
    // Guest will run the following instructions
    I0 = (I_addi_d_form   *)(text + 0);
    I1 = (I_addi_d_form   *)(text + 1);
    I2 = (I_branch_i_form *)(text + 2);
    text[3] = 0xdeadbeef; // illegal
    I4 = (I_add_xo_form   *)(text + 4);
   
    // Possibly overridden by an illegal instruction below
    I5 = (I_branch_i_form *)(text + 5);
   
    // Use an illegal instruction to be the last inserted instruction (I5)
    // in the guest's instruction stream
    text[5] = 0xfeedface;
   
    // Fill the instruction templates
   
    // addi r10,0,4     ; I0
    I0->OP = 14;
    I0->RT = 10;
    I0->RA = 0;
    I0->SI = 4; // load the value '4' in r10
   
    // addi r11,0,5     ; I1
    I1->OP = 14;
    I1->RT = 11;
    I1->RA = 0;
    I1->SI = 5; // load the value '5' in r11
   
    // ba               ; I2
    // We want to branch to the absolute address of the 5th instruction,
    // where the first instruction is at guestTextAddress. Note the shifting.
    //
    I2->OP = 18;
    I2->LI = (guestTextAddress + (4 * 4)) >> 2;
    I2->AA = 1;
    I2->LK = 0;
   
    // I3 is illegal; already populated in the stream
   
    // add  r12,r10,r11 ; I4
    I4->OP = 31;
    I4->RT = 12;
    I4->RA = 10;
    I4->RB = 11;
    I4->OE = 0;
    I4->XO = 266;
    I4->Rc = 0;
   
    // I5 is illegal or an infinite loop; already populated in the stream
   
    Printf("Fabricated instructions for executing "
           "in the guest virtual machine\n");
}
   
// Function to initialize a memory buffer with some machine code
void
initGuestText_Factorial(u_int32_t    *text,
                        vm_address_t  guestTextAddress,
                        vmm_regs32_t *ppcRegs32)
{
    // Machine code for the following function:
    //
    // int 
    // factorial(int n)
    // {
    //     if (n <= 0)
    //         return 1;
    //     else
    //         return n * factorial(n - 1);
    // }
    //
    // You can obtain this from the function's C source using a command-line
    // sequence like the following:
    //
    // $ gcc -static -c factorial.c
    // $ otool -tX factorial.o
    // ...
    //
    u_int32_t factorial_ppc32[] = {
        0x7c0802a6, 0xbfc1fff8, 0x90010008, 0x9421ffa0,
        0x7c3e0b78, 0x907e0078, 0x801e0078, 0x2f800000,
        0x419d0010, 0x38000001, 0x901e0040, 0x48000024,
        0x805e0078, 0x3802ffff, 0x7c030378, 0x4bffffc5,
        0x7c621b78, 0x801e0078, 0x7c0201d6, 0x901e0040,
        0x807e0040, 0x80210000, 0x80010008, 0x7c0803a6,
        0xbbc1fff8, 0x4e800020,
    };
   
    memcpy(text, factorial_ppc32, sizeof(factorial_ppc32)/sizeof(u_int8_t));
   
    // This demo takes an argument in GPR3: the number whose factorial is to
    // be computed. The result is returned in GPR3.
    //
    ppcRegs32->ppcGPRs[3] = 10; // factorial(10)
   
    // Set the LR to the end of the text in the guest's virtual address space.
    // Our demo will only use the LR for returning to the Vmm by placing an
    // illegal instruction's address in it.
    //
    ppcRegs32->ppcLR = guestTextAddress + vm_page_size - 4;
   
    Printf("Injected factorial instructions for executing "
           "in the guest virtual machine\n");
}
   
// Some modularity... these are the demos our program supports
typedef void (* initGuestText_Func)(u_int32_t *, vm_address_t, vmm_regs32_t *);
typedef struct {
    const char         *name;
    initGuestText_Func  textfiller;
} Demo;
   
Demo SupportedDemos[] = {
    {
        "executes a few hand-crafted instructions in a VM",
        initGuestText_Dummy,
    },
    {
        "executes a recursive factorial function in a VM",
        initGuestText_Factorial,
    },
};
#define MAX_DEMO_ID (sizeof(SupportedDemos)/sizeof(Demo))
   
static int demo_id = -1;
   
void
usage(int argc, char **argv)
{
    int i;
   
    if (argc != 2)
        goto OUT;
   
    demo_id = atoi(argv[1]);
    if ((demo_id >= 0) && (demo_id < MAX_DEMO_ID))
        return;
   
OUT:
    fprintf(stderr, "usage: %s <demo ID>\nSupported demos:\n"
            "  ID\tDescription\n", PROGNAME);
    for (i = 0; i < MAX_DEMO_ID; i++)
        fprintf(stderr, "  %d\t%s\n", i, SupportedDemos[i].name);
   
    exit(1);
}
   
int
main(int argc, char **argv)
{
    int i, j;
   
    kern_return_t       kr;
    mach_port_t         myTask;
    unsigned long      *return_params32;
    vmm_features_t      features;
    vmm_regs32_t       *ppcRegs32;
    vmm_version_t       version;
    vmm_thread_index_t  vmmIndex;             // The VM's index
    vm_address_t        vmmUStatePage = 0;    // Page for VM's user state
    vmm_state_page_t   *vmmUState;            // It's a vmm_comm_page_t too
    vm_address_t        guestTextPage = 0;    // Page for guest's text
    vm_address_t        guestStackPage = 0;   // Page for guest's stack
    vm_address_t        guestTextAddress = 0;
    vm_address_t        guestStackAddress = 0;
   
    my_vmm_dispatch = (vmm_dispatch_func_t)vmm_dispatch;
   
    // Ensure that the user chose a demo
    usage(argc, argv);
   
    // Get Vmm version implemented by this kernel
    version = my_vmm_dispatch(kVmmGetVersion);
    Printf("Mac OS X virtual machine monitor (version %lu.%lu)\n",
           (version >> 16), (version & 0xFFFF));
   
    // Get features supported by this Vmm implementation
    features = my_vmm_dispatch(kVmmvGetFeatures);
    Printf("Vmm features:\n");
    for (i = 0; VmmFeatures[i].mask != -1; i++)
        printf("  %-20s = %s\n", VmmFeatures[i].name,
               (features & VmmFeatures[i].mask) ?  "Yes" : "No");
   
    Printf("Page size is %u bytes\n", vm_page_size);
   
    myTask = mach_task_self(); // to save some characters (sure)
   
    // Allocate chunks of page-sized page-aligned memory
   
    // VM user state
    kr = vm_allocate(myTask, &vmmUStatePage, vm_page_size, VM_FLAGS_ANYWHERE);
    OUT_ON_MACH_ERROR("vm_allocate", kr);
    Printf("Allocated page-aligned memory for virtual machine user state\n");
    vmmUState = (vmm_state_page_t *)vmmUStatePage;
   
    // Guest's text
    kr = vm_allocate(myTask, &guestTextPage, vm_page_size, VM_FLAGS_ANYWHERE);
    OUT_ON_MACH_ERROR("vm_allocate", kr);
    Printf("Allocated page-aligned memory for guest's " "text\n");
   
    // Guest's stack
    kr = vm_allocate(myTask, &guestStackPage, vm_page_size, VM_FLAGS_ANYWHERE);
    OUT_ON_MACH_ERROR("vm_allocate", kr);
    Printf("Allocated page-aligned memory for guest's stack\n");
   
    // We will lay out the text and stack pages adjacent to one another in
    // the guest's virtual address space.
    //
    // Virtual addresses increase -->
    // 0              4K             8K             12K
    // +--------------------------------------------+
    // | __PAGEZERO   |  GUEST_TEXT  | GUEST_STACK  |
    // +--------------------------------------------+
    //
    // We put the text page at virtual offset vm_page_size and the stack
    // page at virtual offset (2 * vm_page_size).
    //
   
    guestTextAddress = vm_page_size;
    guestStackAddress = 2 * vm_page_size;
   
    // Initialize a new virtual machine context
    kr = my_vmm_dispatch(kVmmInitContext, version, vmmUState);
    OUT_ON_MACH_ERROR("vmm_init_context", kr);
   
    // Fetch the index returned by vmm_init_context()
    vmmIndex = vmmUState->thread_index;
    Printf("New virtual machine context initialized, index = %lu\n", vmmIndex);
   
    // Set a convenience pointer to the VM's registers
    ppcRegs32 = &(vmmUState->vmm_proc_state.ppcRegs.ppcRegs32);
   
    // Set the program counter to the beginning of the text in the guest's
    // virtual address space
    ppcRegs32->ppcPC = guestTextAddress;
    Printf("Guest virtual machine PC set to %p\n", (void *)guestTextAddress);
   
    // Set the stack pointer (GPR1), taking the Red Zone into account
    #define PAGE2SP(x) ((void *)((x) + vm_page_size - C_RED_ZONE))
    ppcRegs32->ppcGPRs[1] = (u_int32_t)PAGE2SP(guestStackAddress); // 32-bit
    Printf("Guest virtual machine SP set to %p\n", PAGE2SP(guestStackAddress));
   
    // Map the stack page into the guest's address space
    kr = my_vmm_dispatch(kVmmMapPage, vmmIndex, guestStackPage,
                      guestStackAddress, VM_PROT_ALL);
    Printf("Mapping guest stack page\n");
   
    // Call the chosen demo's instruction populator
    (SupportedDemos[demo_id].textfiller)((u_int32_t *)guestTextPage,
                                         guestTextAddress, ppcRegs32);
   
    // Finally, map the text page into the guest's address space, and set the
    // VM running
    //
    Printf("Mapping guest text page and switching to guest virtual machine\n");
    kr = my_vmm_dispatch(kVmmMapExecute, vmmIndex, guestTextPage,
                      guestTextAddress, VM_PROT_ALL);
   
    // Our demo ensures that the last instruction in the guest's text is
    // either an infinite loop or illegal. The monitor will "hang" in the case
    // of an infinite loop. It will have to be interupted (^C) to gain control.
    // In the case of an illegal instruction, the monitor will gain control at
    // this point, and the following code will be executed. Depending on the
    // exact illegal instruction, Mach's error messages may be different.
    //
    if (kr != KERN_SUCCESS)
        mach_error("*** vmm_map_execute32:", kr);
   
    Printf("Returned to vmm\n");
    Printf("Processor state:\n");
   
    printf("  Distance from origin = %lu instructions\n",
           (ppcRegs32->ppcPC - vm_page_size) >> 2);
   
    printf("  PC                   = %p (%lu)\n",
           (void *)ppcRegs32->ppcPC, ppcRegs32->ppcPC);
   
    printf("  Instruction at PC    = %#08x\n",
        ((u_int32_t *)(guestTextPage))[(ppcRegs32->ppcPC - vm_page_size) >> 2]);
   
    printf("  CR                   = %#08lx\n"
           "                         ", ppcRegs32->ppcCR);
    prbits32(ppcRegs32->ppcCR);
   
    printf("  LR                   = %#08lx (%lu)\n",
           ppcRegs32->ppcLR, ppcRegs32->ppcLR);
   
    printf("  MSR                  = %#08lx\n"
           "                         ", ppcRegs32->ppcMSR);
    prbits32(ppcRegs32->ppcMSR);
   
    printf("  return_code          = %#08lx (%lu)\n",
           vmmUState->return_code, vmmUState->return_code);
   
    return_params32 = vmmUState->vmmRet.vmmrp32.return_params;
   
    for (i = 0; i < 4; i++)
        printf("  return_params32[%d]   = 0x%08lx (%lu)\n", i,
               return_params32[i], return_params32[i]);
   
    printf("  GPRs:\n");
    for (j = 0; j < 16; j++) {
        printf("  ");
        for (i = 0; i < 2; i++) {
            printf("r%-2d = %#08lx ", j * 2 + i,
                   ppcRegs32->ppcGPRs[j * 2 + i]);
        }
        printf("\n");
    }
   
    // Tear down the virtual machine ... that's all for now
    kr = my_vmm_dispatch(kVmmTearDownContext, vmmIndex);
    OUT_ON_MACH_ERROR("vmm_init_context", kr);
    Printf("Virtual machine context torn down\n");
   
out:
    if (vmmUStatePage)
        (void)vm_deallocate(myTask, vmmUStatePage, vm_page_size);
   
    if (guestTextPage)
        (void)vm_deallocate(myTask, guestTextPage, vm_page_size);
   
    if (guestStackPage)
        (void)vm_deallocate(myTask, guestStackPage, vm_page_size);
   
    exit(kr);
}
