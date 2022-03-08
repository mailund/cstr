
#include "fastq.h"

#include <stdlib.h>
#include <string.h>

#define INIT_BUF_LEN 256
void init_fastq_iter(
    struct fastq_iter *iter,
    FILE *file
) {
    iter->file = file;
    iter->name = cstr_alloc_sslice_buf(0, INIT_BUF_LEN);
    iter->seq = cstr_alloc_sslice_buf(0, INIT_BUF_LEN);
}

static void copy_line(cstr_sslice_buf **buf, FILE *file)
{
    (*buf)->slice.len = 0; // Reset buffer before we copy
    for (char c = fgetc(file); (c != EOF) && (c != '\n'); c = fgetc(file))
    {
        cstr_append_sslice_buf(buf, c);
    }
    // Append 0 so we can use the buffer as a C string as well.
    cstr_append_sslice_buf(buf, 0);
}

bool next_fastq_record(
    struct fastq_iter *iter,
    struct fastq_record *record
) {
    FILE *file = iter->file;
    if (feof(file))
    {
        return false;
    }
    copy_line(&iter->name, file);
    copy_line(&iter->seq, file);
    record->name = CSTR_SUFFIX(CSTR_SLICE_CONST_CAST(iter->name->slice), 1); // skip @
    record->seq = CSTR_SLICE_CONST_CAST(iter->seq->slice);
    return true;
}

void dealloc_fastq_iter(
    struct fastq_iter *iter
) {
    free(iter->name);
    free(iter->seq);
}
