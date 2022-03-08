
#include "fasta.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char *load_file(const char *fname)
{
    FILE *f = fopen(fname, "rb");
    if (!f)
    {
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET); // rewinding

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);

    string[fsize] = 0;
    fclose(f);

    return string;
}

struct fasta_records
{
    char *buffer;
    struct fasta_record *recs;
};

struct packing
{
    char *front;
    char *pack;
};

static void pack_name(struct packing *pack)
{
    while (true)
    {
        // skip record start and space
        while (*pack->front == '>' || *pack->front == ' ' ||
               *pack->front == '\t')
        {
            pack->front++;
        }
        if (*pack->front == '\0' || *pack->front == '\n')
            break; // end of name or end of file (broken record if end of file)
        (*pack->pack++) = (*pack->front++);
    }

    (*pack->pack++) = '\0';

    // are we done or is there a new front?
    if (*pack->front == '\0')
    {
        pack->front = 0;
    }
    else
    {
        pack->front++;
    }
}

static void pack_seq(struct packing *pack)
{
    assert(pack->front);
    while (true)
    {
        // skip space
        while (*pack->front && isspace(*pack->front))
            pack->front++;
        if (*pack->front == '\0' || *pack->front == '>')
            break; // next header or end of file
        (*pack->pack++) = (*pack->front++);
    }

    (*pack->pack++) = '\0';

    // are we done or is there a new front?
    if (*pack->front == '\0')
    {
        pack->front = 0;
    }
    else
    {
        pack->front++;
    }
}

static struct fasta_record *alloc_rec(const char *name, cstr_const_sslice seq, struct fasta_record *next)
{
    struct fasta_record *rec = cstr_malloc(sizeof *rec);
    memcpy(rec, &(struct fasta_record){ .name = name, .seq = seq, .next = next }, sizeof *rec);
    return rec;
}

struct fasta_records *load_fasta_records(const char *fname)
{
    // stuff to deallocated in case of errors
    struct fasta_records *rec = 0;

    char *string = load_file(fname);
    if (!string)
    {
        // This is the first place we allocate a resource
        // and it wasn't allocated, so we just return rather
        // than jump to fail.
        return 0;
    }

    rec = cstr_malloc(sizeof *rec);
    rec->buffer = string;
    rec->recs = 0;

    char *name;
    char *seq;
    struct packing pack = {rec->buffer, rec->buffer};
    while (pack.front)
    {
        name = (char *)pack.pack;
        pack_name(&pack);

        if (pack.front == 0)
        {
            goto fail;
        }

        seq = pack.pack;
        pack_seq(&pack);

        rec->recs = alloc_rec(name, CSTR_SLICE((const uint8_t *)seq, pack.pack - seq - 1), rec->recs);
    }

    return rec;

fail:
    abort(); // Rather crude error handling, but I don't care right now
    return 0;
}

void free_fasta_records(struct fasta_records *recs)
{
    free(recs->buffer);
    struct fasta_record *rec = recs->recs, *next;
    while (rec)
    {
        next = (struct fasta_record *)rec->next;
        free(rec);
        rec = next;
    }
    free(recs);
}

struct fasta_record *fasta_records(struct fasta_records *recs)
{
    return recs->recs;
}
