#!/bin/sh
#
# readksym.sh
   
PROGNAME=readksym
   
if [ $# -lt 2 ]
then
    echo "usage: $PROGNAME <symbol>  <bytes to read> [hexdump option|-raw]"
    echo "       $PROGNAME <address> <bytes to read> [hexdump option|-raw]"
    exit 1
fi
   
SYMBOL=$1                 # first argument is a kernel symbol
SYMBOL_ADDR=$1            # or a kernel address in hexadecimal
IS_HEX=${SYMBOL_ADDR:0:2} # get the first two characters
NBYTES=$2                 # second argument is the number of bytes to read
HEXDUMP_OPTION=${3:--x}   # by default, we pass '-x' to hexdump
RAW="no"                  # by default, we don't print memory as "raw"
   
if [ ${HEXDUMP_OPTION:0:2} == "-r" ]
then
    RAW="yes" # raw... don't pipe through hexdump -- print as is
fi
   
KERN_SYMFILE=`sysctl -n kern.symfile | tr '\\' '/'` # typically /mach.sym
if [ X"$KERN_SYMFILE" == "X" ]
then
    echo "failed to determine the kernel symbol file's name"
    exit 1
fi
   
if [ "$IS_HEX" != "0x" ]
then
    # use nm to determine the address of the kernel symbol
    SYMBOL_ADDR="0x`nm $KERN_SYMFILE | grep -w $SYMBOL | awk '{print $1}'`"
fi
   
if [ "$SYMBOL_ADDR" == "0x" ] # at this point, we should have an address
then
    echo "address of $SYMBOL not found in $KERN_SYMFILE"
    exit 1
fi
   
if [ ${HEXDUMP_OPTION:0:2} == "-r" ] # raw... no hexdump
then
    dd if=/dev/kmem bs=1 count=$NBYTES iseek=$SYMBOL_ADDR of=/dev/stdout \
        2>/dev/null
else
    dd if=/dev/kmem bs=1 count=$NBYTES iseek=$SYMBOL_ADDR of=/dev/stdout \
        2>/dev/null | hexdump $HEXDUMP_OPTION
fi
   
exit 0
