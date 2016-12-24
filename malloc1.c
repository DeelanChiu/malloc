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

#define next8Alloc(base_addr, num_bytes) ( ALIGN( ADD_BYTES(base_addr, num_bytes) ) )

typedef struct _heap_header_t {
    unsigned int max_size; // in bytes
} heap_header_t ;

typedef struct _block_header_t {
    unsigned int size; // in bytes, meta-data included
  	void *next; //pointer
    void *prev;
    bool in_use_f; // true or false?
} block_header_t ;


int hl_init(void *heap_ptr, unsigned int heap_size) {

	//Fails if heap_size is not large enough to hold meta-data
	if (heap_size < sizeof(heap_header_t)) {
        return FAILURE;
    }
	//initializes the heap
    heap_header_t *heap = (heap_header_t *)heapptr;
    heap->max_size = heap_size;
}

void *hl_alloc(void *heap_ptr, unsigned int payload_size) {
	//run through all blocks in the loop, see which unused block fits payload_size
	//if it finds a fitting block, it will delete the empty block, add the new, used block, and add an empty block after
  
  	//this loop starts with an if and then, afterwards, uses a while loop, keeping a prev pointer
  	//1. see if space is taken up by a used block
  	//2. if it is, see if (next - (blockptr + sizeof(blockptr)) can fit
  	//  3. if it cannot, repeat with the next block
  	//2. if it isn't, save current address, loop until ptr+maxsize to find the next block and see if there is enough space in between
  
  	//4. A good space is found when there is space. In that case, it saves the end limit, which is the ptr to the next block (maybe).
  
  	//this means in the beginning, it will have to loop thr... unless there's a boolean saying if this is the first alloc
  
  	//Allocation
  	//1. set the prev pointer of the block
  	//2. set the next pointer of the previous block to this block
  	//3  see if this block's next pointer is the end of the heap. If it is, done. Or else..
  	//4. set this block's next pointer, and
  	//5. set the next block's previous pointer to this block
  	
	heap_header_t *heap = (heap_header_t*) heapptr;

   	//base of heap + heap header + block header + payload size
  	if (ADD_BYTES(ADD_BYTES(next8Alloc(heapptr, sizeof(heap_header_t)), sizeof(block_header_t)), payload_size) >=  heap->max_size) {
		return FAILURE;
  	}

  	block_header_t *block = (block_header_t *) next8Alloc(heapptr, sizeof(heap_header_t)); //points to beginning of the first block;
  
    block_header_t *prevBloc = *block;
    block_header_t *nextBloc = *block
  
  	//start with the first 
  	if (*block != NULL && block->in_use_f) { //if the block is filled with a used block
      
      	*nextBloc = *(block->next);

      	//if there is enough space between this block and the heap's end if this is the only block
      	// end of heap - (this block's address + block header + block size)
      	//if there is enough space between this block and the next block
      	// next block address - (this block's address + block header + block size)
      	if ((nextBloc == NULL && payload_size <= 
             SUB_BYTES(ADD_BYTES(heapptr , heap->max_size), next8Alloc(ADD_BYTES(block, sizeof(block_header_t)), block->size))) || 
            
            (nextBloc != NULL && payload_size <= SUB_BYTES(block->next, next8Alloc(ADD_BYTES(block, sizeof(block_header_t)), block->size)))) {

          
        	block->next = next8Alloc(block, sizeof(block_header_t)); //set the next pointer of the previous block to this block
        	//allocation 
	        block = next8Alloc(block, sizeof(block_header_t)); //allocate to the space after the block
	        block->next = nextBloc; 
	        block->prev = prevBloc; //points to the first block
	        block->in_use_f = TRUE;
	        block->size = payload_size;
          
      		return SUCCESS; //end of block allocation after the first block in the heap
        
      	}
       //doesn't fit after the first block
          	//if it's here, it means 
	          //there is not enough space between this block and the heap's end and this is the only block (opposite of condition above)
        if ( nextBloc == NULL) {
	            return FAILURE; //no space, fail
        }
          
      
    } else { //if the block is empty or equivalent
        //base of heap + heap header + block header + payload size
        //search for the next used block 
      	while (block <= ADD_BYTES( ADD_BYTES( next8Alloc(heapptr, sizeof(heap_header_t)), sizeof(block_header_t)), payload_size) || 
               *block == NULL || !block->in_use_f)) {
        	block = (char *)block + (ALIGNMENT-1); //iterate address by the 8 byte alignment
      	}
      
      	if ( block > ADD_BYTES( next8Alloc(heapptr, sizeof(heap_header_t)), payload_size )) { //if the beginning of the heap can fit the payload
      		block->next = block; //set the next pointer to current location for now
        	*block = (block_header_t *) next8Alloc(heapptr, sizeof(heap_header_t)); //set the address back to the start of blocks
        	block->prev = NULL; //at the beginning of th heap, no prev block
        	block->in_use_f = TRUE;
        	block->size = payload_size;
        
        	while ( block->next <= ADD_BYTES(heapptr , block->max_size)) { //find the next block
          		block->next = (char *)(block->next) + (ALIGNMENT-1); //iterate address by the 8 byte alignment
        	}
        
        	if ( nextBlock > ADD_BYTES(heapptr , block->max_size)) {
          		block->next = NULL; //if there is no other block, the next ptr points to nothing
        	}

        	return SUCCESS; //end of block allocation at the beginning of the heap  
    	}
      
    	//here: there is not enough space in front of the heap, *block points to the first block
      
    	//do one iteration of the loop so, if unsuccessful, the prev pointer can point here
      
      	block_header_t *nextBloc = *(block->next);

      	//if there is enough space between this block and the heap's end if this is the only block
      	// end of heap - (this block's address + block header + block size)
      	//if there is enough space between this block and the next block
      	// next block address - (this block's address + block header + block size)
      	if ((nextBloc == NULL && payload_size <= 
             SUB_BYTES(ADD_BYTES(heapptr , heap->max_size), next8Alloc(ADD_BYTES(block, sizeof(block_header_t)), block->size))) || 
            
            (nextBloc != NULL && payload_size <= SUB_BYTES(block->next, next8Alloc(ADD_BYTES(block, sizeof(block_header_t)), block->size)))) {

          
        	block->next = next8Alloc(block, sizeof(block_header_t)); //set the next pointer of the previous block to this block
          	*prevBlock = *block;  //save this current block as prev block
          
        	//allocation 
	        block = next8Alloc(block, sizeof(block_header_t)); //allocate to the space after the block
	        block->next = nextBloc; 
	        block->prev = prevBloc; //points to the first block
	        block->in_use_f = TRUE;
	        block->size = payload_size;
          
      		return SUCCESS; //end of block allocation after the first block in the heap
        }
      //doesn't fit after the first block
          	//if it's here, it means 
	          //there is not enough space between this block and the heap's end and this is the only block (opposite of condition above)
        if ( nextBloc == NULL) {
            return FAILURE; //no space, fail
        }
          
    }
  	block = nextBloc; //iterate to the next block
  	//if it makes it here, we expect *block to point at the second block and prev to point at the first block 
        
  	//start looping through the heap
    while (block != NULL) {
      if ((block->next == NULL && payload_size <= 
               SUB_BYTES(ADD_BYTES(heapptr , heap->max_size), next8Alloc(ADD_BYTES(block, sizeof(block_header_t)), block->size))) || 

              (block->next != NULL && payload_size <= SUB_BYTES(block->next, next8Alloc(ADD_BYTES(block, sizeof(block_header_t)), block->size)))) {

              nextBloc = block->next;
              block->next = next8Alloc(block, sizeof(block_header_t)); //set the next pointer of the previous block to this block
              *prevBlock = *block;  //save this current block as prev block

              //allocation 
              block = next8Alloc(block, sizeof(block_header_t)); //allocate to the space after the block
              block->next = nextBloc; 
              block->prev = prevBlock; //points to the first block
              block->in_use_f = TRUE;
              block->size = payload_size;

              return SUCCESS; //end of block allocation after the first block in the heap
       }
      
      block = block->next;
      
    }  


  	return FAILURE;
}

void hl_release(void *heap_ptr, void *payload_ptr) {

  	//after release, run through heap to connect unused blocks together
}

void *hl_resize(void *heap_ptr, void *payload_ptr, unsigned int new_size) {
  	return FAILURE; 

}
