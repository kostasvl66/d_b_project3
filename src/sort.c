#include "sort.h"
#include "bf.h"
#include "chunk.h"
#include "hp_file.h"
#include "merge.h"
#include "record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool shouldSwap(Record* rec1, Record* rec2) {
    int cmp;

    // Πρώτα σύγκριση ως προς name
    cmp = strcmp(rec1->name, rec2->name);
    if (cmp > 0) {
        return true;   // rec1 > rec2
    }
    if (cmp < 0) {
        return false;  // rec1 < rec2
    }

    // Αν τα name είναι ίσα, σύγκριση ως προς surname
    cmp = strcmp(rec1->surname, rec2->surname);
    if (cmp > 0) {
        return true;   // rec1 > rec2
    }

    return false;      // rec1 <= rec2
}

void sort_FileInChunks(int file_desc, int numBlocksInChunk) {
}

/* Comparator wrapper for qsort */
static int recordComparator(const void *a, const void *b) {
    Record *rec1 = (Record *)a;
    Record *rec2 = (Record *)b;

    if (shouldSwap(rec1, rec2))
        return 1;   // rec1 > rec2
    if (shouldSwap(rec2, rec1))
        return -1;  // rec1 < rec2

    return 0;       // equal
}

void sort_Chunk(CHUNK* chunk) {
    if (chunk == NULL || chunk->recordsInChunk <= 1)
        return;

    int n = chunk->recordsInChunk;

    /* Allocate temporary array for records */
    Record *records = malloc(n * sizeof(Record));
    if (records == NULL) {
        // out of memory — nothing we can safely do
        return;
    }

    /* Read all records from the chunk */
    for (int i = 0; i < n; i++) {
        if (CHUNK_GetIthRecord(chunk, i, &records[i]) != 0) {
            free(records);
            return;
        }
    }

    /* Sort in memory */
    qsort(records, n, sizeof(Record), recordComparator);

    /* Write sorted records back in-place */
    for (int i = 0; i < n; i++) {
        if (CHUNK_UpdateIthRecord(chunk, i, records[i]) != 0) {
            free(records);
            return;
        }
    }

    free(records);
}