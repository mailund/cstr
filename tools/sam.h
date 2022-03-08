
#ifndef SAM_H
#define SAM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

void print_sam_line(FILE *file, const char *qname, const char *rname,
                    int pos, const char *cigar, const char *seq);

#endif
