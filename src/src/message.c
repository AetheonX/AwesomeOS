/**
 * @brief: Message management source file
 * @author Valeriy Chibisov
 */

#include <LPC17xx.h>
#include <system_LPC17xx.h>

#ifdef DEBUG_0
#include <stdio.h>
#endif // DEBUG_0
#include "uart_polling.h"
#include "message.h"
//#include "process.h" // problems when including

// Sodding linking....
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


extern pcb_t * current_process();
extern pcb_t * get_process();
//.................

// Sends a message to the process
int k_send_message (uint32_t pid, void *message) {
	MessageEnvelope *menv = (MessageEnvelope *)k_request_memory_block();

	if (message == 0)
		return;

	menv->sender_id 	= (current_process())->m_pid; 	// Field 1: Sender
	menv->receiver_id 	= pid; 							// Field 2: Receiver
	menv->message_type 	= 0; 							// Field 3: Message Type
	menv->message 		= message; 						// Field 4: Message

	enqueue_envelope(pid, menv);

	if ((get_process(pid))->m_state == MSG_BLOCKED)
		move_to_ready(pid);
}

// Process receives a message from sender
void * k_receive_message (uint32_t *sender_ID) {
	envelope *envel = (current_process())->m_envelope;

	/*while (envel == 0) {
		move_to_blocked((current_process())->m_pid, MSG_BLOCKED);
		k_release_processor();
		envel = (current_process())->m_envelope;
	}*/

	// Process all the queued messages
	while (envel != 0) {
		MessageEnvelope *menvel = envel->info;
		uart0_put_string("M:");
		uart0_put_string((unsigned char *)((menvel)->message));
		uart0_put_string("\n\r");

		dequeue_envelope((current_process())->m_pid, envel);

	 	envel = envel->next;
	}
}

// HAS TO HAVE AN EMPTY LINE HERE
