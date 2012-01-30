/**
 * @brief: process.h  process management source file
 * @author Irene Huang
 * @author Thomas Reidemeister
 * @author Valeriy Chibisov
 * @date 2011/01/18
 * NOTE: The example code shows one way of context switching implmentation.
 *       The code only has minimal sanity check. There is no stack overflow check.
 *       The implementation assumes only two simple user processes, NO external interrupts. 
 *  	 The purpose is to show how context switch could be done under stated assumptions. 
 *       These assumptions are not true in the required RTX Project!!!
 *       If you decide to use this piece of code, you need to understand the assumptions and
 *       the limitations. Some part of the code is not written in the most efficient way.
 */

#include <LPC17xx.h>
#include <system_LPC17xx.h>
#include "uart_polling.h"
#include "process.h"

#ifdef DEBUG_0
#include <stdio.h>
#endif // DEBUG_0


/**
 * @biref: initialize all processes in the system
 * NOTE: We assume there are only two user processes in the system in this example.
 *       We should have used an array or linked list pcbs so the repetive coding
 *       can be eliminated.
 *       We should have also used an initialization table which contains the entry
 *       points of each process so that this code does not have to hard code
 *       proc1 and proc2 symbols. We leave all these imperfections as excercies to the reader 
 */
void process_init() 
{
    volatile int i;
	uint32_t * sp;
	int pid = 0;

	// HACK
	pcb[0].m_proc = &proc0;
	pcb[1].m_proc = &proc1;
	pcb[2].m_proc = &proc2;

	for (pid = 0; pid < PNUM; pid++) {
		// initialize the process exception stack frame
		pcb_t pcb_c 		= pcb[pid];
		pcb_c.m_pid 		= pid;
		pcb_c.m_state 		= NEW;
		pcb_c.m_ppriority 	= 3;
	
		sp = pstack[pid] + USR_SZ_STACK;
	    
		// 8 bytes alignement adjustment to exception stack frame
		if (!(((uint32_t)sp) & 0x04))
		    --sp; 
		
		*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
		*(--sp)  = (uint32_t) pcb_c.m_proc; // PC contains the entry point of the process
	
		for (i=0; i<6; i++) // R0-R3, R12 are cleared with 0
			*(--sp) = 0x0;

	    pcb_c.mp_sp = sp;

		pcb[pid] = pcb_c;
	}
}

/*@brief: scheduler, pick the pid of the next to run process
 *@return: pid of the next to run process
 *         -1 if error happens
 *POST: if gp_current_process was NULL, then it gets set to &pcb1.
 *      No other effect on other global variables.
 */
int scheduler(void)
{
	volatile int pid;
	
	if (gp_current_process == NULL) {
	   gp_current_process = &pcb[0];
	   return 0;
	}
	
	pid = gp_current_process->m_pid;
	
	if (pid == 0 )
	    return 1;
	else if (pid == 1 )
	    return 2;
	else if (pid == 2 )
	    return 1;
	else
		return -1; // error code -1
}
/**
 * @brief release_processor(). 
 * @return -1 on error and zero on success
 * POST: gp_current_process gets updated
 */
int k_release_processor(void)
{
	volatile int pid;
	volatile proc_state_t state;
	pcb_t * p_pcb_old = NULL;
	
	pid = scheduler();
	if (gp_current_process == NULL) {
		return -1;  
	}
	
	p_pcb_old = gp_current_process;

	if (pid == -1)
		return -1;

	gp_current_process = &pcb[pid];	
	
	state = gp_current_process->m_state;

	if (state == NEW) {
		if (p_pcb_old->m_state != NEW) {
			p_pcb_old->m_state = RDY;
			p_pcb_old->mp_sp = (uint32_t *) __get_MSP();
		}
		gp_current_process->m_state = RUN;
		__set_MSP((uint32_t) gp_current_process->mp_sp);
		__rte();  // pop exception stack frame from the stack for new processes
	} else if (state == RDY){     
		p_pcb_old->m_state = RDY; 
		p_pcb_old->mp_sp = (uint32_t *) __get_MSP(); // save the old process's sp
		
		gp_current_process->m_state = RUN;
		__set_MSP((uint32_t) gp_current_process->mp_sp); //switch to the new proc's stack		
	} else {
		gp_current_process = p_pcb_old; // revert back to the old proc on error
		return -1;
	}	 	 
	return 0;
}

int k_set_process_priority(int process_ID, int priority) {
	// TODO: Error checking here...

	pcb[process_ID].m_ppriority = priority;

	return 1;
}

int k_get_process_priority (int process_ID) {
	// TODO: Error checking here...

 	return pcb[process_ID].m_ppriority; 
}
// HAS TO HAVE AN EMPTY LINE HERE
