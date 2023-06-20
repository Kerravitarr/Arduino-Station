/*
 * macro.inc
 *
 *  Created: 11.05.2021 22:47:03
 *   Author: Terran
 */ 
 //Убирает флаг из памяти флагов
 .MACRO CBIFlag
	LDS MacroR,Flag
	ANDI MacroR, ~(1 << @0)
	STS Flag,MacroR
 .ENDMACRO
 //Проверяет наличие флага в памяти флагов
  .MACRO TSTFlag
	LDS MacroR,Flag
	ANDI MacroR,(1 << @0)
 .ENDMACRO
 //Устанавливает флаг в памяти флагов
 .MACRO SBIFlag
	LDS MacroR,Flag
	ORI MacroR, (1 << @0)
	STS Flag,MacroR
 .ENDMACRO

 
  .MACRO 	OUTI				; Ёто описание макроса. ¬езде где outi встретитс¤ в коде, то замен¤етс¤ на 
	LDI 	MacroR,@1 			; этот кусок кода, причем @0,@1 это параметры, они замен¤тс¤ введенными параметрами
	OUT 	@0,MacroR 			; макроса. ?анный макрос тупо копирует введенное число сначала в регистр R16, а из него 
.ENDMACRO
.MACRO    UOUT        
   	.IF	@0 < 0x40
      	OUT	@0,@1         
	.ELSE
      	STS	@0,@1
   	.endif
.ENDMACRO
.MACRO    UIN        
   	.IF	@1 < 0x40
      	IN	@0,@1         
	.ELSE
      	LDS	@0,@1
   	.endif
.ENDMACRO
//Сохраняет число в память. Память=Число
.MACRO STI
	LDI MacroR,@1
	STS @0,MacroR
.ENDMACRO
//Прибавляет число к регистру. Регистр += число
.MACRO		ADI
	SUBI	@0,-@1
.ENDMACRO
//Загружает число в регистр (если это нижний регистр!!!) Регистр=число
 .MACRO LDIl
	LDI MacroR,@1
	MOV @0,MacroR
 .ENDMACRO
 //Печатает в УАРТ символ
 .MACRO UARTPrint
	LDI MacroR,@0
	CALL UART_Print
 .ENDM
 //Печатает в УАРТ символ из памяти
 .MACRO UARTPrint_m
	LDS MacroR,@0
	CALL UART_Print
 .ENDM
 //Печатает в УАРТ символ из регистра
 .MACRO UARTPrint_r
	MOV MacroR,@0
	CALL UART_Print
 .ENDM
 //Печатает в УАРТ символ из регистровой пары
 .MACRO UARTPrint_rm
	LD MacroR,@0
	CALL UART_Print
 .ENDM
 //Макрос для запуска I2С писалки с одним параметром - длиной пакета
.MACRO I2CStart
		STI TWI_IOc, 0			//Обнуляем длину сообщения
		STI TWI_IOl, @0			//Длина передаваемого сообщения
		SetTask TS_StartTWI		//Запускаем запись
.ENDMACRO
 //Макрос для запуска I2С писалки с одним параметром - регистром, где лежит длина пакета
.MACRO I2CStart_r
		STI TWI_IOc, 0			//Обнуляем длину сообщения
		STS TWI_IOl, @0			//Длина передаваемого сообщения
		SetTask TS_StartTWI		//Запускаем запись
.ENDMACRO
 //Макрос для запуска I2С читалки с двумя параметрами - регистром длины данных и регистром-адресом
.MACRO I2CRead_r
		STI TWI_IOc, 0			//Обнуляем длину сообщения
		STS TWI_IOl, @0			//Длина передаваемого сообщения
		STS TWI_IO, @1			//Адрес нашей цели
		SetTask TS_StartTWI		//Запускаем запись
.ENDMACRO

.MACRO SetTask
		ldi MacroR, @0			; Обращение к процедуре через событийный диспетчер
		call SendTask				; 
.ENDMACRO

.MACRO SetTimerTask
		LDI 	MacroR, @0
		LDI 	XL, Low(@1)				;
		LDI 	XH, High(@1)			; ЗАдержка в милисекундах
		CALL 	SetTimer
.ENDM

.MACRO WriteInBuf_Z
		LDI 	ZL,low(@0)
		LDI 	ZH,high(@0)
		MOV		MacroR,@1
.ENDM

//Задежка в микросекундах
.MACRO MK_Delay
	.if (@0) > 0                  ; Если @0 <= 0 -- нет генерации задержки
		.if (@0*F_CPU/1000000)/4 > 250          ; Больше тактов нельзя, выводим ошибку
			.error "@0 must be less or equal 250*4"
		.else
			.if (@0*F_CPU/1000000)/4 >= 1                ; Делим на 4, т.к. 1 итерация цикла -- 4 такта
				ldi     MacroR, (@0*F_CPU/1000000)/4
				WDR						  ; 1 такт
				subi    MacroR, 1         ; 1 такт
				brcc    (PC-2)            ; 2 такта
				; На последнем шаге brcc выполнится за 1 такт, но с учетом ldi опять-таки
				; получится 2 такта
			.endif
			.if (@0*F_CPU/1000000)%4 == 1
				nop
			.endif
			.if (@0*F_CPU/1000000)%4 == 2
				nop
				nop
			.endif
			.if (@0*F_CPU/1000000)%4 == 3
				nop
				nop
				nop
			.endif
		.endif
	.endif
.ENDMACRO

 .MACRO AVR_ISP_RST
	TSTFlag rst_active_high
	BRNE /*!=*/ PC+3
		//Ресет у нас низким уровнем  управляется, а тут инверсия
		SBI	SPI_PORT,SPI_SS_PIN
		RJMP PC+1
		CBI	SPI_PORT,SPI_SS_PIN
.ENDM
.MACRO AVR_ISP_NOT_RST
	TSTFlag rst_active_high
	BRNE /*!=*/ PC+3
		//Ресет у нас низким уровнем  управляется, а тут инверсия
		CBI	SPI_PORT,SPI_SS_PIN
		RJMP PC+1
		SBI	SPI_PORT,SPI_SS_PIN
.ENDM