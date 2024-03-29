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
			BRNE 	O04			;������ �����

			LD	 	tmp1, -Z
			LDI 	Counter, 8
			DEC 	Counter2
		BRNE 	O04			;������ �����
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
			BRNE 	I04			;������ �����
			ROR		tmp2

			ST	 	Z+,tmp2
			LDI 	Counter, 8
			DEC 	Counter2
		BRNE 	I04			;������ �����
	SEI
RET

EEPROM_Start:

	TSTFlag EEPROM_write
	BREQ /*=*/ EEPROMrdy_st_read//������
		//������
		LDS tmp1,EEPROMPos_c
		DEC tmp1				//��� � �������� ����
		STS EEPROMPos_c,tmp1 

		LDS	Zl,EEPROMPos_s
		CLR Zh
		OUT 	EEARL,ZL 		; ��������� ����� ������ ������
		OUT 	EEARH,ZH  		; ������� � ������� ���� ������

		INC Zl
		STS EEPROMPos_s,Zl		;� ��������� ��� ����� ����� ���������

		LDI 	ZL,low(EEPROMStr) ;������ ����� � ��� �����
		LDI 	ZH,high(EEPROMStr)		

		LD		tmp1,Z
		OUT 	EEDR,tmp1 	; ����������, ����������, ������
 
		SBI 	EECR,EEMPE 		; ������� ��������������
		SBI 	EECR,EEPE 		; ���������� ����

		lds 	tmp1, EEPROMPos_c
		EEPROMrdy_s_left:				;�������� ����� �� ���� ���
			ldd 	tmp2, Z+1 			;����� ��������� �����
			st 		Z+, tmp2			;��������� � �������
			dec 	tmp1		
		brne 	EEPROMrdy_s_left		; Loop
		RJMP EEPROM_Start_end
	EEPROMrdy_st_read:
		//������
		//��� � ������ ����� ���������� ���������
	EEPROM_Start_end:
	SBI 	EECR,EERIE	;������������� ����������
RET
//��������� I2C �� ��������
StartTWI:
	LDI tmp1,(1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWSTA)			/*��������� ���������� � ��������� ��������� �����, ������ - ��� ��!*/
	UOUT TWCR,tmp1
	CLI
		SBIFlag TWI_Busy
	SEI
RET
//�� ��������� ������/������ �� I2C
TWI_IO_End:
	
	LDS tmp1,Mode
	CPI tmp1,MY_SETUP_I2C_W
	BRNE /*!=*/ TWI_IO_EndMY_SETUP_I2C_W	//�� ������ ��������
		UARTPrint STK_OK
		STI Mode,DefaultMode
		RJMP TIE00
	TWI_IO_EndMY_SETUP_I2C_W:
	CPI tmp1,MY_SETUP_I2C_R
	BRNE /*!=*/ TWI_IO_EndMY_SETUP_I2C_WMY_SETUP_I2C_R	//�� ������ ���������
		LDS tmp1, TWI_IOc //���������� ����������� ����
		LDI 	ZL,low(TWI_IO)	//���� ���������
		LDI 	ZH,high(TWI_IO)
		UARTPrint STK_OK
		
		TWI_IO_EndMY_SETUP_I2C_WMY_SETUP_I2C_R_L:
			UARTPrint_rm Z+
			DEC tmp1
		BRNE /*!=*/ TWI_IO_EndMY_SETUP_I2C_WMY_SETUP_I2C_R_L

		STI Mode,DefaultMode
		RJMP TIER00
	TWI_IO_EndMY_SETUP_I2C_WMY_SETUP_I2C_R:

	TIE00:
		CBIFlag TWI_Busy
RET
//������ �� I2C
TWI_IO_Erro:
	
	LDS tmp1,Mode
	CPI tmp1,MY_SETUP_I2C_W
	BRNE /*!=*/ TWI_IO_ErroMY_SETUP_I2C_W	//�� ������ ��������
		LDS tmp1,TWI_IOl
		UARTPrint_r tmp1
		UARTPrint STK_FAILED
		STI Mode,DefaultMode
		RJMP TIER00
	TWI_IO_ErroMY_SETUP_I2C_W:

	TIER00:
		CBIFlag TWI_Busy
