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

int CHUNK_GetIthRecordInChunk(CHUNK *chunk, int i, Record *record) {
}

int CHUNK_UpdateIthRecord(CHUNK *chunk, int i, Record record) {
}

void CHUNK_Print(CHUNK chunk) {
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
