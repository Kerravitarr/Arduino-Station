/*
 * RTOS.inc
 *
 *  Created: 18.05.2021 9:52:42
 *   Author: Terran
 */ 
 //Кладёт в очередь событий через регистр MacroR
 SendTask:
	push 	ZL
	push 	ZH
	push 	Counter
	push 	tmp1

	ldi 	ZL,low(TaskQueue)		;Загружаем очредеь событий
	ldi 	ZH,high(TaskQueue)

	ldi 	Counter, TaskQueueSize ;Длину этой очереди

	SEQL01: 
		ld 		tmp1, Z+	;Переходим к событию
		cpi 	tmp1, $FF	;Если оно FF - то тут уже пустота, сюда и сохраним
		BRNE /*!=*/ SEQL02
			st 		-Z, MacroR	; Положить в очередь
			RJMP SEQL03
		SEQL02:
		breq 	SEQL02

		dec 	Counter		; А если нет, то ещё один шажок
	BRNE /*!=*/	SEQL01		; Если очередь переполнена, то выходим

	SEQL03:					; EXIT
	pop 	tmp1
	pop 	Counter
	pop 	ZH
	pop 	ZL
ret	

;------------------------------------------------------------------------	
; OSRG - Timer Event
; X - Counter
SetTimer:
	push 	ZL
	push 	ZH
	push 	tmp1
	push 	Counter

	ldi 	ZL,low(TimersPool)
	ldi 	ZH,high(TimersPool)

	ldi 	Counter, TimersPoolSize
	
	STL01: 	
		ld 		tmp1, Z		; Value / Counter
		cp 		tmp1, MacroR		; Поиск в очереди
		BRNE /*!=*/ STL02
			//Такая задача уже есть, обновляем её и выходим
			std 	Z+1, XL		; Critical Section
			std 	Z+2, XH		; Update Counter
			rjmp	STL06		; Exit
		STL02:
	
		subi 	ZL, Low(-3)	; Skip Counter
		sbci 	ZH, High(-3)	; Z+=3

		dec 	Counter		;
	BRNE /*!=*/ STL01 //Всё ищем и ищем
	//Не нашли, а жаль. Значит добавляем в очередь
	ldi 	ZL, low(TimersPool)
	ldi 	ZH, high(TimersPool)

	ldi 	Counter, TimersPoolSize
	
	STL04:	
		ld 		tmp1, Z		; Value / Counter
		cpi 	tmp1, $FF		; Search for Empty Timer
		BRNE /*!=*/ STL05
			st 		Z, MacroR		; Set Event 
			std 	Z+1, XL
			std 	Z+2, XH
			RJMP STL06
		STL05:	
		subi 	ZL, Low(-3)	; Skip Counter
		sbci 	ZH, High(-3)	; Z+=3
		dec 	Counter		;
	BRNE /*!=*/ STL04

	STL06:
	pop 	Counter
	pop 	tmp1
	pop 	ZH
	pop 	ZL
RET	

ClearTaskQueue:
	ldi 	ZL, low(TaskQueue)
	ldi 	ZH, high(TaskQueue)

	ldi 	tmp1, $FF		
	ldi 	Counter, TaskQueueSize

	CEQL01: 
		st 		Z+, tmp1		;
		dec 	Counter		;
	brne 	CEQL01		; Loop
ret
	
ClearTimers:
	ldi 	ZL, low(TimersPool)
	ldi 	ZH, high(TimersPool)

	ldi 	Counter, TimersPoolSize
	ldi 	tmp1, $FF		; Empty 
	ldi 	tmp2, $00

	CTL01:	
		st 		Z+, tmp1		; Event
		st 		Z+, tmp2		; Counter Lo
		st 		Z+, tmp2		; Counter Hi
		dec 	Counter		;
	brne 	CTL01		; Loop
ret	


 ;------------------------------------------------------------------------------
ProcessTaskQueue:
	ldi 	ZL, low(TaskQueue)
	ldi 	ZH, high(TaskQueue)

	ld 		tmp1, Z		; For Event
	cpi 	tmp1, $FF	; No Event or Addr out of Range
	BREQ /*=*/ 	PTQL02	; У нас нет  задач в очереди
	
	clr 	ZH
	lsl 	tmp1
	mov 	ZL, tmp1

	subi 	ZL, low(-TaskProcs*2)
	sbci 	ZH, high(-TaskProcs*2) ; Add
	
	lpm					; mov r0 <- CODE[Z]
	mov 	tmp1, r0
	ld 		r0, Z+		; inc Z
	lpm	
	mov 	ZL, tmp1	; Get Addr
	mov 	ZH, r0
	
	push 	ZL
	push 	ZH

		; Сдвижка очереди на один пункт к началу
		ldi 	Counter, TaskQueueSize-1
		ldi 	ZL, low(TaskQueue)
		ldi 	ZH, high(TaskQueue)
	
		cli
			PTQL01:
				ldd 	tmp1, Z+1 		;Взяли следующий пункт
				st 		Z+, tmp1		;Сохранили в текущий
				dec 	Counter		
			brne 	PTQL01		; Loop
			ldi 	tmp1, $FF	;Добавили в конец пустоту
			st 		Z+, tmp1		
		sei

	pop 	ZH
	pop 	ZL

	ijmp 			; Прыгаем на выполнение задачи
PTQL02:	ret	



TaskProcs: 	.dw Idle
			.equ TS_Idle 				= 0
			.dw TWI_IO_Erro
			.equ TS_TWI_IO_Error 		= 1
			.dw TWI_IO_End
			.equ TS_TWI_IO_End 			= 2
			.dw UART_End
			.equ TS_UART_End 			= 3
			.dw UART_PrintExo
			.equ TS_UART_PrintExo		= 4
			.dw AVRISP_commitPage
			.equ TS_AVRISP_commitPage	= 5
			.dw StartProgrammMode
			.equ TS_StartProgrammMode	= 6
			.dw StartProgrammMode_State1
			.equ TS_StartProgrammMode_State1	= 7
			.dw SPI
			.equ TS_SPI					= 8
			.dw SPI_End
			.equ TS_SPI_End 			= 9
			.dw AVRISP_flash_read_page
			.equ TS_AVRISP_flash_read_page 		= 10
			.dw AVRISP_eeprom_read_page
			.equ TS_AVRISP_eeprom_read_page		= 11
			.dw AVRISP_err
			.equ TS_AVRISP_err					= 12
			.dw AVRISP_write_flash_pages
			.equ TS_AVRISP_write_flash_pages	= 13
			.dw Reset
			.equ TS_Reset						= 14
			.dw EEPROM_Start
			.equ TS_EEPROM_Start				= 15
			.dw EEPROM_End
			.equ TS_EEPROM_End					= 16
			.dw UART_SETUP
			.equ TS_UART_SETUP					= 17
			.dw StartTWI
			.equ TS_StartTWI					= 18	//Запуск передачи I2C