RET

//��������� ������ ���� �� �����.
//���� �� �����, DefaultMode, �� ���� ���� - ����� ������
//���� �� � ������ - �� ������� ��� �� �����
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

//��������� ������ ����, ������������, ���� ���� �� ����, � ��� ������ ������ ����
//tmp2 - ��� ����, � ������� ���.
UART_End_SET_MODE:
	LDS	tmp2,UART_I
	//���� �����, � ������� �� ���������
	CPI	tmp2,0x00 //��� ����������� ���
	BRNE /*!=*/ UART_End_0x00
		STI Mode,MY_SETUP
		RJMP UART_End_SET_MODE_end
	UART_End_0x00:
	CPI	tmp2,CRC_EOP 
	CPI	tmp2,0x20 //�� ��, ����� �������� AVRISP
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
		STI AVRISP_c,0 //�������� ����������
		RJMP UART_End_SET_MODE_end
	UART_End_0x42:
	CPI	tmp2,0x45 
	BRNE /*!=*/ UART_End_0x45
		STI Mode,AVRISP_SET_DEVICE_EXT
		STI AVRISP_c,0 //�������� ����������
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
		//����� ����������������
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
		STI AVRISP_c,0 //�������� ����������
		RJMP UART_End_SET_MODE_end
	UART_End_0x55:
	CPI	tmp2,0x56 
	BRNE /*!=*/ UART_End_0x56
		STI Mode,AVRISP_STK_UNIVERSAL
		STI AVRISP_c,0 //�������� ����������
		RJMP UART_End_SET_MODE_end
	UART_End_0x56:
	CPI	tmp2,0x60 
	CPI	tmp2,0x61 
	CPI	tmp2,0x64 
	BRNE /*!=*/ UART_End_0x64
		STI Mode,AVRISP_STK_PROG_PAGE
		STI AVRISP_c,0 //�������� ����������
		RJMP UART_End_SET_MODE_end
	UART_End_0x64:
	CPI	tmp2,0x74 
	BRNE /*!=*/ UART_End_0x74
		STI Mode,AVRISP_STK_READ_PAGE
		STI AVRISP_c,0 //�������� ����������
		RJMP UART_End_SET_MODE_end
	UART_End_0x74:
	UART_End_SET_MODE_end:
RET

