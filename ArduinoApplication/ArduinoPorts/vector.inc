/*
 * vector.inc
 *
 *  Created: 11.05.2021 22:24:58
 *   Author: Terran
 */ 

.ORG 	0x0000          ; RESET - �����			; ���� �������� � ����, �� ������ ���� ������� 
JMP 	Reset			; ����������, ������� ������� ����� �� �������
						; �� ������ ���������. �� ����� Reset
.ORG	INT0addr		;INT0 - ������� ���������� 0
RJMP	DataIN
.ORG	INT1addr		;INT1 - ������� ���������� 1
RETI
.ORG	PCI0addr		;PCINT0 - ���������� �� ��������� ��������� ������� ������ �������
RETI
.ORG	PCI1addr		;PCINT1 - ���������� �� ��������� ��������� ������ ������ �������
RETI
.ORG	PCI2addr		;PCINT2 - ���������� �� ��������� ��������� ������ ������ �������
RETI
.ORG	WDTaddr			;WDT - ���������� �� ����������� �������
RJMP 	WDT			//��������� ������ ���������� � ������
.ORG	OC2Aaddr			;TIMER2 COMPA - ���������� �� �������/�������� T2 ��� ���������� � A
RJMP	OneMsTimer
.ORG	OC2Baddr			;TIMER2 COMPB - ���������� �� �������/�������� T2 ��� ���������� � B
RETI
.ORG	OVF2addr		; ������������ �������/�������� �2
RETI
.ORG	ICP1addr		; ������ �������/�������� �1
RETI
.ORG	OC1Aaddr		; ���������� ��� �������/�������� �1
RETI
.ORG	OC1Baddr		; Timer/Counter1 Compare Match B
						;���������� �» �������/�������� �1
RETI
.ORG	OVF1addr		; Timer/Counter1 Overflow
						;������������ �������/�������� �1
RETI
.ORG	OVF0addr		; Timer/Counter0 Overflow
						;������������ �������/�������� �0
RETI
.ORG	SPIaddr			; Serial Transfer Complete
RETI					;�������� �� SPI ���������

.ORG	URXCaddr		; USART, Rx Complete
RJMP	UARTCaddr		;USART, ����� ��������

.ORG	UDREaddr		; USART Data Register Empty
RJMP	UARTEaddr		;������� ������ USART ����

.ORG	UTXCaddr		; USART, Tx Complete
						;USART, �������� ���������
RETI
.ORG	ADCCaddr		; ADC Conversion Complete
						;�������������� ��� ���������
RJMP	ADCend			;���, ������

.ORG	ERDYaddr		; EEPROM Ready
RJMP EEPROMrdy			;EEPROM, ������

.ORG	ACIaddr			; Analog Comparator
						;���������� ����������
RETI
.ORG	TWIaddr			; 2-wire Serial Interface
RJMP	TWI_Event		;���������� �� ������ TWI

.ORG	SPMRaddr		; Store Program Memory Ready
						;���������� SPM
RETI

.ORG	INT_VECTORS_SIZE		; ����� ������� ����������