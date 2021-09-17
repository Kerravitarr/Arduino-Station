;
; ArduinoPorts.asm
;
; Created: 11.05.2021 22:24:16
; Author : Terran
;
//http://microsin.net/programming/avr/avr061-stk500-communication-protocol.html- AVRISP
;= Start macro.inc ========================================
.LISTMAC //����� ��������� LISTMAC ���������� ����� ���������� � �������� ���������� �������. \
.LIST

;===========���������==============================================================
	.equ 	F_CPU = 16000000 	
	.equ 	baudrate = 9600  
	.equ 	bauddivider = F_CPU/(16*baudrate)-1

	//���������� � �������
	.set 	SEC_PER_INSTR = 1.0/F_CPU
	//���������� � �����������
	.set 	M_SEC_PER_INSTR = 1000.0/F_CPU
	//���������� � ������������
	.set 	MK_SEC_PER_INSTR = 1000000/F_CPU
	// ����������� ������� SPI (� ��).
	// ��������, ��� ATtiny @ 128 ���: � ������� ������ �������, ��� �������� ������� �������� � ������� ������
	// SPI ������ ����> 2 ������ ��, ������� �������� 3 �����, �.�. ��������� ����
	// f_cpu �� 6:
	// #define SPI_CLOCK (128000/6)
	//
	// �������, ���������� ��������� ��� ATtiny85 @ 1 ���, �������� �������� ��������� �� ���������:
	// ������� � ������
	.equ SPI_SPEED = 1000000/6
	.equ SPI_DELAY = F_CPU/SPI_SPEED
;===========�������==============================================================
			.include "macro.inc"

			.def Counter2 = r2				; ������� (��������������� ������������ ��� ����������� ������)			;
			.def r4_	 = r4				;
			.def r5_	 = r5				;
			.def r6_	 = r6				;
			.def r7_	 = r7				;	
			.def r8_	 = r8				;		
			.def r9_	 = r9				;		
			.def r10_	 = r10				;		
			.def r11_	 = r11				;		
			.def r12_	 = r12				;		
			.def r13_	 = r13				;		
			.def r14_	 = r14				;		
			.def r15_	 = r15				;		
			.def Counter = r16				; ������� (��������������� ������������ ��� ����������� ������)			;
			.def MacroR	 = r17				; ������� ��� ��������
			.def tmp1	 = r18				;
			.def tmp2	 = r19				;
			.def tmp3	 = r20				; ��������� ���������� ������ ����������
			.def interrupt= r21				; ������� ��������� ��� ������� ����������
			.def r22_	 = r22				;
			.def r23_	 = r23				;
			.def r24_	 = r24				;
			.def r25_	 = r25				;
			//.def Xl	 = r26				;�� ��� �������� ������!	
			//.def Xh	 = r27				
			//.def Yl	 = r28			
			//.def Yh	 = r29				
			//.def Zl	 = r30			
			//.def Zh	 = r31		
;============SSEG=============================================================
			.DSEG

Flag:		.byte 1									; �����
			.equ 	TWI_Busy		= 0				; ���� ��������� ����������
			.equ 	EEPROM_write	= 1				; ���� ������ � EEPROM
			.equ 	__				= 2				;
			.equ 	rst_active_high	= 3				; ��� ������ avrisp ���������, ��� RESET ������������ ������� �������
			.equ 	SPI_Busy		= 4				; ���� ��������� ����������
			.equ 	SPI_Phase1		= 5				; ���� ����


			.equ 	TaskQueueSize	= 11			; ������ ������� �������
TaskQueue: 	.byte 	TaskQueueSize 					; ����� ������� ������� � SRAM

			.equ 	TimersPoolSize	= 11			; ���������� ��������
TimersPool:	.byte 	TimersPoolSize*3				; ������ ���������� � ��������

			.equ 	SlowTimersPoolSize	 = 4		; ���������� ��������
SlowTimersPool:	.byte 	SlowTimersPoolSize*2		; ������ ���������� � ��������

			.equ 	WordOutSize		 = 11			; ���������� ��������� ������
WordOut:	.byte 	WordOutSize						; ������ ����

			.equ 	WordInSize			= 4			; ���������� ��������� �����
