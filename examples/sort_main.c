#include "merge.h"
#include "sort.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RECORDS_NUM 500 // you can change it if you want
#define FILE_NAME "data.db"
#define OUT_NAME "out"

int createAndPopulateHeapFile(char *filename);

void sortPhase(int file_desc, int chunkSize);

void mergePhases(int inputFileDesc, int chunkSize, int bWay, int *fileCounter);

int nextOutputFile(int *fileCounter);

void print_and_validate(int file_desc, int chunkSize);

int main() {
    int chunkSize = 20;
    int bWay = 4;
    int fileIterator = 0;
    //
    BF_Init(LRU);
    int file_desc = createAndPopulateHeapFile(FILE_NAME);
    sortPhase(file_desc, chunkSize);
    mergePhases(file_desc, chunkSize, bWay, &fileIterator);
}

int createAndPopulateHeapFile(char *filename) {
    HP_CreateFile(filename);

    int file_desc;
    HP_OpenFile(filename, &file_desc);

    Record record;
    srand(12569874);
    for (int id = 0; id < RECORDS_NUM; ++id) {
        record = randomRecord();
        HP_InsertEntry(file_desc, record);
    }
    return file_desc;
}

/*Performs the sorting phase of external merge sort algorithm on a file specified by 'file_desc', using chunks of size 'chunkSize'*/
void sortPhase(int file_desc, int chunkSize) {
    sort_FileInChunks(file_desc, chunkSize);
}

/* Performs the merge phase of the external merge sort algorithm  using chunks of size 'chunkSize' and 'bWay' merging. The merge phase may be performed in more than one cycles.*/
void mergePhases(int inputFileDesc, int chunkSize, int bWay, int *fileCounter) {
    int outputFileDesc;
    while (chunkSize <= HP_GetIdOfLastBlock(inputFileDesc)) {
        outputFileDesc = nextOutputFile(fileCounter);
        merge(inputFileDesc, chunkSize, bWay, outputFileDesc);
        HP_CloseFile(inputFileDesc);
        chunkSize *= bWay;
        inputFileDesc = outputFileDesc;
    }

    // validating last output
    int lastValidChunkSize = chunkSize / bWay;
    printf("lastValidChunkSize: %d\n", lastValidChunkSize);
    print_and_validate(outputFileDesc, lastValidChunkSize);

    HP_CloseFile(outputFileDesc);
}

/*Creates a sequence of heap files: out0.db, out1.db, ... and returns for each heap file its corresponding file descriptor. */
int nextOutputFile(int *fileCounter) {
    char mergedFile[50];
    char tmp[] = "out";
    sprintf(mergedFile, "%s%d.db", tmp, (*fileCounter)++);
    int file_desc;
    HP_CreateFile(mergedFile);
    HP_OpenFile(mergedFile, &file_desc);
    return file_desc;
}

void print_and_validate(int file_desc, int chunkSize) {
    CHUNK_Iterator it = CHUNK_CreateIterator(file_desc, chunkSize);
    CHUNK chunk;
    if (CHUNK_GetNext(&it, &chunk) == -1) {
        fprintf(stderr, "Failed to get resulting chunk.\n");
        return;
    }

    // printing
    printf("Resulting chunk:\n");
    CHUNK_Print(chunk);

    // validation
    Record prev, current;
    int recordsInChunk = chunk.recordsInChunk;
    for (int i = 1; i < recordsInChunk; i++) {
        if (CHUNK_GetIthRecordInChunk(&chunk, i - 1, &prev) == -1) {
            fprintf(stderr, "Failed to get %dth record.\n", i - 1);
            return;
        }

        if (CHUNK_GetIthRecordInChunk(&chunk, i, &current) == -1) {
            fprintf(stderr, "Failed to get %dth record.\n", i);
            return;
        }

        if (shouldSwap(&prev, &current)) {
            printf("Found unsorted records.\n");
            return;
        }
    }

    if (CHUNK_GetNext(&it, &chunk) == 0) {
        fprintf(stderr, "Found more than one resulting chunks.\n");
        return;
    }

    printf("All records are sorted.\n");
}