//��������������� ��������� ������� ������ �� ����
//�� ��� �������� ������ ���� ����� � ������, ��� �� ���. ������ �� ���������, ��� �� ��� - ������� tmp1
//� � ����������� �� ����� ������������ ���� - tmp2
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
			//��������� ����������� ��������
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
		CPI tmp1,5-1-1 //�� ������ ��� -1
		BREQ /*=*/ UART_End_AVRISP_000
			/*LDI 	ZL,low(AVRISP)
			LDI 	ZH,high(AVRISP) //��������� ����������
			ADD		ZL,tmp1
			SBCI	ZH, 0
			ST Z,tmp2*/
			INC tmp1
			STS AVRISP_c,tmp1
			RJMP UART_End_SET_MODE_work_end
		UART_End_AVRISP_000:
			//��������� ����������� ��������
			STI Mode,AVRISP_EMPTY_REPL
			RJMP UART_End_SET_MODE_work_end
	UART_End_notAVRISP_SET_DEVICE_EXT:
	
	CPI tmp1,AVRISP_STK_UNIVERSAL
	BRNE /*!=*/ UART_End_notAVRISP_STK_UNIVERSAL
		LDS tmp1,AVRISP_c
		CPI tmp1,4-1+1 //���������� ��� �����, ���� 0�20 - �����!
		BREQ /*=*/ UART_End_AVRISP_0000
			//���������� ����� � ��������� ��������
			LDI 	ZL,low(SPIBUF)
			LDI 	ZH,high(SPIBUF)
			ADD		ZL,tmp1
			SBCI	ZH, 0
			ST Z,tmp2
			INC tmp1
			STS AVRISP_c,tmp1
			RJMP UART_End_SET_MODE_work_end
		UART_End_AVRISP_0000:
			//��������� ����������� ��������
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
			//� ��� ����� ������, ��� ������
			CPI tmp2,'F'
			BRNE /*!=*/ UART_End_PP_err
				STI Mode,AVRISP_STK_PROG_PAGE_FILL
				STI AVRISP_c,0 //�������� ����������
				RJMP UART_End_SET_MODE_work_end
			UART_End_PP_err:
				//�� �� ����� �������� � ����� �������!!!!
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
		CPI tmp1,3-1+1 //���������� ��� �����, ���� 0�20 - �����!
		BREQ /*=*/ UART_End_AVRISP_00000
			//���������� ����� � ��������� ��������
			RCALL AVRISP_readInBuf
			RJMP UART_End_SET_MODE_work_end
		UART_End_AVRISP_00000:
			//��������� ����������� ��������
			CPI	tmp2,CRC_EOP
			BREQ /*=*/ UART_End_AVRISP_00001
				RCALL AVRISP_empty_reply
				RJMP UART_End_SET_MODE_work_end
			UART_End_AVRISP_00001:
			//���� CRC ������!
			//�������� ������� �������� CRC
			UARTPrint STK_INSYNC
			
			STI AVRISP_c,0 //�������� �������, �� ������ ������
			STI AVRISP_c+1,0
			LDS tmp1,AVRISP
			STS AVRISP_l,tmp1 //��������� �����, ������� ��� ���� ���������
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
			//��������� ����������� ��������
			CPI tmp2,CRC_EOP
			BRNE /*!=*/ UART_End_AVRISP_11_ErrCRC
				UARTPrint STK_INSYNC
				STI AVRISP_c,0 //�������� �������, �� ������ ������
				STI AVRISP_c+1,0
				LDS tmp1,LOAD_ADDRESS //�������
				STS AVRISP_page,tmp1
				LDS tmp1,LOAD_ADDRESS+1 //�������
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
		 //� ��� ������, ��� ���� �� ������
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
	//PORTx (���������� ���������� �������), DDRx(��������� �������� ����� x �� ���� ��� �����.).
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
	//PORTx (���������� ���������� �������), DDRx(��������� �������� ����� x �� ���� ��� �����.).
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
	//PORTx (���������� ���������� �������), DDRx(��������� �������� ����� x �� ���� ��� �����.).
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
	//ADCSRA, ADMUX, ������
		RCALL readNext_AVRISP
		LDS tmp1,AVRISP_c
		CPI tmp1,3
		BRNE /*!=*/ UART_End_SET_MODE_work_end_bridg
			LDS tmp2,AVRISP+2
			STS ADC_chanel,tmp2
			TST tmp2 //��������, � �� 0 �� ���?
			BREQ /*=*/ UART_End_MY_SETUP_SET_ADC_ERROR//�� �� ����� �� ������ ���� ��������!!!
			LDS tmp2,AVRISP+1
			//ADMUX: REFS1 REFS0 ADLAR - MUX3 MUX2 MUX1 MUX0
					; 0       0 - ������� ���������� �� ����� AREF
					; 0       1 - V������� (���� AREF ������ ���� ��������. ��� � ���� ����� ���������� ����������� �����������)
					; 1       1 - ���������� ��� 1.1 � (� ����� AREF ����� ���������� ����������� �����������)
					;				1 -  ����������� ��������� �������������� �� ������ ���� ADCH:ADCL [9][8][7][6][5][4][3][2]:[1][0][x][x][x][x][x][x]
					;						0	0	0	0 - ���� ADC0
					;						0	0	0	1 - ���� ADC1
					;						0	0	1	0 - ���� ADC2
					;						0	0	1	1 - ���� ADC3
					;						0	1	0	0 - ���� ADC4
					;						0	1	0	1 - ���� ADC5
					;						0	1	1	0 - ���� ADC6
					;						0	1	1	1 - ���� ADC7
					;						1	0	0	0 - ������ �����������
					;						1	1	1	0 - 1.1 �
					;						1	1	1	1 - 0 � (�����)
			UOUT ADMUX,tmp2
			LDS tmp2,AVRISP
			//ADCSRA: ADEN ADSC ADATE ADIF ADIE ADPS2 ADPS1 ADPS
					;  1 - ���
					;		1 - ������ ��������������
					;			1 - ��������� ��������� �������������� �� ���������� �� ������������ ��������� ����������������
					;					� - ���� ����������
					;						1 - ��������� ���������� �� ���
					;							0		0	0 - CLK/2 - ������� ������ (8���)
					;							0		0	1 - CLK/2 - ������� ������ (8���)
					;							0		1	0 - CLK/4 - ������� ������ (4���)
					;							0		1	1 - CLK/8 - ������� ������ (2���)
					;							1		0	0 - CLK/16 - ������� ������ (1���)
					;							1		0	1 - CLK/32 - ������� ������ (500���)
					;							1		1	0 - CLK/64 - ������� ������ (250���)
					;							1		1	1 - CLK/128 - ������� ������ (125���)
			UOUT ADCSRA,tmp2
			STI Mode,DefaultMode
		RJMP UART_End_SET_MODE_work_end
		UART_End_MY_SETUP_SET_ADC_ERROR:
			UARTPrint STK_FAILED
			STI Mode,DefaultMode
	UART_End_notMY_SETUP_SET_ADC:
	
	CPI tmp1,MY_SETUP_I2C_W
	BRNE /*!=*/ UART_End_notMY_SETUP_I2C_W
		RCALL readNext_AVRISP
		LDS tmp1,AVRISP_c
		LDS tmp2,AVRISP	//������ ���� - ����� ������
		CP tmp1,tmp2
		BRNE /*!=*/ UART_End_SET_MODE_work_end
			LDS		tmp1, AVRISP + 1
			LSL		tmp1
			STS		AVRISP + 1, tmp1
			LDI 	ZL,low(AVRISP + 1)	//���� ���������
			LDI 	ZH,high(AVRISP + 1)
			LDI 	XL,low(TWI_IO)		//����� I2C
			LDI 	XH,high(TWI_IO)
			MOV Counter, tmp2
			UART_End_notMY_SETUP_I2C_W_CMP_L:
				DEC tmp2
				BREQ /*=*/ UART_End_notMY_SETUP_I2C_W_CMP	//�������� ���� ���������
				LD tmp1, Z+
				ST X+,tmp1
			RJMP UART_End_notMY_SETUP_I2C_W_CMP_L
			UART_End_notMY_SETUP_I2C_W_CMP:
			LDS tmp2,AVRISP	//������ ���� - ����� ������
			DEC tmp2 //��� ��� ����� ������
			I2CStart_r tmp2
	UART_End_notMY_SETUP_I2C_W:

	CPI tmp1,MY_SETUP_I2C_R
	BRNE /*!=*/ UART_End_notMY_SETUP_I2C_R
		RCALL readNext_AVRISP
		LDS tmp1,AVRISP_c
		CPI tmp1,2
		BRNE /*!=*/ UART_End_SET_MODE_work_end
			LDS tmp1,AVRISP			//�����
			LSL	tmp1
			INC tmp1				//����� ������
			LDS tmp2,AVRISP + 1		//�����
			I2CRead_r tmp2,tmp1
	UART_End_notMY_SETUP_I2C_R:

	UART_End_SET_MODE_work_end:
