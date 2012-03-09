/*
 *@author: Valeriy Chibisov
 */
#include "rtx.h"
#include "uart.h"

#ifdef DEBUG_0
#include <stdio.h>
#endif // DEBUG_0

extern int strcmp(char *c1, char *c2);

extern volatile uint8_t g_UART0_TX_empty; 
extern volatile uint8_t g_UART0_buffer[BUFSIZE];
extern volatile uint32_t g_UART0_count;

// Special Characters that do Special Things
unsigned char CLEAR_SCREEN[5] = {27, 91, '2', 'J', 0};
unsigned char MOVE_CURSOR_BACK[5] = {27, 91, '1', 'D', 0};

//========================================================================================================

// NULL proccess - RESERVED - DO NOT CHANGE
// Process id: 0
void proc_null(void) {
	while (1) {
	 	release_processor();
	}
}

// Responds to only one message type - CRT display request
// Process id: 10
// Note when adding new processes, make sure to increment PNUM inside process.h
void proc_crt_display(void) {
	while (1) {
		int sender_id;
		unsigned char *display_string = receive_message(&sender_id);
	
		if (display_string == 0) {
			// A dummy hack here in order to account for non-blocking receive_message
		} else {
			uart0_put_string(display_string);
		}
	
		release_processor();
	}
}

// Console handling
// Process id: 11
void proc_console(void) {
	while (1) {
		int sender_id;

		send_message(10, CLEAR_SCREEN);
		send_message(10, "$ ");

		while (1) {
			if ( g_UART0_count != 0 ) { // Display input to the screen
				uint8_t *input = g_UART0_buffer;
				 
				send_message(10, input);
				g_UART0_count = 0;

				if (input[0] == 13) { // Enter key has been pressed
					// Process commands here
	
					send_message(10, "\n");
					send_message(10, "$ ");
				} else if (input[0] == 8) { // 1% Bonus mark here for this lol :D
					send_message(10, " ");
					send_message(10, MOVE_CURSOR_BACK);
				}
		    }
		
			release_processor();
		}
	}
}