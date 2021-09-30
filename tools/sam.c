
#include "sam.h"
#include <stdint.h>

void print_sam_line(FILE *file, const char *qname, const char *rname,
                    uint32_t pos, const char *cigar, const uint8_t *seq,
                    const char *qual) {
    fprintf(file, "%s\t0\t%s\t%u\t0\t%s\t*\t0\t0\t%s\t%s\n", qname, rname, pos,
            cigar, seq, qual);
}
