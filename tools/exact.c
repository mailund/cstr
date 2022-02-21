#include <cstr.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstr.h>

#include "fasta.h"
#include "fastq.h"
#include "sam.h"

typedef struct cstr_exact_matcher *(*algorithm_fn)(cstr_sslice, cstr_sslice);

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

    struct fasta_iter faiter;
    struct fasta_record farec;
    struct fastq_iter fqiter;
    struct fastq_record fqrec;
    struct cstr_exact_matcher *matcher = 0;
    char cigarbuf[2048];

    FILE *fq = fopen(argv[3], "r");
    init_fastq_iter(&fqiter, fq);

    while (next_fastq_record(&fqiter, &fqrec)) {
        sprintf(cigarbuf, "%luM", strlen((const char *)fqrec.sequence));
        init_fasta_iter(&faiter, chromosomes);
        while (next_fasta_record(&faiter, &farec)) {
            matcher =
                algo(CSTR_SLICE(farec.seq, farec.seq_len),
                     CSTR_SLICE_STRING((char *)fqrec.sequence));
            
            for (int pos = cstr_exact_next_match(matcher); pos != -1;
                 pos = cstr_exact_next_match(matcher)) {
                print_sam_line(stdout, fqrec.name, farec.name, pos + 1,
                               cigarbuf, fqrec.sequence, fqrec.quality);
            }
            cstr_free_exact_matcher(matcher);
        }
        dealloc_fasta_iter(&faiter);
    }
    dealloc_fastq_iter(&fqiter);
    fclose(fq);

    return 0;
}
