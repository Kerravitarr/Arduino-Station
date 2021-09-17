/*
 * DriverIO.inc
 *
 *  Created: 18.05.2021 9:44:37
 *   Author: Terran
 */ 

O:
	LDI 	ZL,low(WordOut)
	LDI 	ZH,high(WordOut)
	ADIW	ZL,WordOutSize-1

	LDIl	Counter2,WordOutSize
	LDI 	Counter, 8

	CLI
		LD	 	tmp1, Z
		cbi		Serial_parallel_interface_Port,Serial_parallel_interface_Update
		O04:
				cbi		Serial_parallel_interface_Port,Serial_parallel_interface_Strob
				LSL		tmp1
				BRCS O01
					cbi		Serial_parallel_interface_Port,Serial_parallel_interface_Data_O
					RJMP	O02
				O01:
					sbi		Serial_parallel_interface_Port,Serial_parallel_interface_Data_O
				O02:
				sbi		Serial_parallel_interface_Port,Serial_parallel_interface_Strob
				DEC		Counter
			BRNE 	O04			;Прошли слово

			LD	 	tmp1, -Z
			LDI 	Counter, 8
			DEC 	Counter2
		BRNE 	O04			;Прошли фразу
		SBI		Serial_parallel_interface_Port,Serial_parallel_interface_Update
	SEI
RET

I:		
	LDI 	ZL,low(WordIn)
	LDI 	ZH,high(WordIn)

	LDIl	Counter2,WordInSize
	LDI 	Counter, 8

	CLI
		ANDI 	tmp2, 0
		CBI		Serial_parallel_interface_Port,Serial_parallel_interface_Update
		SBI		Serial_parallel_interface_Port,Serial_parallel_interface_Update
		I04:
				cbi		Serial_parallel_interface_Port,Serial_parallel_interface_Strob
				IN		tmp1,Serial_parallel_interface_Pin
				ANDI	tmp1,(1 << Serial_parallel_interface_Data_I)
				BRNE I01
					RJMP	I02
				I01:
					ORI		tmp2,1
				I02:

				ROR		tmp2
				sbi		Serial_parallel_interface_Port,Serial_parallel_interface_Strob
				DEC		Counter
			BRNE 	I04			;Прошли слово
			ROR		tmp2

			ST	 	Z+,tmp2
			LDI 	Counter, 8
			DEC 	Counter2
		BRNE 	I04			;Прошли фразу
	SEI
RET

EEPROM_Start:

	TSTFlag EEPROM_write
	BREQ /*=*/ EEPROMrdy_st_read//Чтение
		//Запись
		LDS tmp1,EEPROMPos_c
		DEC tmp1				//Вот и записали байт
		STS EEPROMPos_c,tmp1 

		LDS	Zl,EEPROMPos_s
		CLR Zh
		OUT 	EEARL,ZL 		; Загружаем адрес нужной ячейки
		OUT 	EEARH,ZH  		; старший и младший байт адреса

		INC Zl
		STS EEPROMPos_s,Zl		;В следующий раз адрес будет следующим

		LDI 	ZL,low(EEPROMStr) ;Писать будем в это место
		LDI 	ZH,high(EEPROMStr)		

		LD		tmp1,Z
		OUT 	EEDR,tmp1 	; Записываем, собственно, данные
 
		SBI 	EECR,EEMPE 		; взводим предохранитель
		SBI 	EECR,EEPE 		; записываем байт

		lds 	tmp1, EEPROMPos_c
		EEPROMrdy_s_left:				;Сдвинули буфер на один шаг
			ldd 	tmp2, Z+1 			;Взяли следующий пункт
			st 		Z+, tmp2			;Сохранили в текущий
			dec 	tmp1		
		brne 	EEPROMrdy_s_left		; Loop
		RJMP EEPROM_Start_end
	EEPROMrdy_st_read:
		//Чтение
		//Оно в прямом эфире происходит автоматом
	EEPROM_Start_end:
	SBI 	EECR,EERIE	;Устанавливаем прерывание
RET

 TWI_IO_End:
		LDI 	Zl,low(TWI_IO)
		LDI 	Zh,high(TWI_IO)
		LD		tmp1,Y
		/*CPI		OSRG,AddrDev_R
		BRNE	TIE01
			RCALL TWI_IO_End_AddrDev_R
			JMP TIE00
		TIE01:*/

		TIE00:
			CBIFlag TWI_Busy
RET


.equ CRC_EOP   =  0x20 //ok it is a space...
.equ STK_OK    =  0x10
.equ STK_FAILED=  0x11
.equ STK_UNKNOWN= 0x12
.equ STK_INSYNC=  0x14
.equ STK_NOSYNC=  0x15


