#include <stdlib.h>
#include <stdio.h>

#include "chunk.h"
#include "mem.h"
#include "value.h"
#include "vm.h"

void initChunk(Chunk *chunk, const char *filename)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    chunk->execution_counts = NULL;
    if (filename == NULL)
    {
        chunk->filename = NULL;
    }
    else
    {
        size_t filenameLen = strlen(filename) + 1;
        chunk->filename = ALLOCATE(char, filenameLen);
        strncpy(chunk->filename, filename, filenameLen);
    }
    initValueArray(&chunk->constants);
}

void recordInstructionExecuted(Chunk *chunk, size_t instruction)
{
    chunk->execution_counts[(uint16_t)instruction]++;
}

void writeChunk(Chunk *chunk, uint8_t byte, int line)
{
    if (chunk->capacity < chunk->count + 1)
    {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(chunk->code, uint8_t, oldCapacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(chunk->lines, int, oldCapacity, chunk->capacity);
        chunk->execution_counts = GROW_ARRAY(chunk->execution_counts, uint16_t, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->execution_counts[chunk->count] = 0;
    chunk->count++;
}

void freeChunk(Chunk *chunk)
{
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    FREE_ARRAY(uint16_t, chunk->execution_counts, chunk->capacity);
    if (chunk->filename != NULL)
    {
        FREE_ARRAY(char, chunk->filename, strlen(chunk->filename) + 1);
    }
    freeValueArray(&chunk->constants);
    initChunk(chunk, NULL);
}

void print_constants(Chunk *chunk)
{
    for (int i = 0; i < chunk->constants.count; i++)
    {
        printf("%d: ", i);
        printObject(chunk->constants.values[i]);
        printf("\n");
    }
}