#include <stdlib.h>
#include <stdio.h>
#include "heaplib.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

/* You must implement these functions according to the specification
 * given in heaplib.h. You can define any types you like to this file.
 *
 * Student 1 Name: Bernard Jiang
 * Student 1 NetID: btj29
 * Student 2 Name: Dylan Chiu
 * Student 2 NetID: dtc66
 * 
 * Include a description of your approach here.
 *
 */

#define ALIGNMENT 8
#define ADD_BYTES(base_addr, num_bytes) (((char *)(base_addr)) + (num_bytes))
#define SUB_BYTES(base_addr, num_bytes) (((char *)(base_addr)) - (num_bytes))
#define ALIGN(base_addr) (((char *)(base_addr)) + ALIGNMENT - (uintptr_t)(base_addr) % ALIGNMENT)
//need define that also finds previous 8 aligned pointer
//need define that finds next 8 aligned pointer
#define next8Alloc(base_addr, num_bytes) (ALIGN(ADD_BYTES(base_addr, num_bytes)))

typedef struct _heap_header_t {
    unsigned int max_size; // in bytes
    void *first;
} heap_header_t;

typedef struct _block_header_t {
    unsigned int size; // in bytes, not including meta-data
    void *next; //pointer
    void *prev;
} block_header_t;

int hl_init(void *heapptr, unsigned int heap_size) {
    heap_header_t *heap = (heap_header_t*) ALIGN(heapptr);

    //Fails if heap_size is not large enough to hold meta-data
    if (heap_size < sizeof(heap_header_t)) {
        return FAILURE;
    }
    //initializes the heap
    heap->max_size = heap_size;
    heap->first = NULL;

    return SUCCESS;
}

void *hl_alloc(void *heapptr, unsigned int payload_size) {
    
    heap_header_t *heap = (heap_header_t*) ALIGN(heapptr);
    block_header_t *block;
    block_header_t *blockptr;
    
    //Size of block including meta-data
    unsigned int block_size = sizeof(block_header_t) + payload_size;

    //Block size too big for heap
    if (block_size > heap->max_size - sizeof(heap_header_t)) {
        return FAILURE;
    }

    block_header_t *first_block = (block_header_t*) heap->first;

    //If first allocation
    if (first_block == NULL) {
        block = (block_header_t*) ADD_BYTES(next8Alloc(heapptr, sizeof(heap_header_t)), sizeof(block_header_t));
        block->size = payload_size;
        block->next = NULL;
        block->prev = NULL;
        heap->first = block;
        
        return block;
    }

    //Block size fits before first block
    if (block_size < (uintptr_t) SUB_BYTES(heap->first, sizeof(block_header_t)) - (uintptr_t)next8Alloc(heapptr, sizeof(heap_header_t))) {
        block = (block_header_t*) ADD_BYTES(next8Alloc(heapptr, sizeof(heap_header_t)), sizeof(block_header_t));
        block->size = payload_size;
        block->next = heap->first;
        block->prev = NULL;
        first_block->prev = block;
        heap->first = block;
        
        return block;
    }

    blockptr = heap->first;
    block_header_t *next_block = (block_header_t*) blockptr->next;

    //Checks if block fits in between existing blocks
    while(next_block != NULL) {
        if (block_size <= (uintptr_t) SUB_BYTES(blockptr->next, sizeof(block_header_t)) - (uintptr_t)next8Alloc(blockptr, block_size)) {
            block = (block_header_t*) ADD_BYTES(next8Alloc(blockptr, sizeof(block_header_t)), sizeof(block_header_t));
            block->size = payload_size;
            block->next = blockptr->next;
            block->prev = blockptr;
            next_block->prev = block;
            blockptr->next = block;

            return block;
        }

        //Iterate to next block if does not fit
        blockptr = blockptr->next;
    }

    //Checks if block fits at end of heap
    if (block_size <= (uintptr_t)heap + heap->max_size - (uintptr_t)next8Alloc(blockptr, block_size)) {
        block = (block_header_t*) ADD_BYTES(next8Alloc(blockptr, sizeof(block_header_t)), sizeof(block_header_t));
        block->size = payload_size;
        block->next = NULL;
        block->prev = blockptr;
        blockptr->next = block;
        
        return block;
    }

    return FAILURE;
}

void hl_release(void *heapptr, void *payload_ptr) {

    heap_header_t *heap = (heap_header_t*) ALIGN(heapptr);
    block_header_t *block;
    block = (block_header_t*) payload_ptr;
    
    //if (heap->init != 1024) return;
    
    block_header_t *nextBlock;
    block_header_t *prevBlock;
    
    if ((uintptr_t)payload_ptr == 0 || heapptr == NULL) {
        return;
    }
    
    if (block != NULL && block->prev == NULL && block->next != NULL) { //deleting the first block
        heap->first = block->next;
        nextBlock = block->next;
        nextBlock->prev = NULL; 
        
    } else if (block != NULL && block->prev == NULL && block->next != NULL) {//deleting the last block
        prevBlock = block->prev;
        prevBlock->next = NULL;
        
    } else if (block != NULL && block->prev == NULL && block->next == NULL) {//deleting the only block
        heap->first = NULL;
        
    } else {
        nextBlock = block->next;
        prevBlock = block->prev;
        nextBlock->prev = block->prev;
        prevBlock->next = block->next;
    }
}

void *hl_resize(void *heapptr, void *payload_ptr, unsigned int new_size) {
    heap_header_t *heap = (heap_header_t*) ALIGN(heapptr);
    block_header_t *block;
    block = (block_header_t*) payload_ptr;
    
    //if (heap->init != 1024) return FAILURE;
    
    unsigned int block_size = sizeof(block_header_t) + new_size;
    
    if ((uintptr_t)payload_ptr == 0) {
        return hl_alloc(heapptr, new_size);
    }
    
    //what if block is the only block?
    //if reducing size or if the new block size fits
    if (new_size <= block->size || 
    (block->next == NULL && block_size <= (uintptr_t)heap + heap->max_size - (uintptr_t)block) || 
    (block->next != NULL && block_size <= (uintptr_t)SUB_BYTES(block->next, sizeof(block_header_t)) - (uintptr_t)SUB_BYTES(block, sizeof(block_header_t)))) { 
            
        block->size = new_size;
        return block;
    } 
    
    //if it's here, it means that the we're trying to make the block bigger and it doesn't fit
    hl_release(heapptr, payload_ptr); //free the block so no other blocks are linked to it
    block_header_t *new_block;
    new_block = (block_header_t*) hl_alloc(heapptr, new_size); //allocate a new block of that space
    
    if (new_block == FAILURE) {
        return FAILURE; //if the block allocation fails, fail
    }
    
    /*if (new_size < block-> size) { //if the new size is smaller than the old size
        memcpy((char*)new_block + sizeof(block_header_t), (char*)block + sizeof(block_header_t), new_size);
        //only copy up till new_size
    } else {*/
        memcpy((char*)new_block + sizeof(block_header_t), (char*)block + sizeof(block_header_t), block->size);
    //}
    
    return new_block;
}
