#include "chunk.h"
#include <merge.h>
#include <stdio.h>

CHUNK_Iterator CHUNK_CreateIterator(int fileDesc, int blocksInChunk) {
}

int CHUNK_GetNext(CHUNK_Iterator *iterator, CHUNK *chunk) {
}

int CHUNK_GetIthRecordInChunk(CHUNK* chunk, int i, Record* record) {
    if (chunk == NULL || record == NULL) {
        return -1;
    }

    // Έλεγχος ορίων
    if (i < 0 || i >= chunk->recordsInChunk) {
        return -1;
    }

    int file_desc = chunk->file_desc;

    // Πόσα records χωράνε σε κάθε block
    int recordsPerBlock = HP_GetMaxRecordsInBlock(file_desc);
    if (recordsPerBlock <= 0) {
        return -1;
    }

    // Υπολογισμός block και cursor
    int blockOffset = i / recordsPerBlock;
    int cursor = i % recordsPerBlock;

    int blockId = chunk->from_BlockId + blockOffset;

    // Ασφάλεια: block εντός chunk
    if (blockId > chunk->to_BlockId) {
        return -1;
    }

    // Ανάκτηση record
    if (HP_GetRecord(file_desc, blockId, cursor, record) == -1) {
        return -1;
    }

    // ΥΠΟΧΡΕΩΤΙΚΟ unpin
    if (HP_Unpin(file_desc, blockId) == -1) {
        return -1;
    }

    return 0; // success
}

int CHUNK_UpdateIthRecord(CHUNK* chunk, int i, Record record) {
    if (chunk == NULL) {
        return -1;
    }

    // Boundary check
    if (i < 0 || i >= chunk->recordsInChunk) {
        return -1;
    }

    int remaining = i;

    // Traverse blocks of the chunk
    for (int blockId = chunk->from_BlockId;
         blockId <= chunk->to_BlockId;
         blockId++) {

        int recordsInBlock = HP_GetRecordCounter(chunk->file_desc, blockId);
        if (recordsInBlock < 0) {
            return -1;
        }

        if (remaining < recordsInBlock) {
            // Found the correct block and cursor
            int cursor = remaining;

            if (HP_UpdateRecord(chunk->file_desc, blockId, cursor, record) == -1) {
                return -1;
            }

            // Mandatory unpin
            if (HP_Unpin(chunk->file_desc, blockId) == -1) {
                return -1;
            }

            return 0; // success
        }

        remaining -= recordsInBlock;
    }

    // Should never reach here if recordsInChunk is correct
    return -1;
}

void CHUNK_Print(CHUNK chunk) {
    Record rec;
    int totalRecords = chunk.recordsInChunk;

    printf("Printing CHUNK from block %d to block %d (Total records: %d)\n",
           chunk.from_BlockId, chunk.to_BlockId, totalRecords);

    for (int i = 0; i < totalRecords; i++) {
        if (CHUNK_GetIthRecordInChunk(&chunk, i, &rec) == 0) {
            printRecord(rec);
        } else {
            fprintf(stderr, "Error reading record %d in chunk.\n", i);
        }
    }
}

CHUNK_RecordIterator CHUNK_CreateRecordIterator(CHUNK *chunk) {
}

int CHUNK_GetNextRecord(CHUNK_RecordIterator *iterator, Record *record) {
}
