#!/bin/sh
# sched_tick.sh
   
SCHED_TICK_ADDR="0x`nm /mach_kernel | grep -w _sched_tick | awk '{print $1}'`"
if [ "$SCHED_TICK_ADDR" == "0x" ]
then
    echo "address of _sched_tick not found in /mach_kernel"
    exit 1
fi
   
dd if=/dev/kmem bs=1 count=4 iseek=$SCHED_TICK_ADDR of=/dev/stdout | hexdump -d
sleep 10
dd if=/dev/kmem bs=1 count=4 iseek=$SCHED_TICK_ADDR of=/dev/stdout | hexdump -d
   
exit 0