WordIn:		.byte 	WordInSize						; ������ ����

ST_Timer:	.byte 2									;����� ��� ���������� �������, ������ 60� ��

ADC_chanel:	.byte 1									;������, ������� ���������� ���

LOAD_ADDRESS:.byte 2								;�����, � �������� ���������� ������ ������ read_page (here)

			.equ 	EEPROMMaxSize	= 2				; ������������ ������ ������������ ����������
EEPROMStr:	.byte 	EEPROMMaxSize					;������ ��������
EEPROMPos_s:.byte 1									;������� � ������, � ������� ������ �����
EEPROMPos_c:.byte 1									;����� ���������, ������� ����� ���������� � EEPROM
			.equ 	EEPROM_BAUD		= 0;1			;�������������� � EEPROM �������� ������

			//============================��� ������ �������� �� ������ �������� - ���� ����� ����������/������������ ���� (������� ����������), ���� ����� ������

TWI_IOc:	.byte 	1								;����� ���������� ��������
TWI_IOl:	.byte 	1								;��������� ��������
			.equ 	TWISize			 = 20			; ���������� �������� ������� �� ������� ������� �� I2C
TWI_IO:		.byte 	TWISize							;


SPIBUF_c:	.byte 1									;����� ������������ ����
SPIBUF_l:	.byte 1									;���� ��������� ��������
			.equ SPIBUFSize	= 8						;SPIBUF ��� �������� � ����� ��������� 
SPIBUF:		.byte SPIBUFSize						;
SPISPEED:	.byte 1									;�������� SPI � ������ ����������


