#include <cstr.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstr.h>

#include "fasta.h"
#include "fastq.h"
#include "sam.h"

typedef cstr_exact_matcher *(*algorithm_fn)(cstr_const_sslice, cstr_const_sslice);

struct alg_choice {
    const char *name;
    algorithm_fn algorithm;
};

struct alg_choice algorithms[] = {
    {"naive", cstr_naive_matcher},
    {"ba", cstr_ba_matcher},
    {"kmp", cstr_kmp_matcher},
};

int main(int argc, const char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s algo fasta fastq\n", argv[0]);
        return 1;
    }

    algorithm_fn algo = 0;
    for (int i = 0; i < sizeof(algorithms) / sizeof(algorithms[0]); i++) {
        if (strcmp(argv[1], algorithms[i].name) == 0) {
            algo = algorithms[i].algorithm;
            break;
        }
    }
    if (!algo) {
        printf("Unknown algorithm: %s\n", argv[1]);
        return 1;
    }

    struct fasta_records *chromosomes = load_fasta_records(argv[2]);

    struct fastq_iter fqiter;
    struct fastq_record fqrec;
    cstr_exact_matcher *matcher = 0;
    char cigarbuf[2048];

    FILE *fq = fopen(argv[3], "r");
    init_fastq_iter(&fqiter, fq);

    while (next_fastq_record(&fqiter, &fqrec)) {
        sprintf(cigarbuf, "%lldM", fqrec.seq.len);
        for (struct fasta_record *farec = fasta_records(chromosomes); farec; farec = farec->next) {
            matcher = algo(farec->seq, fqrec.seq);
            
            for (long long pos = cstr_exact_next_match(matcher); pos != -1;
                 pos = cstr_exact_next_match(matcher)) {
                print_sam_line(stdout, (const char *)fqrec.name.buf, farec->name, pos, cigarbuf, (const char *)fqrec.seq.buf);
            }
            cstr_free_exact_matcher(matcher);
        }
    }
    
    dealloc_fastq_iter(&fqiter);
    fclose(fq);

    return 0;
}
