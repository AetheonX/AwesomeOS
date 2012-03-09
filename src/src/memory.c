#ifdef DEBUG_0
#include <stdio.h>
#endif // DEBUG_0

#include <stdint.h>

#ifdef DEBUG_0
#define USR_SZ_STACK 0x200   // user proc stack size 2048 = 0x200 *4 bytes
#else
#define USR_SZ_STACK 0x400   // user proc stack size 512 = 0x80 *4 bytes
#endif // DEBUG_0

#define NumberOfMemoryBlocks (128)
#define SizeOfMemoryBlock (128)	// in Bytes

extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;
unsigned int currentMemoryAddress = (unsigned int) &Image$$RW_IRAM1$$ZI$$Limit;
static int memBlocks[NumberOfMemoryBlocks] = {0};


void memory_init()
{
	int i = 0;
	
	for (i=0; i < NumberOfMemoryBlocks; i++)
	{
        memBlocks[i] = 0;
	}
}

void* k_request_memory_block(void)
{
	while(1)
	{
		int i;
		void *ret;
	
		for (i=0; i < NumberOfMemoryBlocks; i++)
		{
			if (memBlocks[i] == 0)
			{
				memBlocks[i] = 1;
				ret = (void *)(currentMemoryAddress + (i * SizeOfMemoryBlock));
				printf("req_mem_block returns %X\n", ret);
				return (void *)(currentMemoryAddress + (i * SizeOfMemoryBlock));
			}
		}
    
		// Block the process
		mem_block_current_process();
	}
	
	return 0;
}

uint32_t* request_proc_stack(void)
{
	currentMemoryAddress += USR_SZ_STACK;
	 
	return (uint32_t*)currentMemoryAddress;
}

int k_release_memory_block(void* block)
{
	unsigned int addr = (unsigned int) block;
    int index = (addr - currentMemoryAddress) / SizeOfMemoryBlock;
    memBlocks[index] = 0;
	
	remove_memory_blocks();

	return 0;
}
