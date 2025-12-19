#include "sort.h"
#include "bf.h"
#include "chunk.h"
#include "hp_file.h"
#include "merge.h"
#include "record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool shouldSwap(Record *rec1, Record *rec2) {
    return false;
}

void sort_FileInChunks(int file_desc, int numBlocksInChunk) {
}

void sort_Chunk(CHUNK *chunk) {
}
