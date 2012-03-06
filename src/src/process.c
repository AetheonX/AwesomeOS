/**
 * @brief: process.h  process management source file
 * @author Irene Huang
 * @author Thomas Reidemeister
 * @author Valeriy Chibisov
 */

#include <LPC17xx.h>
#include <system_LPC17xx.h>
#include "uart_polling.h"
#include "process.h"

#ifdef DEBUG_0
#include <stdio.h>
#endif // DEBUG_0

uint32_t next_ready()
{
	pcb_t *result;
	pcb_t *temp;
	
	if(pcb_readyQueue.first != NULL){
		
		result = pcb_readyQueue.first;
		
		while(result->nextReady != NULL){
			result = result->nextReady;
		}

		result->nextReady = pcb_readyQueue.first;
		temp = pcb_readyQueue.first;
		pcb_readyQueue.first = pcb_readyQueue.first->nextReady;		
		temp->nextReady = NULL;

		return temp->m_pid; 
	}

	return -1;
}

void move_to_ready(uint32_t pid)
{
	pcb_t *process = &pcb[pid];
	
	if(pcb_readyQueue.first == NULL) {
		pcb_readyQueue.first = process;
	} else {
		pcb_t *p = pcb_readyQueue.first;
		
		if(p->m_ppriority > process->m_ppriority)
		{
			pcb_readyQueue.first = process;
			process->nextReady = p;
		}
		else
		{
			pcb_t *parent = NULL;
			
			while(p->nextReady != NULL)
			{
				if (p->m_ppriority <= process->m_ppriority)
				{
					parent = p;
					p = p->nextReady;
				}
				else
					break;
			}

			if(parent != NULL)
			{
				process->nextReady = p;
				parent->nextReady = process;
			}
			else
			{
				p->nextReady = process;
			}
		}
	}
	
	remove_from_blocked(pid);
	
	/*
	pcb_readyq.pcb_q[pcb_readyq.nid] = pid;
	pcb_readyq.nid++;

	if (pcb_readyq.nid >= PNUM)
		pcb_readyq.nid = 0;*/
}

void print_ready()
{
	pcb_t *p = pcb_readyQueue.first;
	
	while(p != NULL)
	{
		printf("pid: %d - state: %d\n", p->m_pid, p->m_state);
		p = p->nextReady;
	}
}

void print_blocked()
{
	pcb_t *p = pcb_blockedQueue.first;
	
	while(p != NULL)
	{
		printf("pid: %d\n", p->m_pid, p->m_state);
		p = p->nextBlocked;
	}
}

void remove_from_ready(uint32_t pid)
{
	pcb_t *process = pcb_readyQueue.first;
	pcb_t *parentProcess = NULL;
	
	while(process != NULL)
	{	
		if (process->m_pid == pid)
		{
			if(parentProcess == NULL)
				pcb_readyQueue.first = process->nextReady;
			else
				parentProcess->nextReady = process->nextReady;
			
			process->nextReady = NULL;
			break;
		}
		else
		{
			parentProcess = process;
			process = process->nextReady;
		}
	}
}

void move_to_blocked(uint32_t pid, proc_state_t new_state)
{
	pcb_t *process = &pcb[pid];
	
	if(pcb_blockedQueue.first == NULL) {
		pcb_blockedQueue.first = process;
	} else {
		pcb_t *p = pcb_blockedQueue.first;
		
		if(p->m_ppriority > process->m_ppriority)
		{
			pcb_blockedQueue.first = process;
			process->nextBlocked = p;
		}
		else
		{
			pcb_t *parent = NULL;
			
			while(p->nextBlocked != NULL)
			{
				if (p->m_ppriority <= process->m_ppriority)
				{
					parent = p;
					p = p->nextBlocked;
				}
				else
					break;
			}

			if(parent != NULL)
			{
				process->nextBlocked = p;
				parent->nextBlocked = process;
			}
			else
			{
				p->nextBlocked = process;
			}
		}
	}
	process->m_state = new_state;
	remove_from_ready(pid);
}

void mem_block_current_process()
{
	printf(" called mem_block_current_process for pid %d\n", gp_current_process->m_pid);
	move_to_blocked(gp_current_process->m_pid, MEM_BLOCKED);
	k_release_processor();
}

void remove_memory_blocks()
{
	pcb_t *process = pcb_blockedQueue.first;
	
	while(process != NULL)
	{
		pcb_t *next_blocked = process->nextBlocked;
		
		if (process->m_state == MEM_BLOCKED)
		{
			move_to_ready(process->m_pid);
		}
		
		process = next_blocked;
	}
}

