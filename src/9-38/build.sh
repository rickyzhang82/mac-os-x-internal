mig  $(xcrun --sdk macosx --show-sdk-path)/usr/include/mach/exc.defs

mig  $(xcrun --sdk macosx --show-sdk-path)/usr/include/mach/mach_exc.defs

gcc -Wall -g exception.c mach_excServer.c  -o exception.o 
