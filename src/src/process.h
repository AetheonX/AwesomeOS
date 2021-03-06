#ifndef _PROCESS_H_
#define _PROCESS_H_

#define PNUM 12

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
#include "message.h"

typedef void (*proc_routine_t)(void);

typedef enum {NEW = 0, RDY, RUN, MEM_BLOCKED, MSG_BLOCKED} proc_state_t; 
typedef struct pcb pcb_t;

typedef struct envelope envelope;
typedef struct envelope {
	envelope *next;
	MessageEnvelope *info;
} envelope;

typedef struct pcb {
  uint32_t *mp_sp;      		// stack pointer of the process
  uint32_t m_pid;				// process id
  proc_state_t m_state; 		// state of the process 
  uint32_t m_ppriority; 		// process priority     

  proc_routine_t m_proc;
  pcb_t *nextBlocked;
  pcb_t *nextReady;
  
  envelope *m_envelope;  
} pcb_t;

typedef struct pcb_queue {
	int nid; // next index

	uint32_t pcb_q[PNUM];
} pcb_queue;

typedef struct ProcessQueue {
	pcb_t* first;
} ProcessQueue;

pcb_t pcb[PNUM];
pcb_queue pcb_readyq;
pcb_queue pcb_blockedq;

ProcessQueue pcb_blockedQueue;
ProcessQueue pcb_readyQueue;

pcb_t *gp_current_process = NULL;  // always point to the current process

void remove_from_blocked(uint32_t pid);
void remove_from_ready(uint32_t pid);
void move_to_ready(uint32_t pid);
void move_to_blocked(uint32_t pid, proc_state_t new_state);

pcb_t * current_process();
pcb_t * get_process(uint32_t pid);
void enqueue_envelope(uint32_t pid, MessageEnvelope *envel);
void dequeue_envelope(uint32_t pid, envelope *envel);

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

extern void proc_null(void);		// null process 0
extern void proc1(void);			// test process 1
extern void proc2(void);			// test process 2
extern void proc3(void);			// test process 3
extern void proc4(void);			// test process 4
extern void proc5(void);			// test process 5
extern void proc6(void);			// test process 6
extern void proc_crt_display(void);	// crt display
extern void proc_console(void);	// console process
extern void __rte(void);			// pop exception stack frame

#endif // ! _PROCESS_H_
