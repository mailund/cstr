
#ifndef FASTA_H
#define FASTA_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <cstr.h>

// opaque structure.
struct fasta_records;

struct fasta_record
{
    const char *name;
    cstr_const_sslice seq;
    struct fasta_record *const next;
};

struct fasta_records *load_fasta_records(const char *fname);
void free_fasta_records(struct fasta_records *recs);
struct fasta_record *fasta_records(struct fasta_records *recs);

#endif
