/*
 *@author: Valeriy Chibisov
 */
#include "rtx.h"
#include "uart_polling.h"

#ifdef DEBUG_0
#include <stdio.h>
#endif // DEBUG_0		

// NULL proccess - RESERVED - DO NOT CHANGE
void proc0(void) {
	while (1)
	 	release_processor();
}

void proc1(void)
{
    volatile int i =0;
	volatile int ret_val = 10;
    while (1) {
        if (i!=0 &&i%5 == 0 ) {
			uart0_put_string("\n\r");
            ret_val = release_processor();
#ifdef DEBUG_0
		    printf("\n\rproc1: ret_val=%d. ", ret_val);
#else
#endif // DEBUG_0
        }
        uart0_put_char('A' + i%26);
        i++;
    }

}

void proc2(void)
{
    volatile int i =0;
	volatile int ret_val = 20;
    while (1) {
        if (i!=0 &&i%5 == 0 ) {
			uart0_put_string("\n\r");
            ret_val = release_processor();
#ifdef DEBUG_0
	    	printf("\n\rproc2: ret_val=%d. ", ret_val);
#else
#endif // DEBUG_0
        }
        uart0_put_char('a' + i%26);
        i++;
    }
}

void proc3(void) {
	// TESTING PRIORITY LEVEL GETTER
	while (1) {
		int result;
		result = 1;

		if (get_process_priority(3) != 3)
			result = 0;

		uart0_put_string("G021_test: test 3 ");
		if (result == 0)
			uart0_put_string("FAIL");
		else
			uart0_put_string("OK");
		uart0_put_string("\n\r");

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

		uart0_put_string("G021_test: test 4 ");
		if (result == 0)
			uart0_put_string("FAIL");
		else
			uart0_put_string("OK");
		uart0_put_string("\n\r");

		release_processor();
	}
}

void proc5(void) {
	// TESTING PRIORITY LEVEL SETTER - SETTING PRIORITY OF ANOTHER PROCESS - FAILS
	while (1) {
		int result;
		result = 1;

		set_process_priority(4,2);
		if (get_process_priority(4) != 2)
			result = 0;

		uart0_put_string("G021_test: test 5 ");
		if (result == 0)
			uart0_put_string("FAIL");
		else
			uart0_put_string("OK");
		uart0_put_string("\n\r");

		release_processor();
	}
}

void proc6(void) {
	// TEST CASE HERE!
	while (1)
		release_processor();
}
