
#ifndef SAM_H
#define SAM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*
 These functions provide some rudimentary SAM output. Most SAM flags
 are not provided as this is just an algorithmic exercise.
 */

void print_sam_line(FILE *file, const char *qname, const char *rname,
                    int pos, const char *cigar, const char *seq,
                    const char *qual);

#endif
