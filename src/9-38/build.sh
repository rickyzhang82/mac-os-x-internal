mig  $(xcrun --sdk macosx --show-sdk-path)/usr/include/mach/exc.defs

mig  $(xcrun --sdk macosx --show-sdk-path)/usr/include/mach/mach_exc.defs

CFLAGS="-g -Wall $(pkg-config --cflags capstone)"
LDFLAGS="$(pkg-config --libs capstone)"

gcc $CFLAGS exception.c mach_excServer.c  $LDFLAGS  -o exception.o 
