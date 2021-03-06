/**
 * @brief: uart_irq.c 
 * @author: NXP Semiconductors
 * @author: Y. Huang
 * @date: 2012/01/20
 */

#include <LPC17xx.h>
#include "uart.h"

volatile uint8_t g_UART0_TX_empty=1;
volatile uint8_t g_UART0_buffer[BUFSIZE];
volatile uint32_t g_UART0_count = 0;

/**
 * @brief: initialize the n_uart
 * NOTES: only fully supports uart0 so far, but can be easily extended to other uarts.
 * The step number in the comments matches the item number in Section 14.1 on pg 298
 * of LPC17xx_UM
 */
int uart_init(int n_uart) {

    LPC_UART_TypeDef *pUart;

	if (n_uart ==0 ) {
	    // Steps 1 & 2: system control configuration.
		//              Under CMSIS, system_LPC17xx.c does these two steps
		 
		// Step 1: Power control configuration, table 46 pg63 in LPC17xx_UM
		// enable UART0 power, this is the default setting
		// done in system_LPC17xx.c under CMSIS
		// enclose the code for your refrence
	    //LPC_SC->PCONP |= BIT(3);
	
		
		// Step2: select the clock source, default PCLK=CCLK/4 , where CCLK = 100MHZ.
		// tables 40 and 42	on pg56 and pg57 in LPC17xx_UM
		// Check the PLL0 configuration to see how XTAL=12.0MHZ gets to CCLK=100MHZ
		// in system_LPC17xx.c file
		// enclose the code for your reference
		//LPC_SC->PCLKSEL0 &= ~(BIT(7)|BIT(6));	//PCLK = CCLK/4, default setting after reset	

		// Step 5: Pin Ctrl Block configuration for TXD and RXD
		// See Table 79 on pg108 in LPC17xx_UM for pin settings
		// Done before Steps3-4 for better coding purpose.

		LPC_PINCON->PINSEL0 |= (1 << 4);             // Pin P0.2 used as TXD0 (Com0) 
        LPC_PINCON->PINSEL0 |= (1 << 6);             // Pin P0.3 used as RXD0 (Com0) 

		pUart = (LPC_UART_TypeDef *) LPC_UART0;	 
		
	} else if (n_uart == 1) {
	    
		// see Table 79 on pg108 in LPC17xx_UM for pin settings 
	    LPC_PINCON->PINSEL0 |= (2 << 0);             // Pin P2.0 used as TXD1 (Com1) 
        LPC_PINCON->PINSEL0 |= (2 << 2);             // Pin P2.1 used as RXD1 (Com1) 

		pUart = (LPC_UART_TypeDef *) LPC_UART1;
		
	} else {
		return 1;	// not supported yet
	} 

    // Step 3: Transmission Configuration

	// Step 3a: DLAB=1, 8N1
	pUart->LCR    = UART_8N1;// see uart.h file 

	// Step 3b: 115200 baud rate @ 25.0 MHZ PCLK
	// seep section 14.4.12.1 pg313-315 in LPC17xx_UM for baud rate calculation
	pUart->DLM = 0;			 // see table 274, pg302 in LPC17xx_UM
	pUart->DLL = 9;			 // see table 273, pg302 in LPC17xx_UM
	pUart->FDR = 0x21;       // FR = 1.507 ~ 1/2, DivAddVal = 1, MulVal = 2
	                         // FR = 1.507 = 25MHZ/(16*9*115200)
							 // see table 285 on pg312 in LPC_17xxUM
 

	// Step 4 FIFO setup
	pUart->FCR = 0x07;		 // enable Rx and Tx FIFOs, clear Rx and Tx FIFOs
							 // Trigger level 0 (1 char per interrupt)
							 // see table 278 on pg305 in LPC17xx_UM


	// Step 5 was done between step 2 and step 4 a few lines above

	// Step 6 Interrupt setting
	// Step 6a: enable interrupt bits wihtin the specific peripheral register
    // Interrupt Sources Setting: RBR, THRE or RX Line Stats
	// See Table 50 on pg73 in LPC17xx_UM for all possible UART0 interrupt sources
	// See Table 275 on pg 302 in LPC17xx_UM for IER setting 
	pUart->LCR &= ~(BIT(7)); // disable the Divisior Latch Access Bit DLAB=0
	pUart->IER = IER_RBR | IER_THRE | IER_RLS; 

	// Step 6b: enable the UART interrupt from the system level
	// Use CMSIS call
	NVIC_EnableIRQ(UART0_IRQn);

	return 0;
}

