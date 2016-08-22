// pthread_atfork.c
   
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
   
// handler to be called before fork()
void
prepare(void)
{
    printf("prepare\n");
}
   
// handler to be called after fork() in the parent
void
parent(void)
{
    printf("parent\n");
}
   
// handler to be called after fork() in the child
void
child(void)
{
    printf("child\n");
}
   
int
main(void)
{
    (void)pthread_atfork(prepare, parent, child);
    (void)fork();
    _exit(0);
}