UART_End:
	LDS	tmp2,UART_I
	LDS tmp1,Mode
	CPI tmp1,DefaultMode
	BRNE /*!=*/ UART_End_notDefMode
		RCALL UART_End_SET_MODE
		RJMP UART_End_00
	UART_End_notDefMode:
		RCALL UART_End_SET_MODE_work
	UART_End_00:
RET

//Программа выбора мода, активируется, если мода не было, а тут пришёл первый байт
UART_End_SET_MODE:
	LDS	tmp2,UART_I
	//Ищем режим, в котором мы находимся
	CPI	tmp2,0x00 //Мой собственный мод
	BRNE /*!=*/ UART_End_0x00
		STI Mode,MY_SETUP
		RJMP UART_End_SET_MODE_end
	UART_End_0x00:
	CPI	tmp2,CRC_EOP 
	CPI	tmp2,0x20 //Всё ок, конец передачи AVRISP
	BRNE /*!=*/ UART_End_0x20
		RJMP UART_End_SET_MODE_end
	UART_End_0x20:
	CPI	tmp2,0x30 //Get Synchronization
	BRNE /*!=*/ UART_End_0x30
		STI Mode,AVRISP_EMPTY_REPL
		RJMP UART_End_SET_MODE_end
	UART_End_0x30:
	CPI	tmp2,0x31 
	CPI	tmp2,0x41
	BRNE /*!=*/ UART_End_0x41
		STI Mode,AVRISP_GET_PARAMETER
		RJMP UART_End_SET_MODE_end
	UART_End_0x41:
	CPI	tmp2,0x42 
	BRNE /*!=*/ UART_End_0x42
		STI Mode,AVRISP_SET_DEVICE
		STI AVRISP_c,0 //Начинаем вычитывать
		RJMP UART_End_SET_MODE_end
	UART_End_0x42:
	CPI	tmp2,0x45 
	BRNE /*!=*/ UART_End_0x45
		STI Mode,AVRISP_SET_DEVICE_EXT
		STI AVRISP_c,0 //Начинаем вычитывать
		RJMP UART_End_SET_MODE_end
	UART_End_0x45:
	CPI	tmp2,0x50 
	BRNE /*!=*/ UART_End_0x50
		SetTask TS_StartProgrammMode
		STI Mode,AVRISP_STK_ENTER_PROGMODE
		RJMP UART_End_SET_MODE_end
	UART_End_0x50:
	CPI	tmp2,0x51 
	BRNE /*!=*/ UART_End_0x51
		//Конец программированию
		STI Mode,AVRISP_EMPTY_REPL

		AVR_ISP_NOT_RST
			
		IN tmp1,SPI_DDR
		ANDI tmp1,~((1 << SPI_SCK_PIN)|(1 << SPI_MOSI_PIN)|(1 << SPI_MISO_PIN)|(1 << SPI_SS_PIN))
		OUT SPI_DDR,tmp1

		IN tmp1,SPI_PORT
		ANDI tmp1,~((1 << SPI_SCK_PIN)|(1 << SPI_MOSI_PIN)|(1 << SPI_MISO_PIN)|(1 << SPI_SS_PIN))
		OUT SPI_PORT,tmp1
		RJMP UART_End_SET_MODE_end
	UART_End_0x51:
	CPI	tmp2,0x55 
	BRNE /*!=*/ UART_End_0x55
		STI Mode,AVRISP_STK_LOAD_ADDRESS
		STI AVRISP_c,0 //Начинаем вычитывать
		RJMP UART_End_SET_MODE_end
	UART_End_0x55:
	CPI	tmp2,0x56 
	BRNE /*!=*/ UART_End_0x56
		STI Mode,AVRISP_STK_UNIVERSAL
		STI AVRISP_c,0 //Начинаем вычитывать
		RJMP UART_End_SET_MODE_end
	UART_End_0x56:
	CPI	tmp2,0x60 
	CPI	tmp2,0x61 
	CPI	tmp2,0x64 
	BRNE /*!=*/ UART_End_0x64
		STI Mode,AVRISP_STK_PROG_PAGE
		STI AVRISP_c,0 //Начинаем вычитывать
		RJMP UART_End_SET_MODE_end
	UART_End_0x64:
	CPI	tmp2,0x74 
	BRNE /*!=*/ UART_End_0x74
		STI Mode,AVRISP_STK_READ_PAGE
		STI AVRISP_c,0 //Начинаем вычитывать
		RJMP UART_End_SET_MODE_end
	UART_End_0x74:
	UART_End_SET_MODE_end:
RET

