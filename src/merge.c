#include <limits.h>
#include <merge.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc)
{
    CHUNK_Iterator chunkIt =
        CHUNK_CreateIterator(input_FileDesc, chunkSize);

    while (1) {

        /* ===== Φόρτωση έως bWay chunks ===== */
        CHUNK chunks[bWay];
        CHUNK_RecordIterator recIts[bWay];
        Record current[bWay];
        bool hasRecord[bWay];

        int activeChunks = 0;

        for (int i = 0; i < bWay; i++) {
            if (CHUNK_GetNext(&chunkIt, &chunks[i]) == 0) {
                recIts[i] = CHUNK_CreateRecordIterator(&chunks[i]);
                if (CHUNK_GetNextRecord(&recIts[i], &current[i]) == 0) {
                    hasRecord[i] = true;
                    activeChunks++;
                } else {
                    hasRecord[i] = false;
                }
            } else {
                hasRecord[i] = false;
            }
        }

        /* Αν δεν φορτώθηκε κανένα chunk → τέλος */
        if (activeChunks == 0)
            break;

        /* ===== Κλασικό b-way merge ===== */
        while (activeChunks > 0) {

            int minIdx = -1;

            for (int i = 0; i < bWay; i++) {
                if (!hasRecord[i])
                    continue;

                if (minIdx == -1 ||
                    shouldSwap(&current[minIdx], &current[i])) {
                    minIdx = i;
                }
            }

            /* Γράφουμε το μικρότερο record στο output */
            HP_InsertEntry(output_FileDesc, current[minIdx]);

            /* Προχωράμε τον iterator του συγκεκριμένου chunk */
            if (CHUNK_GetNextRecord(&recIts[minIdx], &current[minIdx]) != 0) {
                hasRecord[minIdx] = false;
                activeChunks--;
            }
        }
    }
}