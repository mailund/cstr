
#include "sam.h"
#include <stdint.h>

void print_sam_line(FILE *file, const char *qname, const char *rname,
                    int pos, const char *cigar, const char *seq)
{
    fprintf(file, "%s\t%s\t%d\t%s\t%s\n", qname, rname, pos + 1, cigar, (const char *)seq);
}