//Непосредственно обработка каждого режима из мода
UART_End_SET_MODE_work:
	LDS tmp1,Mode
	LDS	tmp2,UART_I
	CPI tmp1,AVRISP_SET_DEVICE
	BRNE /*!=*/ UART_End_notAVRISP_SET_DEVICE
		LDS tmp1,AVRISP_c
		CPI tmp1,20-1
		BREQ /*=*/ UART_End_AVRISP_00
			RCALL AVRISP_readInBuf
			RJMP UART_End_SET_MODE_work_end
		UART_End_AVRISP_00:
			//Закончили вычитывание символов
			LDS tmp1,AVRISP
			CPI tmp1,0xe0
			BRLO /*<*/ UART_End_AVRISP_01
				SBIFlag rst_active_high
				RJMP UART_End_AVRISP_02
			UART_End_AVRISP_01:
				CBIFlag rst_active_high
			UART_End_AVRISP_02:
			LDI tmp1,0xFF
			LDS tmp2,AVRISP+13 //low
			SBRC tmp2,5 //32
				LDI tmp1,0xF0
			SBRC tmp2,6 //64
				LDI tmp1,0xE0
			SBRC tmp2,7 //128
				LDI tmp1,0xC0
			LDS tmp2,AVRISP+12 //high
			SBRC tmp2,1 //256
				LDI tmp1,0x80
			STS AVRISP_pagesize,tmp1
			STI Mode,AVRISP_EMPTY_REPL
			RJMP UART_End_SET_MODE_work_end
	UART_End_notAVRISP_SET_DEVICE:
	
	CPI tmp1,AVRISP_SET_DEVICE_EXT
	BRNE /*!=*/ UART_End_notAVRISP_SET_DEVICE_EXT
		LDS tmp1,AVRISP_c
		CPI tmp1,5-1-1 //Хз откуда ещё -1
		BREQ /*=*/ UART_End_AVRISP_000
			/*LDI 	ZL,low(AVRISP)
			LDI 	ZH,high(AVRISP) //Полностью игнорируем
			ADD		ZL,tmp1
			SBCI	ZH, 0
			ST Z,tmp2*/
			INC tmp1
			STS AVRISP_c,tmp1
			RJMP UART_End_SET_MODE_work_end
		UART_End_AVRISP_000:
			//Закончили вычитывание символов
			STI Mode,AVRISP_EMPTY_REPL
			RJMP UART_End_SET_MODE_work_end
	UART_End_notAVRISP_SET_DEVICE_EXT:
	
	CPI tmp1,AVRISP_STK_UNIVERSAL
	BRNE /*!=*/ UART_End_notAVRISP_STK_UNIVERSAL
		LDS tmp1,AVRISP_c
		CPI tmp1,4-1+1 //Вычитываем все байты, даже 0х20 - конец!
		BREQ /*=*/ UART_End_AVRISP_0000
			//Вычитываем сразу в сообщение отправки
			LDI 	ZL,low(SPIBUF)
			LDI 	ZH,high(SPIBUF)
			ADD		ZL,tmp1
			SBCI	ZH, 0
			ST Z,tmp2
			INC tmp1
			STS AVRISP_c,tmp1
			RJMP UART_End_SET_MODE_work_end
		UART_End_AVRISP_0000:
			//Закончили вычитывание символов
			SetTask TS_SPI
			STI SPIBUF_l,0x04
			RJMP UART_End_SET_MODE_work_end
	UART_End_notAVRISP_STK_UNIVERSAL:

	CPI tmp1,AVRISP_STK_LOAD_ADDRESS
	BRNE /*!=*/ UART_End_notAVRISP_STK_LOAD_ADDRESS
		LDS tmp1,AVRISP_c
		CPI tmp1,0
		BRNE /*!=*/ UART_End_notOne
			STS LOAD_ADDRESS,tmp2
			RJMP UART_End_notTwo
		UART_End_notOne:
			STS LOAD_ADDRESS+1,tmp2
			STI Mode,AVRISP_EMPTY_REPL
			RJMP UART_End_SET_MODE_work_end
		UART_End_notTwo:
		INC tmp1
		STS AVRISP_c,tmp1
		RJMP UART_End_SET_MODE_work_end
	UART_End_notAVRISP_STK_LOAD_ADDRESS:

	CPI tmp1,AVRISP_STK_PROG_PAGE
	BRNE /*!=*/ UART_End_notAVRISP_STK_PROG_PAGE
		LDS tmp1,AVRISP_c
		CPI tmp1,0
		BRNE /*!=*/ UART_End_PP_notOne
			STS AVRISP_l,tmp2
			RJMP UART_End_PP_notTwo
		UART_End_PP_notOne:
		CPI tmp1,1
		BRNE /*!=*/ UART_End_PP_notThree
			STS AVRISP_l+1,tmp2
			RJMP UART_End_PP_notTwo
		UART_End_PP_notThree:
			//У нас новый символ, тип памяти
			CPI tmp2,'F'
			BRNE /*!=*/ UART_End_PP_err
				STI Mode,AVRISP_STK_PROG_PAGE_FILL
				STI AVRISP_c,0 //Начинаем вычитывать
				RJMP UART_End_SET_MODE_work_end
			UART_End_PP_err:
				//Мы не умеем работать с такой памятью!!!!
				STI Mode,AVRISP_ERR_MODE
				SetTimerTask TS_AVRISP_err,1
				RJMP UART_End_SET_MODE_work_end
		UART_End_PP_notTwo:
		INC tmp1
		STS AVRISP_c,tmp1
		RJMP UART_End_SET_MODE_work_end
	UART_End_notAVRISP_STK_PROG_PAGE:
	
	CPI tmp1,AVRISP_STK_READ_PAGE
	BRNE /*!=*/ UART_End_notAVRISP_STK_READ_PAGE
		LDS tmp1,AVRISP_c
		CPI tmp1,3-1+1 //Вычитываем все байты, даже 0х20 - конец!
		BREQ /*=*/ UART_End_AVRISP_00000
			//Вычитываем сразу в сообщение отправки
			RCALL AVRISP_readInBuf
			RJMP UART_End_SET_MODE_work_end
		UART_End_AVRISP_00000:
			//Закончили вычитывание символов
			CPI	tmp2,CRC_EOP
			BREQ /*=*/ UART_End_AVRISP_00001
				RCALL AVRISP_empty_reply
				RJMP UART_End_SET_MODE_work_end
			UART_End_AVRISP_00001:
			//Даже CRC Совпал!
			//Посылаем команду передать CRC
			UARTPrint STK_INSYNC
			
			STI AVRISP_c,0 //Обнуляем счётчик, на всякий случай
			STI AVRISP_c+1,0
			LDS tmp1,AVRISP
			STS AVRISP_l,tmp1 //Сохранили длину, сколько нам надо прочитать
			LDS tmp1,AVRISP+1
			STS AVRISP_l+1,tmp1
			LDS tmp1,AVRISP+2
			CPI tmp1,'F'
			BREQ /*=*/ UART_End_AVRISP_flash_read_page
			CPI tmp1,'E'
			BREQ /*=*/ UART_End_AVRISP_eeprom_read_page
				UARTPrint STK_FAILED
				STI Mode,DefaultMode
			UART_End_AVRISP_flash_read_page:
				SetTask TS_AVRISP_flash_read_page
				STI Mode,AVRISP_STK_READ_PAGE_FLASH
			RJMP UART_End_SET_MODE_work_end
			UART_End_AVRISP_eeprom_read_page:
				SetTask TS_AVRISP_eeprom_read_page
				STI Mode,AVRISP_STK_READ_PAGE_EEPROM
			RJMP UART_End_SET_MODE_work_end
	UART_End_notAVRISP_STK_READ_PAGE:

	CPI tmp1,AVRISP_GET_PARAMETER
	BRNE /*!=*/ UART_End_notAVRISP_GET_PARAMETER
		CPI tmp2,0x81
		BRNE /*!=*/ UART_End_AVRISP_81
			LDI MacroR,1
			RCALL AVRISP_byte_reply
			RJMP UART_End_SET_MODE_work_end
		UART_End_AVRISP_81:
		CLR MacroR
		RCALL AVRISP_byte_reply
		RJMP UART_End_SET_MODE_work_end
	UART_End_notAVRISP_GET_PARAMETER:

	CPI tmp1,AVRISP_STK_PROG_PAGE_FILL
	BRNE /*!=*/ UART_End_notAVRISP_STK_PROG_PAGE_FILL
		LDS tmp1,AVRISP_c
		LDS tmp3,AVRISP_l+1
		CP tmp1,tmp3
		BREQ /*=*/ UART_End_AVRISP_11
			RCALL AVRISP_readInBuf
			RJMP UART_End_SET_MODE_work_end
		UART_End_AVRISP_11:
			//Закончили вычитывание символов
			CPI tmp2,CRC_EOP
			BRNE /*!=*/ UART_End_AVRISP_11_ErrCRC
				UARTPrint STK_INSYNC
				STI AVRISP_c,0 //Обнуляем счётчик, на всякий случай
				STI AVRISP_c+1,0
				LDS tmp1,LOAD_ADDRESS //Старший
				STS AVRISP_page,tmp1
				LDS tmp1,LOAD_ADDRESS+1 //Младший
				LDS tmp2, AVRISP_pagesize
				AND tmp1,tmp2
				STS AVRISP_page+1,tmp1
				SetTask TS_AVRISP_write_flash_pages
				RJMP UART_End_SET_MODE_work_end
			UART_End_AVRISP_11_ErrCRC:
				RCALL AVRISP_empty_reply
			RJMP UART_End_SET_MODE_work_end
	UART_End_notAVRISP_STK_PROG_PAGE_FILL:

	CPI tmp1,AVRISP_EMPTY_REPL
	BRNE /*!=*/ UART_End_notAVRISP_EMPTY_REPL
		RCALL AVRISP_empty_reply
		RJMP UART_End_SET_MODE_work_end
	UART_End_notAVRISP_EMPTY_REPL:

	CPI tmp1,AVRISP_STK_ENTER_PROGMODE
	BRNE /*!=*/ UART_End_notAVRISP_STK_ENTER_PROGMODE
		RJMP UART_End_SET_MODE_work_end
	UART_End_notAVRISP_STK_ENTER_PROGMODE:

	CPI tmp1,AVRISP_ERR_MODE
	BRNE /*!=*/ UART_End_notAVRISP_ERR_MODE
		 //У нас ошибка, ждём пока всё пройдёт
		SetTimerTask TS_AVRISP_err,1
		RJMP UART_End_SET_MODE_work_end_print
	UART_End_notAVRISP_ERR_MODE:

	CPI tmp1,MY_SETUP
	BRNE /*!=*/ UART_End_notMY_SETUP
		RCALL UART_End_SET_MODE_work_MyMode
		RJMP UART_End_SET_MODE_work_end
	UART_End_notMY_SETUP:

	CPI tmp1,MY_SETUP_SET_BAUD
	BRNE /*!=*/ UART_End_notMY_SETUP_SET_BAUD
		LDS tmp1,AVRISP_c
		CPI tmp1,1
		BRNE /*!=*/ UART_End_SSB_notOne
			LDS tmp2,AVRISP_l+1
			STS EEPROMStr+1,tmp2
			LDS tmp2,AVRISP_l
			STS EEPROMStr,tmp2
			STI EEPROMPos_s,EEPROM_BAUD
			STI EEPROMPos_c,2
			SBIFlag EEPROM_write
			SetTask TS_EEPROM_Start
			SetTimerTask TS_Reset,1000
			RJMP UART_End_SET_MODE_work_end
		UART_End_SSB_notOne:
			STS AVRISP_l,tmp2
		UART_End_SSB_notEnd:
		INC tmp1
		STS AVRISP_c,tmp1
		RJMP UART_End_SET_MODE_work_end
	UART_End_notMY_SETUP_SET_BAUD:
	
	CPI tmp1,MY_SETUP_SET_PORTB
	BRNE /*!=*/ UART_End_notMY_SETUP_SET_PORTB
	//PORTx (Управление состоянием выходов), DDRx(Настройка разрядов порта x на вход или выход.).
		RCALL readNext_AVRISP
		LDS tmp1,AVRISP_c
		CPI tmp1,2
		BRNE /*!=*/ UART_End_SET_MODE_work_end_bridg
			LDS tmp2,AVRISP
			OUT PORTB,tmp2
			LDS tmp2,AVRISP+1
			OUT DDRB,tmp2
			STI Mode,DefaultMode
			UARTPrint STK_OK
		RJMP UART_End_SET_MODE_work_end
	UART_End_notMY_SETUP_SET_PORTB:

	CPI tmp1,MY_SETUP_SET_PORTC
	BRNE /*!=*/ UART_End_notMY_SETUP_SET_PORTC
	//PORTx (Управление состоянием выходов), DDRx(Настройка разрядов порта x на вход или выход.).
		RCALL readNext_AVRISP
		LDS tmp1,AVRISP_c
		CPI tmp1,2
		BRNE /*!=*/ UART_End_SET_MODE_work_end_bridg
			LDS tmp2,AVRISP
			OUT PORTC,tmp2
			LDS tmp2,AVRISP+1
			OUT DDRC,tmp2
			STI Mode,DefaultMode
			UARTPrint STK_OK
		RJMP UART_End_SET_MODE_work_end
	UART_End_notMY_SETUP_SET_PORTC:

	CPI tmp1,MY_SETUP_SET_PORTD
	BRNE /*!=*/ UART_End_notMY_SETUP_SET_PORTD
	//PORTx (Управление состоянием выходов), DDRx(Настройка разрядов порта x на вход или выход.).
		RCALL readNext_AVRISP
		LDS tmp1,AVRISP_c
		CPI tmp1,2
		BRNE /*!=*/ UART_End_SET_MODE_work_end_bridg
			LDS tmp2,AVRISP
			OUT PORTD,tmp2
			LDS tmp2,AVRISP+1
			OUT DDRD,tmp2
			STI Mode,DefaultMode
			UARTPrint STK_OK
		RJMP UART_End_SET_MODE_work_end
					UART_End_SET_MODE_work_end_bridg: RJMP UART_End_SET_MODE_work_end
	UART_End_notMY_SETUP_SET_PORTD:

	CPI tmp1,MY_SETUP_SET_ADC
	BRNE /*!=*/ UART_End_notMY_SETUP_SET_ADC
	//ADCSRA, ADMUX, каналы
		RCALL readNext_AVRISP
		LDS tmp1,AVRISP_c
		CPI tmp1,3
		BRNE /*!=*/ UART_End_SET_MODE_work_end
			LDS tmp2,AVRISP
			//ADCSRA: ADEN ADSC ADATE ADIF ADIE ADPS2 ADPS1 ADPS
					;  1 - вкл
					;		1 - запуск преобразования
					;			1 - позволяет запускать преобразование по прерыванию от переферийных устройств микроконтроллера
					;					х - Флаг прерываний
					;						1 - разрешает прерывания от АЦП
					;							0		0	0 - CLK/2 - частота работы (8МГц)
					;							0		0	1 - CLK/2 - частота работы (8МГц)
					;							0		1	0 - CLK/4 - частота работы (4МГц)
					;							0		1	1 - CLK/8 - частота работы (2МГц)
					;							1		0	0 - CLK/16 - частота работы (1МГц)
					;							1		0	1 - CLK/32 - частота работы (500кГц)
					;							1		1	0 - CLK/64 - частота работы (250кГц)
					;							1		1	1 - CLK/128 - частота работы (125кГц)
			UOUT ADCSRA,tmp2
			LDS tmp2,AVRISP+1
			//ADMUX: REFS1 REFS0 ADLAR - MUX3 MUX2 MUX1 MUX0
					; 0       0 - опорное напряжение на входе AREF
					; 0       1 - Vпитания (вход AREF должен быть отключен. или к нему можно подключить фильтрующий конденсатор)
					; 1       1 - внутренний ИОН 1.1 В (к входу AREF можно подключить фильтрующий конденсатор)
					;				1 -  выравнивать результат преобразования по левому краю ADCH:ADCL [9][8][7][6][5][4][3][2]:[1][0][x][x][x][x][x][x]
					;						0	0	0	0 - вход ADC0
					;						0	0	0	1 - вход ADC1
					;						0	0	1	0 - вход ADC2
					;						0	0	1	1 - вход ADC3
					;						0	1	0	0 - вход ADC4
					;						0	1	0	1 - вход ADC5
					;						0	1	1	0 - вход ADC6
					;						0	1	1	1 - вход ADC7
					;						1	0	0	0 - датчик температуры
					;						1	1	1	0 - 1.1 В
					;						1	1	1	1 - 0 В (земля)
			UOUT ADMUX,tmp2
			LDS tmp2,AVRISP+2
			STS ADC_chanel,tmp2

			TST tmp2 //Проверим, а не 0 ли тут?
			BREQ /*=*/ UART_End_SET_MODE_work_end//МЫ НИ КОГДА НЕ ДОЛЖНЫ ТУДА ПОПАДАТЬ!!!

			STI Mode,DefaultMode
			UARTPrint STK_OK
		RJMP UART_End_SET_MODE_work_end
	UART_End_notMY_SETUP_SET_ADC:
	

	UART_End_SET_MODE_work_end:
