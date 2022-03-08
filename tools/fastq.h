
#ifndef FASTQ_H
#define FASTQ_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <cstr.h>

struct fastq_iter {
    FILE *file;
    cstr_sslice_buf *name;
    cstr_sslice_buf *seq;
};
struct fastq_record {
    cstr_const_sslice name;
    cstr_const_sslice seq;
};

void init_fastq_iter(struct fastq_iter *iter, FILE *file);
bool next_fastq_record(struct fastq_iter *iter, struct fastq_record *record);
void dealloc_fastq_iter(struct fastq_iter *iter);


#endif