RET
	UART_End_SET_MODE_work_end_print:
	SetTask TS_UART_PrintExo
RET

//������ ��������� ���� �� ����
//��������!!!! ��������!!!!
//tmp2 - ��������, ������� ���� ��������
//������������ ����������� ���� Z
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

//� ��� ������ ���� ��� - ��� ���. ������ ������ ����,
//���� ������ ��� ������ �� ���
UART_End_SET_MODE_work_MyMode:
	LDS	tmp2,UART_I
	//���� �����, � ������� �� ���������
	CPI	tmp2,0x03
	BRNE /*!=*/ notMode_0x03
		STI Mode,MY_SETUP_SET_BAUD
		STI AVRISP_c,0 //�������� ����������
		RJMP UART_End_SET_MODE_work_MyMode_end
	notMode_0x03:
	CPI	tmp2,0x04
	BRNE /*!=*/ notMode_0x04
		STI Mode,MY_SETUP_SET_PORTB
		STI AVRISP_c,0 //�������� ����������
		RJMP UART_End_SET_MODE_work_MyMode_end
	notMode_0x04:
	CPI	tmp2,0x05
	BRNE /*!=*/ notMode_0x05
		STI Mode,MY_SETUP_SET_PORTC
		STI AVRISP_c,0 //�������� ����������
		RJMP UART_End_SET_MODE_work_MyMode_end
	notMode_0x05:
	CPI	tmp2,0x06
	BRNE /*!=*/ notMode_0x06
		STI Mode,MY_SETUP_SET_PORTD
		STI AVRISP_c,0 //�������� ����������
		RJMP UART_End_SET_MODE_work_MyMode_end
	notMode_0x06:
	CPI	tmp2,0x07
	BRNE /*!=*/ notMode_0x07
		STI Mode,MY_SETUP_SET_ADC
		STI AVRISP_c,0 //�������� ����������
		RJMP UART_End_SET_MODE_work_MyMode_end
	notMode_0x07:
	CPI	tmp2,0x08
	BRNE /*!=*/ notMode_0x08 //�������� ��� �������� ������
		STI Mode,DefaultMode
		IN MacroR,PINB		//0
		RCALL UART_Print
		IN MacroR,DDRB		//1
		RCALL UART_Print 
		IN MacroR,PORTB		//2
		RCALL UART_Print 
		IN MacroR,PINC		//3
		RCALL UART_Print 
		IN MacroR,DDRC		//4
		RCALL UART_Print 
		IN MacroR,PORTC		//5
		RCALL UART_Print 
		IN MacroR,PIND		//6
		RCALL UART_Print 
		IN MacroR,DDRD		//7
		RCALL UART_Print 
		IN MacroR,PORTD		//8
		RCALL UART_Print 
		UARTPrint STK_OK	//9
		RJMP UART_End_SET_MODE_work_MyMode_end
	notMode_0x08:
	CPI	tmp2,0x09
	BRNE /*!=*/ notMode_0x09	//������� ��������� �� I2C
		STI Mode,MY_SETUP_I2C_W
		STI AVRISP_c,0 //�������� ����������
		RJMP UART_End_SET_MODE_work_MyMode_end
	notMode_0x09:
	CPI	tmp2,0x0A
	BRNE /*!=*/ notMode_0x0A	//������� ��������� �� I2C
		STI Mode,MY_SETUP_I2C_R
		STI AVRISP_c,0 //�������� ����������
		RJMP UART_End_SET_MODE_work_MyMode_end
	notMode_0x0A:
	CPI	tmp2,0x20
	BRNE /*!=*/ notMode_0x20 //�������� �����
		STI Mode,DefaultMode
		UARTPrint STK_OK
		RJMP UART_End_SET_MODE_work_MyMode_end
	notMode_0x20:

	UARTPrint STK_UNKNOWN
	STI Mode,DefaultMode
	UART_End_SET_MODE_work_MyMode_end:
RET

//� tmp1 - ����� ��������
//� tmp2 - ��, ��� ����� ��������!!!
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
		//����������� ���, ��� ���� ��������� ��������� ���� - 4�
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
		//����������� ���, ��� ���� ��������� ��������� ���� - 4�
		LDS MacroR,SPIBUF+3
		CALL UART_Print
		SPI_End_AVRISP_STK_PROG_PAGE_FILL: //�� ���� �����, �� ��� ������ �������
		LDS Zh,AVRISP_c
		LDS Zl,AVRISP_c+1 //�������!
		ADIW Zl,1
		LDS tmp1,AVRISP_l
		LDS tmp2,AVRISP_l+1 //�������!
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
				LDS tmp1,LOAD_ADDRESS //�������
				LDS tmp2,AVRISP_page //�������
					CPSE tmp1,tmp2
						RJMP SPI_End_PP_updateFlash
					//������� ����� �����
					LDS tmp1,LOAD_ADDRESS+1 //�������
					LDS tmp2, AVRISP_pagesize
					AND tmp1,tmp2
					LDS tmp2,AVRISP_page+1 //�������
					CPSE tmp1,tmp2
						RJMP SPI_End_PP_updateFlash
					//����� ������� � �������, �������� �� ���������
					SetTask TS_AVRISP_write_flash_pages
				RJMP SPI_End_end
				SPI_End_PP_updateFlash:
					//���� �������� ��������
					SetTask TS_AVRISP_commitPage
				RJMP SPI_End_end
			RJMP SPI_End_end
		SPI_End_END_RPEEPROM:
			LDS tmp1,Mode
			CPI tmp1,AVRISP_STK_PROG_PAGE_FILL
			BRNE /*!=*/ SPI_End_END_RPEEPROM_OK
				//� ��� ����� ����������������
				//���� �������� ��������
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
		LDS Zl,AVRISP_c+1 //�������!
		ADIW Zl,1
		LDS tmp1,AVRISP_l
		LDS tmp2,AVRISP_l+1 //�������!
		CP Zl,tmp2
		CPC Zh,tmp1
		BRSH /*>=*/ SPI_End_PP_commit_end
			//� ��� ����� ��������� �����
			LDS tmp1,LOAD_ADDRESS //�������
			STS AVRISP_page,tmp1
			LDS tmp1,LOAD_ADDRESS+1 //�������
			LDS tmp2, AVRISP_pagesize
			AND tmp1,tmp2
			STS AVRISP_page+1,tmp1
			STI Mode,AVRISP_STK_PROG_PAGE_FILL
			SetTask TS_AVRISP_write_flash_pages
			RJMP SPI_End_end
		SPI_End_PP_commit_end:
			//��������� ���������������
			UARTPrint STK_OK
			STI Mode,DefaultMode
			RJMP SPI_End_end
	SPI_End_notAVRISP_STK_PROG_PAGE_COMMIT:

	SPI_End_end:
	STI SPIBUF_l,0x00
	STI SPIBUF_c,0x00
RET

//��������� ������ �� ����������������� ������
EEPROM_End:
	LDS	tmp1,ModeEEPROM
	//���� �����, � ������� �� ���������
	CPI	tmp1,EEPROM_READ_UART //����� ������ �� ������ ������ �� �����
	BRNE /*!=*/ notEEPROM_READ_UART
		LDS 	tmp1, EEPROMStr+1 //������ ������ �����
		UOUT 	UBRR0L,tmp1
		LDS 	tmp1, EEPROMStr
		UOUT 	UBRR0H,tmp1
		UARTPrint CRC_EOP
		RJMP EEPROM_End_def
	notEEPROM_READ_UART:


	LDS tmp1,Mode
	CPI tmp1,MY_SETUP_SET_BAUD
	BRNE /*!=*/ EEPROM_EndnotMY_SETUP_SET_BAUD //�������� � EEPROM
		STI Mode,DefaultMode 
		UARTPrint STK_OK
	EEPROM_EndnotMY_SETUP_SET_BAUD:


	EEPROM_End_def:
	STI ModeEEPROM,DefaultMode
	EEPROM_End_end:
RET