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

void sort_Chunk(CHUNK *chunk) {
}