RET
	UART_End_SET_MODE_work_end_print:
	SetTask TS_UART_PrintExo
RET

//Читает следующий байт из УАРТ
//ВНИМАНИЕ!!!! РЕГИСТРЫ!!!!
//tmp2 - Значение, которое надо записать
//ИСПОЛЬЗУЕТСЯ РЕГИСТРОВАЯ ПАРА Z
readNext_AVRISP:
	PUSH tmp1
	LDS tmp1,AVRISP_c
	LDI 	ZL,low(AVRISP)
	LDI 	ZH,high(AVRISP)
	ADD		ZL,tmp1
	SBCI	ZH, 0
	ST		Z,tmp2
	INC tmp1
	STS AVRISP_c,tmp1
	POP tmp1
RET

//Выбор моего собственного мода
UART_End_SET_MODE_work_MyMode:
	LDS	tmp2,UART_I
	//Ищем режим, в котором мы находимся
	CPI	tmp2,0x03
	BRNE /*!=*/ notMode_0x03
		STI Mode,MY_SETUP_SET_BAUD
		STI AVRISP_c,0 //Начинаем вычитывать
		RJMP UART_End_SET_MODE_work_MyMode_end
	notMode_0x03:
	CPI	tmp2,0x04
	BRNE /*!=*/ notMode_0x04
		STI Mode,MY_SETUP_SET_PORTB
		STI AVRISP_c,0 //Начинаем вычитывать
		RJMP UART_End_SET_MODE_work_MyMode_end
	notMode_0x04:
	CPI	tmp2,0x05
	BRNE /*!=*/ notMode_0x05
		STI Mode,MY_SETUP_SET_PORTC
		STI AVRISP_c,0 //Начинаем вычитывать
		RJMP UART_End_SET_MODE_work_MyMode_end
	notMode_0x05:
	CPI	tmp2,0x06
	BRNE /*!=*/ notMode_0x06
		STI Mode,MY_SETUP_SET_PORTD
		STI AVRISP_c,0 //Начинаем вычитывать
		RJMP UART_End_SET_MODE_work_MyMode_end
	notMode_0x06:
	CPI	tmp2,0x07
	BRNE /*!=*/ notMode_0x07
		STI Mode,MY_SETUP_SET_ADC
		STI AVRISP_c,0 //Начинаем вычитывать
		RJMP UART_End_SET_MODE_work_MyMode_end
	notMode_0x07:
	CPI	tmp2,0x20
	BRNE /*!=*/ notMode_0x20 //Проверка связи
		STI Mode,DefaultMode
		UARTPrint STK_OK
		RJMP UART_End_SET_MODE_work_MyMode_end
	notMode_0x20:

	UARTPrint STK_FAILED
	STI Mode,DefaultMode
	UART_End_SET_MODE_work_MyMode_end:
