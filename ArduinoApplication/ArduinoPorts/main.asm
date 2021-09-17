;
; ArduinoPorts.asm
;
; Created: 11.05.2021 22:24:16
; Author : Terran
;
//http://microsin.net/programming/avr/avr061-stk500-communication-protocol.html- AVRISP
;= Start macro.inc ========================================
.LISTMAC //После директивы LISTMAC компилятор будет показывать в листинге содержимое макроса. \
.LIST

;===========Константы==============================================================
	.equ 	F_CPU = 16000000 	
	.equ 	baudrate = 9600  
	.equ 	bauddivider = F_CPU/(16*baudrate)-1

	//Инструкций в секунду
	.set 	SEC_PER_INSTR = 1.0/F_CPU
	//Инструкций в милисекунду
	.set 	M_SEC_PER_INSTR = 1000.0/F_CPU
	//Инструкций в микросекунду
	.set 	MK_SEC_PER_INSTR = 1000000/F_CPU
	// Настраиваем частоту SPI (в Гц).
	// Например, для ATtiny @ 128 кГц: в таблице данных указано, что тактовый импульс высокого и низкого уровня
	// SPI должен быть> 2 циклов ЦП, поэтому возьмите 3 цикла, т.е. разделите цель
	// f_cpu на 6:
	// #define SPI_CLOCK (128000/6)
	//
	// Частота, достаточно медленная для ATtiny85 @ 1 МГц, является разумным значением по умолчанию:
	// Частота в Герцах
	.equ SPI_SPEED = 1000000/6
	.equ SPI_DELAY = F_CPU/SPI_SPEED
;===========Макросы==============================================================
			.include "macro.inc"

			.def Counter2 = r2				; Счетчик (преимущественно используется для организации циклов)			;
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
			.def Counter = r16				; Счетчик (преимущественно используется для организации циклов)			;
			.def MacroR	 = r17				; Регистр для макросов
			.def tmp1	 = r18				;
			.def tmp2	 = r19				;
			.def tmp3	 = r20				; Некоторые переменные общего назначения
			.def interrupt= r21				; Регистр полностью под властью прерываний
			.def r22_	 = r22				;
			.def r23_	 = r23				;
			.def r24_	 = r24				;
			.def r25_	 = r25				;
			//.def Xl	 = r26				;На них работает таймер!	
			//.def Xh	 = r27				
			//.def Yl	 = r28			
			//.def Yh	 = r29				
			//.def Zl	 = r30			
			//.def Zh	 = r31		
;============SSEG=============================================================
			.DSEG

Flag:		.byte 1									; Флаги
			.equ 	TWI_Busy		= 0				; Флаг занятости устройства
			.equ 	EEPROM_write	= 1				; Флаг записи в EEPROM
			.equ 	__				= 2				;
			.equ 	rst_active_high	= 3				; Для режима avrisp указывает, что RESET активируется высоким уровнем
			.equ 	SPI_Busy		= 4				; Флаг занятости устройства
			.equ 	SPI_Phase1		= 5				; Флаг фазы


			.equ 	TaskQueueSize	= 11			; Размер очереди событий
TaskQueue: 	.byte 	TaskQueueSize 					; Адрес очереди сотытий в SRAM

			.equ 	TimersPoolSize	= 11			; Количество таймеров
TimersPool:	.byte 	TimersPoolSize*3				; Адреса информации о таймерах

			.equ 	SlowTimersPoolSize	 = 4		; Количество таймеров
SlowTimersPool:	.byte 	SlowTimersPoolSize*2		; Адреса информации о таймерах

			.equ 	WordOutSize		 = 11			; Количество регистров вывода
WordOut:	.byte 	WordOutSize						; Адреса слов

			.equ 	WordInSize			= 4			; Количество регистров ввода
WordIn:		.byte 	WordInSize						; Адреса слов

ST_Timer:	.byte 2									;Байты для медленного таймера, отсчёт 60к мс

ADC_chanel:	.byte 1									;Каналы, которые использует АЦП

LOAD_ADDRESS:.byte 2								;Адрес, с которого начинается работа режима read_page (here)

			.equ 	EEPROMMaxSize	= 2				; Максимальный размер записываемой информации
EEPROMStr:	.byte 	EEPROMMaxSize					;Память аппарата
EEPROMPos_s:.byte 1									;Позиция в памяти, с которой ведётся обмен
EEPROMPos_c:.byte 1									;Длина сообщения, которым нужно обменяться с EEPROM
			.equ 	EEPROM_BAUD		= 0;1			;Местоположение в EEPROM скорости обмена

			//============================Все буфера строятся по одному принципу - байт числа переданных/обработанных байт (счётчик заполнения), байт длины буфера

TWI_IOc:	.byte 	1								;Число переданных символов
TWI_IOl:	.byte 	1								;Ожидается символов
			.equ 	TWISize			 = 20			; Количество символов которые мы ожидаем принять по I2C
TWI_IO:		.byte 	TWISize							;


SPIBUF_c:	.byte 1									;Число обработанных байт
SPIBUF_l:	.byte 1									;Надо отправить символов
			.equ SPIBUFSize	= 8						;SPIBUF Для передачи и приёма сообщений 