void remove_from_blocked(uint32_t pid)
{
	pcb_t *process = pcb_blockedQueue.first;
	pcb_t *parentProcess = NULL;
	
	while(process != NULL)
	{	
		if (process->m_pid == pid)
		{
			if(parentProcess == NULL)
				pcb_blockedQueue.first = process->nextBlocked;
			else
				parentProcess->nextBlocked = process->nextBlocked;
			
			process->nextBlocked = NULL;
			process->m_state = RDY;
			break;
		}
		else
		{
			parentProcess = process;
			process = process->nextBlocked;
		}
	}
}

bool is_process_blocked(uint32_t pid)
{
	pcb_t *process = &pcb[pid];

	if (process->m_state == MEM_BLOCKED || process->m_state == MSG_BLOCKED)
		return true;
	
	return false;
}

void process_init() 
{
    volatile int i;
	uint32_t *sp;
	int pid = 0;
	pcb_blockedQueue.first = NULL;
	pcb_readyQueue.first = NULL;

	// Init all the system processes' entry points here
	pcb[0].m_proc = &proc0;
	pcb[1].m_proc = &proc1;
	pcb[2].m_proc = &proc2;
	pcb[3].m_proc = &proc3;
	pcb[4].m_proc = &proc4;
	pcb[5].m_proc = &proc5;
	pcb[6].m_proc = &proc6;
	
	for (pid = 0; pid < PNUM; pid++) {
		// initialize the process exception stack frame
		pcb_t pcb_c 		= pcb[pid];
		pcb_c.m_pid 		= pid;
		pcb_c.m_state 		= NEW;
		pcb_c.m_ppriority 	= (pid == 0) ? 4 : 3;
		pcb_c.nextBlocked	= NULL;
		pcb_c.nextReady		= NULL;
		
		sp = (uint32_t*)request_proc_stack();
	    
		// 8 bytes alignement adjustment to exception stack frame
		if (!(((uint32_t)sp) & 0x04))
		    --sp; 
		
		*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
		*(--sp)  = (uint32_t) pcb_c.m_proc; // PC contains the entry point of the process
	
		for (i=0; i<6; i++) // R0-R3, R12 are cleared with 0
			*(--sp) = 0x0;

	    pcb_c.mp_sp = sp;

		pcb[pid] = pcb_c;
		move_to_ready(pid);
	}

	pcb_readyq.nid = 1;
}

int scheduler(void)
{
	/*volatile int pid;

	if (gp_current_process == NULL) {
	   gp_current_process = &pcb[next_ready()];
	   return 0;
	}
	
	pid = gp_current_process->m_pid;
	if (pid < 0)
		return -1; // error code -1
	
	// Round-robin approach*/
	return next_ready();
}

int k_release_processor(void)
{
	volatile int pid;
	volatile proc_state_t state;
	pcb_t * p_pcb_old = NULL;
	
	pid = scheduler();
	if (pid == -1)
		return -1;

	/*if (gp_current_process == NULL) {
		return -1;  
	}*/
	p_pcb_old = gp_current_process;
	gp_current_process = &pcb[pid];	
	
	state = gp_current_process->m_state;
	
	if (state == NEW) {
		if (p_pcb_old != NULL && p_pcb_old->m_state != NEW) {
			if(p_pcb_old->m_state != MEM_BLOCKED && p_pcb_old->m_state != MSG_BLOCKED){
				p_pcb_old->m_state = RDY;
			}
			p_pcb_old->mp_sp = (uint32_t *) __get_MSP();
		}
		gp_current_process->m_state = RUN;
		__set_MSP((uint32_t) gp_current_process->mp_sp);
		__rte();  // pop exception stack frame from the stack for new processes
	} else if (state == RDY) {
		if (p_pcb_old != NULL) {
			if(p_pcb_old->m_state != MEM_BLOCKED && p_pcb_old->m_state != MSG_BLOCKED){
				p_pcb_old->m_state = RDY;
			}
			p_pcb_old->mp_sp = (uint32_t *) __get_MSP(); // save the old process's sp
		}
		
		gp_current_process->m_state = RUN;
		__set_MSP((uint32_t) gp_current_process->mp_sp); //switch to the new proc's stack		
	} else {
		gp_current_process = p_pcb_old; // revert back to the old proc on error
		return -1;
	}
	return 0;
}

int k_set_process_priority(int process_ID, int priority) {
	// Restrictions - processID cannot be 0, processID has to be the same as the running process, priority level has to be between 0 and 3 (inclusively)
	if (process_ID == 0 || gp_current_process->m_pid != process_ID || priority < 0 || priority > 3)
		return -1; // -1 Error code

	pcb[process_ID].m_ppriority = priority;

	return 1;
}

int k_get_process_priority (int process_ID) {
	// Restrictions - processID cannot be less than zero and greater than the last process in the system (assuming all processIDs get assigned sequentially)
	if (process_ID < 0 || process_ID >= PNUM)
		return -1; // -1 Error code

 	return pcb[process_ID].m_ppriority; 
}
// HAS TO HAVE AN EMPTY LINE HERE
