/**
 * @brief: process.h  process management hearder file
 * @author Irene Huang
 * @author Thomas Reidemeister
 * @author Valeriy Chibisov
 * @date 2011/01/18
 * NOTE: Assuming there are only two user processes in the system
 */

#ifndef _PROCESS_H_
#define _PROCESS_H_

#define PNUM 7

#define NULL 0
#define bool int
#define false 0
#define true 1
#define INITIAL_xPSR 0x01000000  // user process initial xPSR value

#ifdef DEBUG_0
#define USR_SZ_STACK 0x200   // user proc stack size 2048 = 0x200 *4 bytes
#else
#define USR_SZ_STACK 0x080   // user proc stack size 512 = 0x80 *4 bytes
#endif // DEBUG_0

#include <stdint.h>

typedef void (*proc_routine_t)(void);

typedef enum {NEW = 0, RDY, RUN, MEM_BLOCKED, MSG_BLOCKED} proc_state_t; 
typedef struct pcb pcb_t;

typedef struct pcb {
  
  //Note you may want to add your own member variables
  //in order to finish P1 and the entire project 
  //struct pcb *mp_next;  // next pcb, not used in this example, RTX project most likely will need it, keep here for reference
  
  uint32_t *mp_sp;      		// stack pointer of the process
  uint32_t m_pid;				// process id
  proc_state_t m_state; 		// state of the process 
  uint32_t m_ppriority; 		// process priority     

  proc_routine_t m_proc;
  pcb_t *nextBlocked;
  pcb_t *nextReady; 
} pcb_t;

typedef struct pcb_queue {
	int nid; // next index

	uint32_t pcb_q[PNUM];
} pcb_queue;

typedef struct ProcessQueue {
	pcb_t* first;
} ProcessQueue;

// TODO: turn this into a dynamically allocated memory
uint32_t pstack[PNUM][USR_SZ_STACK];      // stack for proc1

pcb_t pcb[PNUM];
pcb_queue pcb_readyq;
pcb_queue pcb_blockedq;

ProcessQueue pcb_blockedQueue;
ProcessQueue pcb_readyQueue;

pcb_t  *gp_current_process = NULL;  // always point to the current process

void remove_from_blocked(uint32_t pid);
void remove_from_ready(uint32_t pid);
void move_to_ready(uint32_t pid);
void move_to_blocked(uint32_t pid, proc_state_t new_state);
void remove_memory_blocks();
void block_current_process(proc_state_t state);
void print_ready();
void print_blocked();
bool is_process_blocked(uint32_t pid);
extern void process_init(void);	    					// initialize all procs in the system
int scheduler(void);									// pick the pid of the next to run process
int k_release_process(void);							// kernel release_process API
int k_set_process_priority(int process_ID, int priority); // set process priority API
int k_get_process_priority(int process_ID);	 			// get process priority API

extern void proc0(void);			// user process 0
extern void proc1(void);			// user process 1
extern void proc2(void);			// user process 2
extern void proc3(void);			// user process 3
extern void proc4(void);			// user process 4
extern void proc5(void);			// user process 5
extern void proc6(void);			// user process 6
extern void __rte(void);			// pop exception stack frame

#endif // ! _PROCESS_H_