SPIBUF:		.byte SPIBUFSize						;
SPISPEED:	.byte 1									;Скорость SPI в тактах процессора


AVRISP_c:	.byte 2									;Счётчик для буфера (2й байт используется в циклах!!!
AVRISP_l:	.byte 2									;Длина пакета
AVRISP:		.byte 40								;Буфер для AVRISP
AVRISP_page:.byte 2									;Страница записи
AVRISP_pagesize:.byte 1								;Размер страницы. FF - деф, F0 - 32, E0 - 64, C0 - 128, 80 - 256

//			Циклический буфер
			.equ 	UARTSize		 = 40			; Количество символов которые мы ожидаем выкинуть
UART_O_buf:	.byte 	UARTSize						; Cообщение по UART
UART_O_head:.byte 	1								; Голова буфера
UART_O_tail:.byte 	1								; Хвост буфера
UART_I:		.byte 	1								; Cообщение по UART,которое мы примем - всего один символ ожидается



Mode:		.byte 1									;Режим
			.equ DefaultMode		= 0				;Ни чего не происходит, просто существуем
			.equ AVRISP_SET_DEVICE	= 1				; Установка параметров программирования для текущего устройства.
			.equ AVRISP_GET_PARAMETER	= 2			; Получение значения допустимого параметра стартеркита STK500.
			.equ AVRISP_EMPTY_REPL	= 3				; Пустой ответ
			.equ AVRISP_SET_DEVICE_EXT	= 4			; Устанавливает расширенные параметры программирования для текущего устройства.
			.equ AVRISP_STK_UNIVERSAL	= 5			; Универсальная команда, которая используется для отправки обычного 32-битного потока данных/команд напрямую в интерфейс SPI текущего устройства. В ответе возвращается последние выдвинутые 8 бит.
			.equ AVRISP_STK_ENTER_PROGMODE = 6		;Вход в режим программирования для выбранного устройства.
			.equ AVRISP_STK_LOAD_ADDRESS = 7		;Команда загружает в стартеркит 16-битный адрес.
			.equ AVRISP_STK_READ_PAGE		= 8		;Читает блок данных из FLASH или EEPROM в памяти текущего устройства.
			.equ AVRISP_STK_READ_PAGE_FLASH		= 9	;Читает блок данных из FLASH
			.equ AVRISP_STK_READ_PAGE_EEPROM	= 10;Читает блок данных из EEPROM
			.equ AVRISP_STK_PROG_PAGE	= 11		;Загружает блок данных в стартеркит, и программирует этими данными память FLASH или EEPROM текущего устройства. 
			.equ AVRISP_ERR_MODE		= 12		;Загружает блок данных в стартеркит, и программирует этими данными память FLASH или EEPROM текущего устройства. 
			.equ AVRISP_STK_PROG_PAGE_FILL= 13		;Читает данные, которые потом будем писать
			.equ AVRISP_STK_PROG_PAGE_COMMIT= 14	;Записывает страницу в память чипа
			.equ MY_SETUP					= 15	;Сейчас у нас собственный режим, следующее число - режим
			.equ MY_SETUP_SET_BAUD			= 16	;Сохраняем скорость передачи
			.equ MY_SETUP_SET_PORTB			= 17	;Сохраняем биты порта B - PORTx (Управление состоянием выходов), DDRx(Настройка разрядов порта x на вход или выход.).
			.equ MY_SETUP_SET_PORTC			= 18	;Сохраняем биты порта C - PORTx (Управление состоянием выходов), DDRx(Настройка разрядов порта x на вход или выход.).
			.equ MY_SETUP_SET_PORTD			= 19	;Сохраняем биты порта D - PORTx (Управление состоянием выходов), DDRx(Настройка разрядов порта x на вход или выход.).
			.equ MY_SETUP_SET_ADC			= 20	;Сохраняем настройки АЦП - ADCSRA, ADMUX, каналы

			
ModeEEPROM:	.byte 1									;Режим для чтения из энергонезависимой памяти
			//.equ DefaultMode		= 0				;Ни чего не происходит, просто существуем
			.equ EEPROM_READ_UART	= 1				;Чтение настроек УАРТ

;===========CSEG==============================================================
			; Собственно код начинается отсюда
.CSEG
//Вектор прерываний
.include "vector.inc"
//Обработчики прерываний
.include "Interrupts.inc"
//Функции, вызываемые макросами
.include "macroFun.inc"
//Программы:
.include "AVRISP.inc"
; DriverIO
.include "DriverIO.inc"
; RTOS Here
.include "RTOS.inc"
;=============================================================================
; Main code
;=============================================================================
Reset:		OUTI 	SPL,low(RAMEND) 		; Первым делом инициализируем стек
			OUTI	SPH,High(RAMEND)								

; Очистка памяти
RAM_Flush:	LDI		ZL,Low(SRAM_START)
			LDI		ZH,High(SRAM_START)
			CLR		tmp1
			
			Flush:		
				ST 		Z+,tmp1
				CPI		ZH,High(RAMEND)
			BRNE	Flush
				CPI		ZL,Low(RAMEND)
			BRNE	Flush

			LDI	ZL, 30		; Адрес самого старшего регистра	
			CLR	ZH			; А тут у нас будет ноль
				DEC	ZL		; Уменьшая адрес
				ST	Z, ZH	; Записываем в регистр 0
			BRNE PC-2	; Пока не перебрали все не успокоились


;==================================================================================
; Init RTOS
;==================================================================================
	ldi tmp1, 0x00
	out SREG, tmp1			; Инициализация SREG 

	RCALL ClearTimers		; Очистить список таймеров РТОС
	RCALL ClearTaskQueue		; Очистить очередь событий РТОС
	sei						; Разрешить обработку прерываний

	; Init Timer 2
	; Основной таймер для ядра РТОС

	.equ MainClock 		= 16000000				; CPU Clock
	.equ TimerDivider 	= MainClock/64/1000 	; 1 mS
			
	ldi tmp1,low(TimerDivider)
	UOUT OCR2A,tmp1				; Установить значение в регистр сравнения

	LDI tmp1,(1<<WGM01)|(0<<WGM00)	; Автосброс после достижения регистра сравнения
	UOUT TCCR2A,tmp1			
	LDI tmp1,0b0000100			; Freq = CK/64 - Установить режим и предделитель //64 было выше
	UOUT TCCR2B,tmp1			
	LDI tmp1,0b011
	UOUT TIMSK2,tmp1			; Разрешение прерываний

	CLR tmp1					; Установить начальное значение счётчиков
	UOUT TCNT2,tmp1				;

	;Сторожевой таймер
	WDR
	LDI tmp1,(0 << WDIF) | (1 << WDIE) | (0 << WDE) | (1 << WDP3) | (1 << WDP2) | (1 << WDP1) | (1 << WDP0);; // Установите бит «Разрешить изменение бит» и «Включить режим сброса системы Watchdog»
	UOUT WDTCSR,tmp1
	LDI tmp1,(1 << WDIF) | (1 << WDIE) | (0 << WDE) | (1 << WDP3) | (1 << WDP2) | (1 << WDP1) | (1 << WDP0); // Устанавливаем предварительную калибровку сторожевого таймера WDPS_1S 
	UOUT WDTCSR,tmp1
	WDR

	//I2C
	LDI tmp1,0x72			;скорость передачи (при 8 мГц получается 100 кГц), а у нас хз
	UOUT TWBR,tmp1
	LDI tmp1,0x00				
	UOUT TWSR,tmp1

	//Speep
	LDI tmp1,(1 << SE)			;Мод Idle, спать можно
	UOUT SMCR,tmp1
			
	//UART
	//Скорость
	LDI 	tmp1, low(bauddivider)
	UOUT 	UBRR0L,tmp1
	LDI 	tmp1, high(bauddivider)
	UOUT 	UBRR0H,tmp1

	LDI 	tmp1,0
	UOUT 	UCSR0A, tmp1
 
	; Формат кадра - 8 бит, пишем в регистр UCSRC, за это отвечает бит селектор
	LDI 	tmp1, (0<<USBS0)|(1<<UCSZ00)|(1<<UCSZ01)
	UOUT 	UCSR0C, tmp1

	; Прерывания разрешены, прием-передача разрешен.
	LDI 	tmp1, (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)|(0<<TXCIE0)|(0<<UDRIE0)
	UOUT 	UCSR0B, tmp1	

	STI SPISPEED,SPI_DELAY

	SetTimerTask TS_UART_SETUP, 30000

	UARTPrint CRC_EOP
;==================================================================================
; Инициализация периферии
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
	.equ 	SPI_SS_PIN = 1						;Порт и пины для управления SPI
	.equ 	SPI_SCK_PIN = 2
	.equ 	SPI_MOSI_PIN = 3
	.equ 	SPI_MISO_PIN = 4
	//SBI	DDRD,UART_RX						; Выход
	//CBI	DDRD,UART_TX						; Вход

Background: 
			OUTI 	SPL,low(RAMEND) 		; Ещё раз инициализируем стек, если мы сюда перешли как к задаче, то мы удаляем нахрен все переходы и начинаем с начала
			OUTI	SPH,High(RAMEND)	
;==============================================================================
;Main Code Here

Main:		
			SEI								; Разрешаем прерывания.
			wdr								; Reset Watch DOG (Если не "погладить" "собаку". то она устроит конец света в виде reset для процессора)
			CALL 	ProcessTaskQueue		; Обработка очереди процессов
			RCALL 	Idle					; Простой Ядра	
			LDS		tmp2,TaskQueue			// Смотрим на первую задачу, есть она вообще?	
			CPI		tmp2,0xFF
			BRNE	Main //Задачи ещё есть?
				//Пусто, можно и вздремнуть
				SLEEP
				WDRM: rjmp 	Main					; Основной цикл микроядра РТОС	
;==============================================================================
;Taskss
;=============================================================================
Idle:	
RET

Delay:
	WDR						  ; 1 такт
	subi    MacroR, 1         ; 1 такт
	brcc    (PC-1)            ; 2 такта
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