#include <limits.h>
#include <merge.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Δομή για διατήρηση των τρεχόντων εγγραφών κάθε chunk
typedef struct {
    CHUNK_RecordIterator it; // iterator για το chunk
    Record currentRecord;    // τρέχον record
    int hasRecord;           // 1 αν υπάρχει έγκυρο record, 0 αν τελείωσε
} MergeEntry;

void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
    CHUNK_Iterator chunkIter = CHUNK_CreateIterator(input_FileDesc, chunkSize);
    CHUNK chunks[bWay];
    MergeEntry entries[bWay];
    int numChunks = 0;

    // Φόρτωσε τα πρώτα bWay chunks
    for (int i = 0; i < bWay; i++) {
        if (CHUNK_GetNext(&chunkIter, &chunks[i]) == 0) {
            entries[i].it = CHUNK_CreateRecordIterator(&chunks[i]);
            if (CHUNK_GetNextRecord(&entries[i].it, &entries[i].currentRecord) == 0) {
                entries[i].hasRecord = 1;
            } else {
                entries[i].hasRecord = 0;
            }
            numChunks++;
        } else {
            break;
        }
    }

    Record outputBuffer[HP_GetMaxRecordsInBlock(output_FileDesc)];
    int outputBufferCount = 0;
    int maxOutputRecords = HP_GetMaxRecordsInBlock(output_FileDesc);

    while (1) {
        int minIndex = -1;

        // Βρες το ελάχιστο record μεταξύ των chunks
        for (int i = 0; i < numChunks; i++) {
            if (entries[i].hasRecord) {
                if (minIndex == -1) {
                    minIndex = i;
                } else if (shouldSwap(&entries[minIndex].currentRecord, &entries[i].currentRecord)) {
                    // Το minIndex > i, άρα το i είναι μικρότερο
                    minIndex = i;
                }
            }
        }

        if (minIndex == -1) {
            // Όλα τα chunks εξαντλήθηκαν
            break;
        }

        // Πρόσθεσε το μικρότερο record στο output buffer
        outputBuffer[outputBufferCount++] = entries[minIndex].currentRecord;

        // Αν γεμίσει το output block, γράψτο στο output file
        if (outputBufferCount == maxOutputRecords) {
            for (int j = 0; j < outputBufferCount; j++) {
                HP_InsertEntry(output_FileDesc, outputBuffer[j]);
            }
            outputBufferCount = 0;
        }

        // Φόρτωσε το επόμενο record από το chunk που έδωσε το ελάχιστο
        if (CHUNK_GetNextRecord(&entries[minIndex].it, &entries[minIndex].currentRecord) == 0) {
            entries[minIndex].hasRecord = 1;
        } else {
            entries[minIndex].hasRecord = 0;
        }
    }

    // Γράψε τυχόν υπόλοιπα records στο output file
    for (int j = 0; j < outputBufferCount; j++) {
        HP_InsertEntry(output_FileDesc, outputBuffer[j]);
    }
}
