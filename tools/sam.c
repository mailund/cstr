
#include "sam.h"
#include <stdint.h>

void print_sam_line(FILE *file, const char *qname, const char *rname,
                    int pos, const char *cigar, const uint8_t *seq,
                    const char *qual) {
    fprintf(file, "%s\t0\t%s\t%d\t0\t%s\t*\t0\t0\t%s\t%s\n", qname, rname, pos,
            cigar, (const char *)seq, qual);
}
