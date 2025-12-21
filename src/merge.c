#include <limits.h>
#include <merge.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// // Δομή για διατήρηση των τρεχόντων εγγραφών κάθε chunk
// typedef struct {
//     CHUNK_RecordIterator it; // iterator για το chunk
//     Record currentRecord;    // τρέχον record
//     int hasRecord;           // 1 αν υπάρχει έγκυρο record, 0 αν τελείωσε
// } MergeEntry;
//
// void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc) {
//     CHUNK_Iterator chunkIter = CHUNK_CreateIterator(input_FileDesc, chunkSize);
//     CHUNK chunks[bWay];
//     MergeEntry entries[bWay];
//     int numChunks = 0;
//
//     // Φόρτωσε τα πρώτα bWay chunks
//     for (int i = 0; i < bWay; i++) {
//         if (CHUNK_GetNext(&chunkIter, &chunks[i]) == 0) {
//             entries[i].it = CHUNK_CreateRecordIterator(&chunks[i]);
//             if (CHUNK_GetNextRecord(&entries[i].it, &entries[i].currentRecord) == 0) {
//                 entries[i].hasRecord = 1;
//             } else {
//                 entries[i].hasRecord = 0;
//             }
//             numChunks++;
//         } else {
//             break;
//         }
//     }
//
//     Record outputBuffer[HP_GetMaxRecordsInBlock(output_FileDesc)];
//     int outputBufferCount = 0;
//     int maxOutputRecords = HP_GetMaxRecordsInBlock(output_FileDesc);
//
//     while (1) {
//         int minIndex = -1;
//
//         // Βρες το ελάχιστο record μεταξύ των chunks
//         for (int i = 0; i < numChunks; i++) {
//             if (entries[i].hasRecord) {
//                 if (minIndex == -1) {
//                     minIndex = i;
//                 } else if (shouldSwap(&entries[minIndex].currentRecord, &entries[i].currentRecord)) {
//                     // Το minIndex > i, άρα το i είναι μικρότερο
//                     minIndex = i;
//                 }
//             }
//         }
//
//         if (minIndex == -1) {
//             // Όλα τα chunks εξαντλήθηκαν
//             break;
//         }
//
//         // Πρόσθεσε το μικρότερο record στο output buffer
//         outputBuffer[outputBufferCount++] = entries[minIndex].currentRecord;
//
//         // Αν γεμίσει το output block, γράψτο στο output file
//         if (outputBufferCount == maxOutputRecords) {
//             for (int j = 0; j < outputBufferCount; j++) {
//                 HP_InsertEntry(output_FileDesc, outputBuffer[j]);
//             }
//             outputBufferCount = 0;
//         }
//
//         // Φόρτωσε το επόμενο record από το chunk που έδωσε το ελάχιστο
//         if (CHUNK_GetNextRecord(&entries[minIndex].it, &entries[minIndex].currentRecord) == 0) {
//             entries[minIndex].hasRecord = 1;
//         } else {
//             entries[minIndex].hasRecord = 0;
//         }
//     }
//
//     // Γράψε τυχόν υπόλοιπα records στο output file
//     for (int j = 0; j < outputBufferCount; j++) {
//         HP_InsertEntry(output_FileDesc, outputBuffer[j]);
//     }
// }

void merge(int input_FileDesc,
           int chunkSize,
           int bWay,
           int output_FileDesc) {
    CHUNK_Iterator chunkIt =
        CHUNK_CreateIterator(input_FileDesc, chunkSize);

    while (1) {
        CHUNK chunks[bWay];
        CHUNK_RecordIterator recIts[bWay];
        Record currentRecords[bWay];
        bool active[bWay];

        int actualChunks = 0;

        /* Πάρε έως bWay chunks */
        for (int i = 0; i < bWay; i++) {
            if (CHUNK_GetNext(&chunkIt, &chunks[i]) == 0) {
                recIts[i] = CHUNK_CreateRecordIterator(&chunks[i]);
                if (CHUNK_GetNextRecord(&recIts[i], &currentRecords[i]) == 0) {
                    active[i] = true;
                    actualChunks++;
                } else {
                    active[i] = false;
                }
            } else {
                active[i] = false;
            }
        }

        /* Αν δεν πήραμε κανένα chunk, τέλος */
        if (actualChunks == 0)
            break;

        /* Merge loop */
        while (1) {
            int minIdx = -1;

            for (int i = 0; i < bWay; i++) {
                if (!active[i])
                    continue;

                if (minIdx == -1 ||
                    shouldSwap(&currentRecords[minIdx],
                               &currentRecords[i])) {
                    minIdx = i;
                }
            }

            if (minIdx == -1)
                break; // όλοι οι iterators τελείωσαν

            /* Γράψε το μικρότερο record στο output */
            HP_InsertEntry(output_FileDesc, currentRecords[minIdx]);

            /* Προχώρα τον iterator που το έδωσε */
            if (CHUNK_GetNextRecord(&recIts[minIdx],
                                    &currentRecords[minIdx]) != 0) {
                active[minIdx] = false;
            }
        }
    }
}
