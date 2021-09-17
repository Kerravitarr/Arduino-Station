/*
 * macroFun.inc
 *
 *  Created: 20.05.2021 9:31:53
 *   Author: Terran
 */ 
 
UART_Print:
	CLI
		RCALL UART_Print_withuotCLI
	SEI
RET

UART_Print_withuotCLI:
	PUSH Zl
	PUSH Zh
	PUSH tmp1
	PUSH MacroR

	LDS tmp1,UART_O_head
	INC tmp1
	CPI tmp1,UARTSize
	BRLO /*<*/ UART_Print_notEnd
		CLR tmp1
	UART_Print_notEnd:
	LDS Zl,UART_O_tail
	CP tmp1,Zl
	BREQ /*=*/ UART_Print_contAdd
		LDI 	ZL,low(UART_O_buf)
		LDI 	ZH,high(UART_O_buf)
		LDS		MacroR,UART_O_head
		ADD		ZL,MacroR
		SBCI	ZH, 0
		POP		MacroR
		ST		Z,MacroR
		STS UART_O_head,tmp1
		RJMP UART_Print_Add
	UART_Print_contAdd:
		POP MacroR
	UART_Print_Add:
	POP tmp1
	POP Zh
	POP Zl
	LDS MacroR,UCSR0B
	SBRC MacroR,UDRIE0
		RJMP UART_Print_UART_Start
	//UART не запущен
	LDI 	MacroR, (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)|(0<<TXCIE0)|(1<<UDRIE0)
	UOUT 	UCSR0B, MacroR
	UART_Print_UART_Start:
RET