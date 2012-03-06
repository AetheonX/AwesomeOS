/*
 *@author: Valeriy Chibisov
 */
#include "rtx.h"
#include "uart_polling.h"

#ifdef DEBUG_0
#include <stdio.h>
#endif // DEBUG_0

// HACK
int results[7];
void* lastMemBlock;

// NULL proccess - RESERVED - DO NOT CHANGE
void proc0(void) {
	while (1) {
	 	release_processor();
	}
}

void proc1(void) {
    while (1) {
		uart0_put_string("P1:\n\r");

		receive_message(0);

		uart0_put_string("~~~~~~~~~~~~~~~~~~P1:\n\r");

		release_processor();
    }
}

void proc2(void) {
    while (1) {
		uart0_put_string("P2:\n\r");
		send_message(1, "Communication established!");
		send_message(1, "This is a test");
		send_message(1, "22222222");
		send_message(1, "333333333");
        lastMemBlock = request_memory_block();
		printf("mem block: %X\n", lastMemBlock);
		results[2] = (lastMemBlock == 0) ? 0 : 1;

		uart0_put_string("~~~~~~~~~~~~~~~~~~P2:\n\r");

		release_processor();
		release_memory_block(lastMemBlock);
    }
}

void proc3(void) {
	// TESTING PRIORITY LEVEL GETTER
	int i = 1;
	while (i) {
		int result;
		result = 1;

		if (get_process_priority(3) != 3)
			result = 0;

		results[3] = result;
		
		if (i == 6)
			release_memory_block(lastMemBlock);
		
		i++;
		
		release_processor();
	}
}

void proc4(void) {
	// TESTING PRIORITY LEVEL SETTER - CORRECT USAGE
	while (1) {
		int result;
		result = 1;

		set_process_priority(4,2);
		if (get_process_priority(4) != 2)
			result = 0;
		set_process_priority(4,3);

		results[4] = result;

		release_processor();
	}
}

void proc5(void) {
	// TESTING PRIORITY LEVEL SETTER - SETTING PRIORITY OF ANOTHER PROCESS - FAILS
	while (1) {
		int result;
		result = 1;

		set_process_priority(4,2);
		if (get_process_priority(4) == 2)
			result = 0;

		results[5] = result;

		release_processor();
	}
}

void proc6(void) {
	// OUTPUT PROCESS - gathers data from all the other processes and outputs to UART0
	while (1) {
		int counter = 0;
		int total	= 5;
		int i = 0;
		int asciioffset = 48;
		
		uart0_put_string("G021_test: START\n\r");
		uart0_put_string("G021_test: total ");uart0_put_char(total+asciioffset);uart0_put_string(" tests\n\r");
		for (i = 1; i < total+1; i++) {
			uart0_put_string("G021_test: test ");
			uart0_put_char(i+asciioffset);
			
			if (is_process_blocked(i))
			{
				uart0_put_string(" BLOCKED");
			}
			else
			{
				if (results[i] == 0)
					uart0_put_string(" FAIL");
				else {
					uart0_put_string(" OK");
					counter++;
				}
			}
			uart0_put_string("\n\r");
		}
		uart0_put_string("G021_test: ");uart0_put_char(counter+asciioffset);uart0_put_string("/");uart0_put_char(total+asciioffset);uart0_put_string(" tests OK\n\r");
		uart0_put_string("G021_test: ");uart0_put_char((total-counter)+asciioffset);uart0_put_string("/");uart0_put_char(total+asciioffset);uart0_put_string(" tests FAIL\n\r");
		uart0_put_string("G021_test: END\n\r");
		
		release_processor();
	}
}
