// kvm_hagfish.c
   
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <kvm.h>
   
#define TARGET_ADDRESS (u_long)0x5000
#define TARGET_NBYTES  (size_t)7
#define PROGNAME        "kvm_hagfish"
   
int
main(void)
{
    kvm_t *kd;
    char   buf[8] = { '\0' };
   
    kd = kvm_open(NULL,      // kernel executable; use default
                  NULL,      // kernel memory device; use default
                  NULL,      // swap device; use default
                  O_RDONLY,  // flags
                  PROGNAME); // error prefix string
    if (!kd)
        exit(1);
   
   
    if (kvm_read(kd, TARGET_ADDRESS, buf, TARGET_NBYTES) != TARGET_NBYTES)
        perror("kvm_read");
    else
        printf("%s\n", buf);
   
    kvm_close(kd);
   
    exit(0);
}