AVRISP_c:	.byte 2									;������� ��� ������ (2� ���� ������������ � ������!!!
AVRISP_l:	.byte 2									;����� ������
AVRISP:		.byte 40								;����� ��� AVRISP
AVRISP_page:.byte 2									;�������� ������
AVRISP_pagesize:.byte 1								;������ ��������. FF - ���, F0 - 32, E0 - 64, C0 - 128, 80 - 256

//			����������� �����
			.equ 	UARTSize		 = 40			; ���������� �������� ������� �� ������� ��������
UART_O_buf:	.byte 	UARTSize						; C�������� �� UART
UART_O_head:.byte 	1								; ������ ������
UART_O_tail:.byte 	1								; ����� ������
UART_I:		.byte 	1								; C�������� �� UART,������� �� ������ - ����� ���� ������ ���������



Mode:		.byte 1									;�����
			.equ DefaultMode		= 0				;�� ���� �� ����������, ������ ����������
			.equ AVRISP_SET_DEVICE	= 1				; ��������� ���������� ���������������� ��� �������� ����������.
			.equ AVRISP_GET_PARAMETER	= 2			; ��������� �������� ����������� ��������� ����������� STK500.
			.equ AVRISP_EMPTY_REPL	= 3				; ������ �����
			.equ AVRISP_SET_DEVICE_EXT	= 4			; ������������� ����������� ��������� ���������������� ��� �������� ����������.
			.equ AVRISP_STK_UNIVERSAL	= 5			; ������������� �������, ������� ������������ ��� �������� �������� 32-������� ������ ������/������ �������� � ��������� SPI �������� ����������. � ������ ������������ ��������� ���������� 8 ���.
			.equ AVRISP_STK_ENTER_PROGMODE = 6		;���� � ����� ���������������� ��� ���������� ����������.
			.equ AVRISP_STK_LOAD_ADDRESS = 7		;������� ��������� � ���������� 16-������ �����.
			.equ AVRISP_STK_READ_PAGE		= 8		;������ ���� ������ �� FLASH ��� EEPROM � ������ �������� ����������.
			.equ AVRISP_STK_READ_PAGE_FLASH		= 9	;������ ���� ������ �� FLASH
			.equ AVRISP_STK_READ_PAGE_EEPROM	= 10;������ ���� ������ �� EEPROM
			.equ AVRISP_STK_PROG_PAGE	= 11		;��������� ���� ������ � ����������, � ������������� ����� ������� ������ FLASH ��� EEPROM �������� ����������. 
			.equ AVRISP_ERR_MODE		= 12		;��������� ���� ������ � ����������, � ������������� ����� ������� ������ FLASH ��� EEPROM �������� ����������. 
			.equ AVRISP_STK_PROG_PAGE_FILL= 13		;������ ������, ������� ����� ����� ������
			.equ AVRISP_STK_PROG_PAGE_COMMIT= 14	;���������� �������� � ������ ����
			.equ MY_SETUP					= 15	;������ � ��� ����������� �����, ��������� ����� - �����
			.equ MY_SETUP_SET_BAUD			= 16	;��������� �������� ��������
			.equ MY_SETUP_SET_PORTB			= 17	;��������� ���� ����� B - PORTx (���������� ���������� �������), DDRx(��������� �������� ����� x �� ���� ��� �����.).
			.equ MY_SETUP_SET_PORTC			= 18	;��������� ���� ����� C - PORTx (���������� ���������� �������), DDRx(��������� �������� ����� x �� ���� ��� �����.).
			.equ MY_SETUP_SET_PORTD			= 19	;��������� ���� ����� D - PORTx (���������� ���������� �������), DDRx(��������� �������� ����� x �� ���� ��� �����.).
			.equ MY_SETUP_SET_ADC			= 20	;��������� ��������� ��� - ADCSRA, ADMUX, ������

			
ModeEEPROM:	.byte 1									;����� ��� ������ �� ����������������� ������
			//.equ DefaultMode		= 0				;�� ���� �� ����������, ������ ����������
			.equ EEPROM_READ_UART	= 1				;������ �������� ����

;===========CSEG==============================================================
			; ���������� ��� ���������� ������
.CSEG
//������ ����������
.include "vector.inc"
//����������� ����������
.include "Interrupts.inc"
//�������, ���������� ���������
.include "macroFun.inc"
//���������:
.include "AVRISP.inc"
; DriverIO
.include "DriverIO.inc"
; RTOS Here
.include "RTOS.inc"
;=============================================================================
; Main code
;=============================================================================
Reset:		OUTI 	SPL,low(RAMEND) 		; ������ ����� �������������� ����
			OUTI	SPH,High(RAMEND)								

; ������� ������
RAM_Flush:	LDI		ZL,Low(SRAM_START)
			LDI		ZH,High(SRAM_START)
			CLR		tmp1
			
			Flush:		
				ST 		Z+,tmp1
				CPI		ZH,High(RAMEND)
			BRNE	Flush
				CPI		ZL,Low(RAMEND)
			BRNE	Flush

			LDI	ZL, 30		; ����� ������ �������� ��������	
			CLR	ZH			; � ��� � ��� ����� ����
				DEC	ZL		; �������� �����
				ST	Z, ZH	; ���������� � ������� 0
			BRNE PC-2	; ���� �� ��������� ��� �� �����������


;==================================================================================
; Init RTOS
;==================================================================================
	ldi tmp1, 0x00
	out SREG, tmp1			; ������������� SREG 

	RCALL ClearTimers		; �������� ������ �������� ����
	RCALL ClearTaskQueue		; �������� ������� ������� ����
	sei						; ��������� ��������� ����������

	; Init Timer 2
	; �������� ������ ��� ���� ����

	.equ MainClock 		= 16000000				; CPU Clock
	.equ TimerDivider 	= MainClock/64/1000 	; 1 mS
			
	ldi tmp1,low(TimerDivider)
	UOUT OCR2A,tmp1				; ���������� �������� � ������� ���������

	LDI tmp1,(1<<WGM01)|(0<<WGM00)	; ��������� ����� ���������� �������� ���������
	UOUT TCCR2A,tmp1			
	LDI tmp1,0b0000100			; Freq = CK/64 - ���������� ����� � ������������ //64 ���� ����
	UOUT TCCR2B,tmp1			
	LDI tmp1,0b011
	UOUT TIMSK2,tmp1			; ���������� ����������

	CLR tmp1					; ���������� ��������� �������� ���������
	UOUT TCNT2,tmp1				;

	;���������� ������
	WDR
	LDI tmp1,(0 << WDIF) | (1 << WDIE) | (0 << WDE) | (1 << WDP3) | (1 << WDP2) | (1 << WDP1) | (1 << WDP0);; // ���������� ��� ���������� ��������� ��� � ��������� ����� ������ ������� Watchdog�
	UOUT WDTCSR,tmp1
	LDI tmp1,(1 << WDIF) | (1 << WDIE) | (0 << WDE) | (1 << WDP3) | (1 << WDP2) | (1 << WDP1) | (1 << WDP0); // ������������� ��������������� ���������� ����������� ������� WDPS_1S 
	UOUT WDTCSR,tmp1
	WDR

	//I2C
	LDI tmp1,0x72			;�������� �������� (��� 8 ��� ���������� 100 ���), � � ��� ��
	UOUT TWBR,tmp1
	LDI tmp1,0x00				
	UOUT TWSR,tmp1

	//Speep
	LDI tmp1,(1 << SE)			;��� Idle, ����� �����
	UOUT SMCR,tmp1
			
	//UART
	//��������
	LDI 	tmp1, low(bauddivider)
	UOUT 	UBRR0L,tmp1
	LDI 	tmp1, high(bauddivider)
	UOUT 	UBRR0H,tmp1

	LDI 	tmp1,0
	UOUT 	UCSR0A, tmp1
 
	; ������ ����� - 8 ���, ����� � ������� UCSRC, �� ��� �������� ��� ��������
	LDI 	tmp1, (0<<USBS0)|(1<<UCSZ00)|(1<<UCSZ01)
	UOUT 	UCSR0C, tmp1

	; ���������� ���������, �����-�������� ��������.
	LDI 	tmp1, (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)|(0<<TXCIE0)|(0<<UDRIE0)
	UOUT 	UCSR0B, tmp1	

	STI SPISPEED,SPI_DELAY

	SetTimerTask TS_UART_SETUP, 30000

	UARTPrint CRC_EOP
;==================================================================================
; ������������� ���������
;----------------------------------------------------------------------------------	
	.equ 	UART_RX = 0 
	.equ 	UART_TX = 1 
	.equ 	Serial_parallel_interface_Data_O = 1 
	.equ 	Serial_parallel_interface_Port = PORTD 
	.equ 	Serial_parallel_interface_Pin = PIND
	.equ 	Serial_parallel_interface_Data_I = 1 
	.equ 	Serial_parallel_interface_Strob = 1
	.equ 	Serial_parallel_interface_Update = 1
	.equ 	SPI_PORT = PORTC
	.equ 	SPI_DDR = DDRC
	.equ 	SPI_PIN = PINC
	.equ 	SPI_SS_PIN = 1						;���� � ���� ��� ���������� SPI
	.equ 	SPI_SCK_PIN = 2
	.equ 	SPI_MOSI_PIN = 3
	.equ 	SPI_MISO_PIN = 4
	//SBI	DDRD,UART_RX						; �����
	//CBI	DDRD,UART_TX						; ����

Background: 
			OUTI 	SPL,low(RAMEND) 		; ��� ��� �������������� ����, ���� �� ���� ������� ��� � ������, �� �� ������� ������ ��� �������� � �������� � ������
			OUTI	SPH,High(RAMEND)	
;==============================================================================
;Main Code Here

Main:		
			SEI								; ��������� ����������.
			wdr								; Reset Watch DOG (���� �� "���������" "������". �� ��� ������� ����� ����� � ���� reset ��� ����������)
			CALL 	ProcessTaskQueue		; ��������� ������� ���������
			RCALL 	Idle					; ������� ����	
			LDS		tmp2,TaskQueue			// ������� �� ������ ������, ���� ��� ������?	
			CPI		tmp2,0xFF
			BRNE	Main //������ ��� ����?
				//�����, ����� � ����������
				SLEEP
				WDRM: rjmp 	Main					; �������� ���� ��������� ����	
;==============================================================================
;Taskss
;=============================================================================
Idle:	
RET

Delay:
	WDR						  ; 1 ����
	subi    MacroR, 1         ; 1 ����
	brcc    (PC-1)            ; 2 �����
RET

UART_SETUP:
	STI EEPROMPos_c,2
	CBIFlag EEPROM_write
	STI ModeEEPROM,EEPROM_READ_UART
	SetTask TS_EEPROM_Start
RET


.ESEG
.dw low(bauddivider)
.dw high(bauddivider)