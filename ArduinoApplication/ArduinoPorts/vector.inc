/*
 * vector.inc
 *
 *  Created: 11.05.2021 22:24:58
 *   Author: Terran
 */ 

.ORG 	0x0000          ; RESET - сброс			; Проц стартует с нуля, но дальше идут вектора 
JMP 	Reset			; прерываний, поэтому отсяюда сразу же прыгаем
						; на начало программы. На метку Reset
.ORG	INT0addr		;INT0 - внешнее прерывание 0
RJMP	DataIN
.ORG	INT1addr		;INT1 - внешнее прерывание 1
RETI
.ORG	PCI0addr		;PCINT0 - прерывание по изменению состояния нулевой группы выводов
RETI
.ORG	PCI1addr		;PCINT1 - прерывание по изменению состояния первой группы выводов
RETI
.ORG	PCI2addr		;PCINT2 - прерывание по изменению состояния второй группы выводов
RETI
.ORG	WDTaddr			;WDT - прерывание от сторожевого таймера
RJMP 	WDT			//Попробуем просто продолжить с начала
.ORG	OC2Aaddr			;TIMER2 COMPA - прерывание от таймера/счетчика T2 при совпадении с A
RJMP	OneMsTimer
.ORG	OC2Baddr			;TIMER2 COMPB - прерывание от таймера/счетчика T2 при совпадении с B
RETI
.ORG	OVF2addr		; Переполнение таймера/счетчика Т2
RETI
.ORG	ICP1addr		; Захват таймера/счетчика Т1
RETI
.ORG	OC1Aaddr		; Совпадение «А» таймера/счетчика Т1
RETI
.ORG	OC1Baddr		; Timer/Counter1 Compare Match B
						;Совпадение «В» таймера/счетчика Т1
RETI
.ORG	OVF1addr		; Timer/Counter1 Overflow
						;Переполнение таймера/счетчика Т1
RETI
.ORG	OVF0addr		; Timer/Counter0 Overflow
						;Переполнение таймера/счетчика Т0
RETI
.ORG	SPIaddr			; Serial Transfer Complete
RETI					;Передача по SPI завершена

.ORG	URXCaddr		; USART, Rx Complete
RJMP	UARTCaddr		;USART, прием завершен

.ORG	UDREaddr		; USART Data Register Empty
RJMP	UARTEaddr		;Регистр данных USART пуст

.ORG	UTXCaddr		; USART, Tx Complete
						;USART, передача завершена
RETI
.ORG	ADCCaddr		; ADC Conversion Complete
						;Преобразование АЦП завершено
RJMP	ADCend			;АЦП, готово

.ORG	ERDYaddr		; EEPROM Ready
RJMP EEPROMrdy			;EEPROM, готово

.ORG	ACIaddr			; Analog Comparator
						;Аналоговый компаратор
RETI
.ORG	TWIaddr			; 2-wire Serial Interface
RJMP	TWI_Event		;Прерывание от модуля TWI

.ORG	SPMRaddr		; Store Program Memory Ready
						;Готовность SPM
RETI

.ORG	INT_VECTORS_SIZE		; Конец таблицы прерываний