RET

//В tmp1 - номер элемента
//В tmp2 - то, что нужно записать!!!
AVRISP_readInBuf:
	LDI 	ZL,low(AVRISP)
	LDI 	ZH,high(AVRISP)
	ADD		ZL,tmp1
	SBCI	ZH, 0
	ST Z,tmp2
	INC tmp1
	STS AVRISP_c,tmp1
RET

AVRISP_empty_reply:
	STI Mode,DefaultMode
	LDS	tmp2,UART_I
	CPI	tmp2,CRC_EOP
	BRNE /*!=*/ UART_End_AVRISP_0
		UARTPrint STK_INSYNC
		UARTPrint STK_OK
		RET
	UART_End_AVRISP_0:
		UARTPrint STK_NOSYNC
RET

AVRISP_byte_reply:
	PUSH MacroR
	STI Mode,DefaultMode
	UARTPrint STK_INSYNC
	POP MacroR
	CALL UART_Print
	UARTPrint STK_OK
RET
/*	AVRISP_byte_reply_0:
		STI	UART_Ol,1
		STI	UART_O+0,STK_NOSYNC
RET*/

UART_PrintExo:
	LDS		MacroR,UART_I
	CALL UART_Print
	UARTPrint '\n'
RET

SPI_End:
	LDS tmp1,Mode
	CPI tmp1,AVRISP_STK_UNIVERSAL
	BRNE /*!=*/SPI_End_notAVRISP_STK_UNIVERSAL
		//Стандартный мод, нам надо отправить последний байт - 4й
		LDS MacroR,SPIBUF+3
		RCALL AVRISP_byte_reply
		RJMP SPI_End_end
	SPI_End_notAVRISP_STK_UNIVERSAL:
	CPI tmp1,AVRISP_STK_ENTER_PROGMODE
	BRNE /*!=*/SPI_End_notAVRISP_STK_ENTER_PROGMODE
		STI	UART_I,CRC_EOP
		RCALL AVRISP_empty_reply
		RJMP SPI_End_end
	SPI_End_notAVRISP_STK_ENTER_PROGMODE:
	
	CPI tmp1,AVRISP_STK_PROG_PAGE_FILL
	BREQ /*=*/ SPI_End_AVRISP_STK_PROG_PAGE_FILL
	CPI tmp1,AVRISP_STK_READ_PAGE_FLASH
	BREQ /*=*/ SPI_End_AVRISP_STK_READ_PAGE_FLASH
	CPI tmp1,AVRISP_STK_READ_PAGE_EEPROM
	BRNE /*!=*/SPI_End_notAVRISP_STK_READ_PAGE_EEPROM_bridge
		SPI_End_AVRISP_STK_READ_PAGE_FLASH:
		//Стандартный мод, нам надо отправить последний байт - 4й
		LDS MacroR,SPIBUF+3
		CALL UART_Print
		SPI_End_AVRISP_STK_PROG_PAGE_FILL: //Всё тоже самое, но без печати символа
		LDS Zh,AVRISP_c
		LDS Zl,AVRISP_c+1 //Младший!
		ADIW Zl,1
		LDS tmp1,AVRISP_l
		LDS tmp2,AVRISP_l+1 //Младший!
		CP Zl,tmp2
		CPC Zh,tmp1
		BRSH /*>=*/ SPI_End_END_RPEEPROM
			STS AVRISP_c,Zh
			STS AVRISP_c+1,Zl
			LDS tmp1,Mode
			CPI tmp1,AVRISP_STK_READ_PAGE_FLASH
			BREQ /*=*/ SPI_End_RP_flash
			CPI tmp1,AVRISP_STK_PROG_PAGE_FILL
			BREQ /*=*/ SPI_End_PP_flash
				SetTask TS_AVRISP_eeprom_read_page
				RJMP SPI_End_end
			SPI_End_RP_flash:
				SetTask TS_AVRISP_flash_read_page
				RJMP SPI_End_end
	SPI_End_notAVRISP_STK_READ_PAGE_EEPROM_bridge: RJMP SPI_End_notAVRISP_STK_READ_PAGE_EEPROM
			SPI_End_PP_flash:
				LDS tmp1,LOAD_ADDRESS //Старший
				LDS tmp2,AVRISP_page //Старший
					CPSE tmp1,tmp2
						RJMP SPI_End_PP_updateFlash
					//Старшие части равны
					LDS tmp1,LOAD_ADDRESS+1 //Младший
					LDS tmp2, AVRISP_pagesize
					AND tmp1,tmp2
					LDS tmp2,AVRISP_page+1 //Младший
					CPSE tmp1,tmp2
						RJMP SPI_End_PP_updateFlash
					//Равны старшие и младшие, страницу не обновляем
					SetTask TS_AVRISP_write_flash_pages
				RJMP SPI_End_end
				SPI_End_PP_updateFlash:
					//Надо обновить страницу
					SetTask TS_AVRISP_commitPage
				RJMP SPI_End_end
			RJMP SPI_End_end
		SPI_End_END_RPEEPROM:
			LDS tmp1,Mode
			CPI tmp1,AVRISP_STK_PROG_PAGE_FILL
			BRNE /*!=*/ SPI_End_END_RPEEPROM_OK
				//У нас режим программирования
				//Надо обновить страницу
				SetTask TS_AVRISP_commitPage
				RJMP SPI_End_end
			SPI_End_END_RPEEPROM_OK:
				UARTPrint STK_OK
				STI Mode,DefaultMode
		RJMP SPI_End_end
	SPI_End_notAVRISP_STK_READ_PAGE_EEPROM:

	
	CPI tmp1,AVRISP_STK_PROG_PAGE_COMMIT
	BRNE /*!=*/SPI_End_notAVRISP_STK_PROG_PAGE_COMMIT
		LDS Zh,AVRISP_c
		LDS Zl,AVRISP_c+1 //Младший!
		ADIW Zl,1
		LDS tmp1,AVRISP_l
		LDS tmp2,AVRISP_l+1 //Младший!
		CP Zl,tmp2
		CPC Zh,tmp1
		BRSH /*>=*/ SPI_End_PP_commit_end
			//У нас будет следующая часть
			LDS tmp1,LOAD_ADDRESS //Старший
			STS AVRISP_page,tmp1
			LDS tmp1,LOAD_ADDRESS+1 //Младший
			LDS tmp2, AVRISP_pagesize
			AND tmp1,tmp2
			STS AVRISP_page+1,tmp1
			STI Mode,AVRISP_STK_PROG_PAGE_FILL
			SetTask TS_AVRISP_write_flash_pages
			RJMP SPI_End_end
		SPI_End_PP_commit_end:
			//Закончили программировать
			UARTPrint STK_OK
			STI Mode,DefaultMode
			RJMP SPI_End_end
	SPI_End_notAVRISP_STK_PROG_PAGE_COMMIT:

	SPI_End_end:
	STI SPIBUF_l,0x00
	STI SPIBUF_c,0x00
RET

//Окончание чтения из энергонезависимой памяти
EEPROM_End:
	LDS	tmp1,ModeEEPROM
	//Ищем режим, в котором мы находимся
	CPI	tmp1,EEPROM_READ_UART //Режим чтения из памяти данных по уарту
	BRNE /*!=*/ notEEPROM_READ_UART
		LDS 	tmp1, EEPROMStr+1 //Читаем младшм вперёд
		UOUT 	UBRR0L,tmp1
		LDS 	tmp1, EEPROMStr
		UOUT 	UBRR0H,tmp1
		UARTPrint CRC_EOP
		RJMP EEPROM_End_def
	notEEPROM_READ_UART:


	LDS tmp1,Mode
	CPI tmp1,MY_SETUP_SET_BAUD
	BRNE /*!=*/ EEPROM_EndnotMY_SETUP_SET_BAUD //Записали в EEPROM
		STI Mode,DefaultMode 
		UARTPrint STK_OK
	EEPROM_EndnotMY_SETUP_SET_BAUD:


	EEPROM_End_def:
	STI ModeEEPROM,DefaultMode
	EEPROM_End_end:
RET