/**
 * @brief: use CMSIS ISR for UART0 IRQ Handler
 * NOTE: This example shows how to save/restore all registers rather than just
 *       those backed up by the exception stack frame. We add extra
 *       push and pop instructions in the assembly routine. 
 *       The actual c_UART0_IRQHandler does the rest of irq handling
 */
__asm void UART0_IRQHandler(void)
{
	PRESERVE8
	IMPORT c_UART0_IRQHandler
	PUSH{r4-r11, lr}
	BL c_UART0_IRQHandler
	POP{r4-r11, pc}
} 
/**
 * @brief: c UART0 IRQ Handler
 */
void c_UART0_IRQHandler(void)
{
    uint8_t IIR_IntId;	    // Interrupt ID from IIR		
	uint8_t LSR_Val;	    // LSR Value
	uint8_t dummy = dummy;	// dummy variable to clear interrupt upon LSR error
	LPC_UART_TypeDef * pUart = (LPC_UART_TypeDef *)LPC_UART0;

	// Reading IIR automatically acknowledges the interrupt
	IIR_IntId = (pUart->IIR) >> 1 ; // skip pending bit in IIR

	if (IIR_IntId & IIR_RDA) {  // Receive Data Avaialbe
	    // Note: read RBR will clear the interrupt
	    g_UART0_buffer[g_UART0_count++] = pUart->RBR; // read from the uart
	    if ( g_UART0_count == BUFSIZE ) {
		    g_UART0_count = 0;  // buffer overflow
	    }	
	} else if (IIR_IntId & IIR_THRE) {  // THRE Interrupt, transmit holding register empty
	    LSR_Val = pUart->LSR;
	    if(LSR_Val & LSR_THRE) {
	        g_UART0_TX_empty = 1;	// UART is ready to transmit 
	    } else {  
	        g_UART0_TX_empty = 0;  // UART is not ready to transmit yet
		}
	    
	} else if (IIR_IntId & IIR_RLS) {
	    LSR_Val = pUart->LSR;
		if (LSR_Val  & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) ) {
		    // There are errors or break interrupt 
	        // Read LSR will clear the interrupt 
	        dummy = pUart->RBR; //Dummy read on RX to clear interrupt, then bail out
			return ;    // error occurs, return
		}
		// If no error on RLS, normal ready, save into the data buffer.
	    // Note: read RBR will clear the interrupt 
		if (LSR_Val & LSR_RDR) { // Receive Data Ready
		    g_UART0_buffer[g_UART0_count++] = pUart->RBR; // read from the uart
	        if ( g_UART0_count == BUFSIZE ) {
		        g_UART0_count = 0;  // buffer overflow
	        }	
		}	    
	} else {  // IIR_CTI and reserved combination are not implemented yet
	    return;
	}	
}

void uart_send_string( uint32_t n_uart, uint8_t *p_buffer, uint32_t len )
{
    LPC_UART_TypeDef *pUart;

    if(n_uart == 0 ) {  // UART0 is implemented
        pUart = (LPC_UART_TypeDef *) LPC_UART0;
    } else {  // other UARTs are not implemented
        return;
    }

    while ( len != 0 ) {
	    // THRE status, contain valid data  
	    while ( !(g_UART0_TX_empty & 0x01) );	
	    pUart->THR = *p_buffer;
		g_UART0_TX_empty = 0;  // not empty in the THR until it shifts out
	    p_buffer++;
	    len--;
    }
    return;
}

int uart_put_char(int n_uart, unsigned char c)
{
	uint8_t buf[1];
	buf[0] = c;

	LPC_UART0->IER = IER_THRE | IER_RLS;			// Disable RBR 
	uart_send_string(n_uart, buf, 1);
	LPC_UART0->IER = IER_THRE | IER_RLS | IER_RBR;	// Re-enable RBR
}

int uart_put_string(int n_uart, unsigned char *s)
{
    if (n_uart >1 ) return -1;	// only uart0 is supported for now
	while (*s !=0) { // loop through each char in the string
		uart_put_char(n_uart, *s++);  // print the char, then ptr increments
	}
	return 0;
}

