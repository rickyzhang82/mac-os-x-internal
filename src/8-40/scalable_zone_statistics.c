// scalable_zone_statistics.c
   
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <malloc/malloc.h>
#include "nomalloc_printf.h"
   
#define PROGNAME "scalable_zone_statistics"
   
enum { TINY_REGION, SMALL_REGION, LARGE_REGION, HUGE_REGION };
   
extern boolean_t scalable_zone_statistics(malloc_zone_t *,
                                          malloc_statistics_t *, unsigned);
   
void
print_statistics(const char *label, malloc_statistics_t *stats)
{
    nomalloc_printf("%8s%16u%16lu%16lu", label, stats->blocks_in_use,
                     stats->size_in_use, stats->max_size_in_use);
    if (stats->size_allocated != -1)
        nomalloc_printf("%16lu\n", stats->size_allocated);
    else
        printf("%16s\n", "-");
}
   
int
main(int argc, char **argv)
{
    void                *ptr = NULL;
    unsigned long long   size;
    malloc_statistics_t  stats;
    malloc_zone_t       *zone;
   
    if (!(zone = malloc_default_zone()))
        exit(1);
   
    if (argc == 2) {
        if ((size = strtoull(argv[1], NULL, 10)) == ULLONG_MAX) {
            fprintf(stderr, "invalid allocation size (%s)\n", argv[1]);
            exit(1);
        }
   
        if ((ptr = malloc((size_t)size)) == NULL) {
            perror("malloc");
            exit(1);
        }
    }
   
    nomalloc_printf("%8s%16s%16s%16s%16s\n", "Region", "Blocks in use",
                     "Size in use", "Max size in use", "Size allocated");
    scalable_zone_statistics(zone, &stats, TINY_REGION);
    print_statistics("tiny", &stats);
    scalable_zone_statistics(zone, &stats, SMALL_REGION);
    print_statistics("small", &stats);
    scalable_zone_statistics(zone, &stats, LARGE_REGION);
    stats.size_allocated = -1;
    print_statistics("large", &stats);
    scalable_zone_statistics(zone, &stats, HUGE_REGION);
    stats.size_allocated = -1;
    print_statistics("huge", &stats);
   
    if (ptr)
        free(ptr);
   
    exit(0);
}
