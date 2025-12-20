#include "chunk.h"
#include <merge.h>
#include <stdio.h>

CHUNK_Iterator CHUNK_CreateIterator(int fileDesc, int blocksInChunk) {
    CHUNK_Iterator iterator;

    iterator.file_desc = fileDesc;

    /* Το block 0 περιέχει metadata, άρα τα chunks ξεκινούν από block 1 */
    iterator.firstBlockID = 1;

    /* Δεν έχει επιστραφεί ακόμα κανένα chunk */
    iterator.lastBlockID = 0;

    /* Σταθερό μέγεθος chunk (σε blocks) */
    iterator.blocksInChunk = blocksInChunk;

    return iterator;
}

int CHUNK_GetNext(CHUNK_Iterator *iterator, CHUNK *chunk) {
    if (iterator == NULL || chunk == NULL)
        return -1;

    // Πάρε το id του τελευταίου block στο αρχείο
    int lastBlockInFile = HP_GetIdOfLastBlock(iterator->file_desc);
    if (lastBlockInFile < 1) // αν δεν υπάρχουν data blocks
        return -1;

    // Υπολόγισε την αρχή του επόμενου chunk
    int startBlock = iterator->lastBlockID + 1;

    if (startBlock > lastBlockInFile)
        return -1; // δεν υπάρχουν άλλα chunks

    // Υπολόγισε το τέλος του chunk
    int endBlock = startBlock + iterator->blocksInChunk - 1;
    if (endBlock > lastBlockInFile)
        endBlock = lastBlockInFile;

    // Γέμισε τα πεδία του chunk
    chunk->file_desc = iterator->file_desc;
    chunk->from_BlockId = startBlock;
    chunk->to_BlockId = endBlock;
    chunk->blocksInChunk = endBlock - startBlock + 1;

    // Υπολόγισε τον συνολικό αριθμό records στο chunk
    int totalRecords = 0;
    for (int b = startBlock; b <= endBlock; b++) {
        int recordsInBlock = HP_GetRecordCounter(iterator->file_desc, b);
        totalRecords += recordsInBlock;
    }
    chunk->recordsInChunk = totalRecords;

    // Ενημέρωσε τον iterator για το τελευταίο block που επέστρεψε
    iterator->lastBlockID = endBlock;

    return 0; // επιτυχία
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
    CHUNK_RecordIterator it;
    it.chunk = *chunk;                       // Αντιγραφή του chunk στο iterator
    it.currentBlockId = chunk->from_BlockId; // Ξεκινάμε από το πρώτο block του chunk
    it.cursor = 0;                           // Ξεκινάμε από το πρώτο record του block
    return it;
}

int CHUNK_GetNextRecord(CHUNK_RecordIterator *iterator, Record *record) {
    // Έλεγχος αν φτάσαμε πέρα από το τέλος του chunk
    if (iterator->currentBlockId > iterator->chunk.to_BlockId) {
        return -1; // τέλος
    }

    int recCount = HP_GetRecordCounter(iterator->chunk.file_desc, iterator->currentBlockId);
    if (recCount == -1)
        return -1; // σφάλμα

    // Αν ο cursor έχει φτάσει πέρα από τα records του τρέχοντος block, προχώρησε στο επόμενο block
    while (iterator->cursor >= recCount) {
        if (iterator->currentBlockId == iterator->chunk.to_BlockId) {
            return -1; // δεν υπάρχει επόμενο block, τέλος chunk
        }
        // επόμενο block
        iterator->currentBlockId++;
        iterator->cursor = 0;
        recCount = HP_GetRecordCounter(iterator->chunk.file_desc, iterator->currentBlockId);
        if (recCount == -1)
            return -1; // σφάλμα
    }

    // Ανάκτηση του record
    if (HP_GetRecord(iterator->chunk.file_desc, iterator->currentBlockId, iterator->cursor, record) == -1) {
        return -1; // σφάλμα
    }

    // Ενημέρωση cursor για την επόμενη κλήση
    iterator->cursor++;

    // Καλή πρακτική: ξε-κλείδωμα του block μετά την ανάγνωση
    HP_Unpin(iterator->chunk.file_desc, iterator->currentBlockId);

    return 0; // επιτυχία
}
