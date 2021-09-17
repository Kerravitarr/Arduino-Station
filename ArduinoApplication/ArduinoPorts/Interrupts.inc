/*
 * Interrupts.inc
 *
 *  Created: 18.05.2021 9:27:22
 *   Author: Terran
 */ 

 OneMsTimer:
			in 		interrupt,SREG			; Save Sreg
			PUSH 	interrupt				; Сохранение регистра OSRG и регистра состояния SREG

			PUSH 	ZL	
			PUSH 	ZH						; сохранение Регистра Z
			PUSH 	Counter					; сохранение Регистра Counter
			PUSH 	MacroR					; сохранение Регистра MacroR
	
			LDI 	ZL,low(TimersPool)		; Загрузка с регистр Z адреса RAM, 
			LDI 	ZH,high(TimersPool)		; по которому находится информация о таймерах

			LDI 	Counter,TimersPoolSize	; максимальное количество таймеров
	
Comp1L01:	ld 		interrupt,Z				; interrupt = [Z] ; Получить номер события
			cpi 	interrupt,$FF			; Проверить на "NOP"
			breq 	Comp1L03				; Если NOP то переход к следующей позиции

			clt								; Флаг T используется для сохранения информации об окончании счёта
			ldd 	interrupt,Z+1			; 
			subi 	interrupt,Low(1) 		; Уменьшение младшей части счётчика на 1
			std 	Z+1,interrupt			;
			breq 	Comp1L02				; Если 0 то флаг T не устанавливаем
			set								; 

Comp1L02:	ldd 	interrupt,Z+2			;
			sbci 	interrupt,High(1) 		; Уменьшение старшей части счётчика на 1
			std 	Z+2,interrupt			;
			brne 	Comp1L03				; Счёт не окончен
			brts 	Comp1L03				; Счёт не окончен (по T)	
	
			ld 		MacroR,Z				; Получить номер события
			CALL 	SendTask				; послать в системную очередь задач
	
			LDI 	interrupt,$FF			; = NOP (задача выполнена, таймер самоудаляется)
			st 		Z, interrupt			; Clear Event

Comp1L03:	subi 	ZL,Low(-3)				; Skip Counter
			sbci 	ZH,High(-3)				; Z+=3 - переход к следующему таймеру
			dec 	Counter					; счетчик таймеров
			brne 	Comp1L01				; Loop	


			LDS 	ZL,ST_Timer				; Загрузка в регистр Z времени в мс
			LDS 	ZH,ST_Timer+1				; Загрузка в регистр Z времени в мс

			cpi   ZL,low(60000)				;Сравнение младших байт
			ldi   interrupt,high(60000)		;
			cpc   ZH,interrupt				;Сравнение старшего байта
			BREQ /*=*/ OCI_01
				ADIW ZL,1
				STS ST_Timer,ZL
				STS ST_Timer+1,ZH
				RJMP OCI_00
			OCI_01:
			;Прошло 60000 милисекунд, или 60с, или 1 минута
				//SlowTimerService
				CLR interrupt
				STS ST_Timer,interrupt
				STS ST_Timer+1,interrupt
			OCI_00:
			
			pop 	MacroR					; восстанавливаем переменные
			pop 	Counter					; 
			pop 	ZH
			pop 	ZL

			pop 	interrupt				; Восстанавливаем регистры
			out 	SREG,interrupt			; 
RETI						; выходим из прерывания

DataIN:
			//SetTask TS_I
			LDI 	interrupt, 1
			OUT		EIFR,interrupt
RETI

WDT:
	//Сбросили собачку
	wdr
	//А теперь узнаем из какого места мы попали в эту славную дыру
	POP Zh
	POP Zl
	LDI Yl, low(WDRM)
	LDI Yh, high(WDRM)
	CP		Zl, YL
	CPC		Zh, YH
	BRNE WDT01 //Если не равны, то мы зависли
		//Мы не зависли, это что то со сном, но на всякий пожарный мы обновим все подпрограммы
JMP Background
	WDT01:
		//Мы капитально так зависли, надо перезагружаться
JMP Reset		


;-----------------------------------------------------------------------------
TWI_Event:
		PUSH	ZL
		PUSH	ZH
		PUSH	MacroR
		in 		interrupt,SREG			; Save Sreg
		PUSH	interrupt
		LDS	interrupt,TWSR
/*
		CPI	OSRG,0x00			//Bus Fail Автобус сломался… эээ в смысле аппаратная ошибка шины. Например, внезапный старт посреди передачи бита.
		BRNE	TWIE01
				//Обработка
		TWIE01:
*/
		CPI	interrupt,0x08			//Start Был сделан старт. Теперь мы решаем что делать дальше, например послать адрес ведомого
		BRNE	TWIE02
			LDS		interrupt,TWI_IO
			UOUT TWDR,interrupt			//Пишем адрес
			LDI		interrupt,1	
			STS		TWI_IOc,interrupt	//Записали один символ
			RJMP TWIE_SEND_WRITE
		TWIE02:
/*
		CPI	OSRG,0x10			//ReStart Был обнаружен повторный старт. Можно переключиться с записи на чтение или наоборот. От логики зависит.
		BRNE	TWIE10
				//Обработка
				RJMP TWIE00
		TWIE10:
*/
		CPI	interrupt,0x18 			//SLA+W+ACK Мы отправили адрес с битом записи, а в ответ получили ACK от ведомого. Значит можно продолжать.
		BRNE	TWIE18
			RJMP TWIE28_st //Выполняем действие как для 0х28
		TWIE18:

		CPI	interrupt,0x20			//SLA+W+NACK Мы отправили адрес с битом записи, а нас послали NACK. Обидно, сгенерим ошибку или повторим еще раз.
		BRNE	TWIE20
				//Обработка
				SetTask TS_TWI_IO_Error
				RJMP TWIE_SEND_STOP
		TWIE20:


		CPI	interrupt,0x28 			//Byte+ACK Мы послали байт и получили подтверждение, что ведомый его принял. Продолжаем.
		BRNE	TWIE28
			TWIE28_st:
			LDS		interrupt,TWI_IOc		//Сколько мы послали
			LDS		MacroR,TWI_IOl			//Сколько мы должны послать
			CP interrupt,MacroR
			BREQ /*=*/ TWIE2802 //Всё, совпало, всё отправили, ура!
				//Не совпало, пишем дальше
				LDI 	ZL,low(TWI_IO)
				LDI 	ZH,high(TWI_IO)
				ADD		Zl,interrupt //Сдвигаем окошечко
				sbci	ZH, 0
				INC		interrupt				//+1, у нас новенький
				STS		TWI_IOc,interrupt

				LD		interrupt,Z
				UOUT	TWDR,interrupt //Передаём слово
				RJMP	TWIE_SEND_ACK
			TWIE2802:
				SetTask TS_TWI_IO_End
			RJMP TWIE_SEND_STOP
		TWIE28:
/*
		CPI	OSRG,0x30			//Byte+NACK Мы послали байт, но подтверждение не получили. Видимо ведомый уже сыт по горло нашими подачками или он захлебнулся в данных. Либо его ВНЕЗАПНО посреди передачи данных украли инопланетяне.
		BRNE	TWIE30
				//Обработка
				RJMP TWIE00
		TWIE30:

		CPI	OSRG,0x38			//Collision А у нас тут клановые разборки — пришел другой мастер, по хамски нас перебил, да так, что мы от возмущения аж заткнулись. Ничего I’l be back! До встречи через n тактов!
		BRNE	TWIE38
				//Обработка
				RJMP TWIE00
		TWIE38:
*/
		CPI	interrupt,0x40		//SLA+R+ACK Послали адрес с битом на чтение, а ведомый отозвался. Хорошо! Будем читать.
		BRNE	TWIE40
			TSTFlag TWI_Busy
			BRNE /*!=*/ TWIE4002//Наш автобус не занят, ни чего не ждали, пошло оно всё, ни чего не знаем
				//Кто то ожидает чего то, отлично. Читаем!
				RJMP TWIE_SEND_ACK
			TWIE4002:
				//Ни чего не знаем, это не мы
			RJMP TWIE_SEND_NACK
		TWIE40:
/*
		CPI	OSRG,0x48			//SLA+R+NACK Крикнули в шину «Эй ты, с адресом ХХХ, почитай нам сказки» А в ответ «Иди NACK!» В смысле на запрос адреса с битом чтения никто не откликнулся. Видимо не хотят или заняты. Также может быть никого нет дома.
		BRNE	TWIE48
				//Обработка
				RJMP TWIE00
		TWIE48:
*/
		CPI	interrupt,0x50 		//Receive Byte Мы приняли байт. И думаем что бы ответить ведомому. ACK или NACK.
		BRNE	TWIE50	
			LDS		interrupt,TWI_IOc		//Сколько мы прочитали
			CPI		interrupt,TWISize
			BREQ /*=*/ TWIE_SEND_NACK//Проблемка, у нас совпадение и ни кто ни чего не сделал. Заканчиваем приём
				//Всё ок, читаем, место есть
				LDI 	ZL,low(TWI_IO)
				LDI 	ZH,high(TWI_IO)
				ADD		Zl,interrupt //Сдвигаем окошечко
				sbci	ZH, 0
				INC		interrupt				//+1, у нас новенький
				STS		TWI_IOc,interrupt
				LDS		MacroR,TWDR
				ST		Z,MacroR		//Вот мы и сохранили наш байт
				CPI		interrupt,TWISize
				BRNE /*!=*/ TWIE_SEND_ACK //Всё ок, место есть
					//Проблемка, это был последний. Надо что то сделать, срочно. Или будет беда
					SetTask TS_TWI_IO_End		
			RJMP TWIE_SEND_ACK
		TWIE50:

		CPI	interrupt,0x58			//Receive Byte+NACK Мы приняли байт от ведомого и сказали ему «иди NACK!» И он обиженый ушел, освободив шину.
		BRNE	TWIE58
				RJMP TWIE_SEND_STOP
		TWIE58:

/*		CPI	OSRG,0x60			//Receive SLA+W Сидим на шине, никого не трогаем, ни с кем не общаемся. А тут нас по имени… Конечно отзовемся :)
		BRNE	TWIE60
				//Обработка
				RJMP TWIE00
		TWIE60:

		CPI	OSRG,0x68			//Receive SLA+W LP Мы были мастером, трепались с подчиненными по шине. И тут появляется на шине другой, более равный, мастер, перебивает нас и молвит «Уважаемый ХХ, а не возьмете ли вы вот эти байтики…» Чтож, он круче. Придется бросать передачу и брать его байты себе.
		BRNE	TWIE68
				//Обработка
				RJMP TWIE00
		TWIE68:

		CPI	OSRG,0x70			//Receive SLA+W Broadcast Ситуация повторяется, но на этот раз слышим уже знакомое нам «Слышь, тыыыы». Кто? К кому? Игнорируем Broadcast запросы? Или нет? Зависит от моральных качеств программы.
		BRNE	TWIE70
				//Обработка
				RJMP TWIE00
		TWIE70:

		CPI	OSRG,0x78			//Receive SLA+W LP Broadcast Были мы, как нам казалось, самыми крутыми мастерами на шине. Пока не появился другой мастер и перебив нас прогундосил на всю шину «Эй, слышь тыыы. Слушай сюда» Девайсы-лохи, с неотключенными широковещательными запросами подчиняются. Остальные отмораживаются и всякое Broadcast-быдло игнорируют.
		BRNE	TWIE78
				//Обработка
				RJMP TWIE00
		TWIE78:

		CPI	OSRG,0x80			//Receive Byte & 0x90 Receive Byte Broadcast Принимаем байты. От кого и в каком виде не важно. Решаем что сказать Давай еще (ACK) или «Иди NACK». Тут уже по обстоятельствам.
		BRNE	TWIE80
				//Обработка
				RJMP TWIE00
		TWIE80:

		CPI	OSRG,0x88			//Receive Last Byte & 0x98 Receive Last Byte Broadcast Приняли последний байт и распихиваем по карманам.
		BRNE	TWIE88
				//Обработка
				RJMP TWIE00
		TWIE88:

		CPI	OSRG,0xA0			//Receive ReStart Ой у нас Повторный старт. Видимо то что пришло в первый раз был таки адрес страницы. А сейчас пойдут данные…
		BRNE	TWIEA0
				//Обработка
				RJMP TWIE00
		TWIEA0:

		CPI	OSRG,0xA8			//Receive SLA+R Либо просто к нам какой то другой мастер по имени обращается и просить ему передать байтиков.
		BRNE	TWIEA8
				//Обработка
				RJMP TWIE00
		TWIEA8:

		CPI	OSRG,0xB0			//Receive SLA+R LP Слали мы что то слали, а тут нас перебивает другой мастер, обращается по имени и говорит «А ну ХХ зачитай нам что нибудь из Пушкина» Что делать, приходится читать.
		BRNE	TWIEB0
				//Обработка
				RJMP TWIE00
		TWIEB0:

		CPI	OSRG,0xB8			//Send Byte Receive ACK Ну дали мы ему байт. Он нам ACK. А мы тем временем думаем слать ему еще один (последний) и говорить «иди NACK». Или же у нас дофига их и можно еще пообщаться.
		BRNE	TWIEB8
				//Обработка
				RJMP TWIE00
		TWIEB8:

		CPI	OSRG,0xC0			//Send Last Byte Receive NACK Дали мастеру последний имеющийся байт, а он нам «иди NACK». Хамло. Ну и хрен с ним. Уходим с шины.
		BRNE	TWIEC0
				//Обработка
				RJMP TWIE00
		TWIEC0:

		CPI	OSRG,0xC8			//Send Last Byte Receive ACK Дали мастеру последний имеющийся байт, а он требует еще. Но у нас нету, так что разворачиваемся и уходим с шины. А он пусть карманы воздухом наполняет (в этот момент мастер начнет получать якобы от slave 0xFF байты, на самом деле это просто чтение висящей шины).
		BRNE	TWIEC8
				//Обработка
				RJMP TWIE00
		TWIEC8:
*/

	/*
		TWINT — флаг прерывания. Сброс его означает что конечный автомат провернется дальше, а прерывание будет снова уловлено.
		TWEA — Enable ACK. Разрешение ответа ACK. Если его включить, то автомат TWI будет отзываться на свой адрес, а также давать ACK во всех случаях когда этого требует протокол. Скажем, после прием байта. Если нам надо послать NACK то бит не ставим.
		TWSTA — Сформировать старт. Причем не факт, что прямо сейчас. От текущего состояния зависит. Например, если этот бит поставить в 0х38 состоянии, то старт будет сформирован когда шина освободится. Контроллер TWI умный и сам все хорошо знает.
		TWSTO — сделать Stop. Опять же аналогично. После записи в этот бит модуль отваливается от сессии передачи. МК становится неадресованным ведомым.
		TWWC — конфликт записи. Сигнализирует о том, что кто то из прикладной программы записал в TWDR данные. Тогда как в TWDR полагается писать только при поднятом TWINT (в нашем случае это будет толкьо в обработчике прерывания).
		TWEN — блок TWI включен. Только и всего.
		TWIE — прерывания от TWI разрешены.
	*/
	TWIE_SEND_ACK:
		LDI interrupt,(1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE)
		RJMP TWIE_SEND
	TWIE_SEND_WRITE: //Просто пишем в порт, без всякого там
	TWIE_SEND_NACK:
		LDI interrupt,(1<<TWINT)|(1<<TWEN)|(1<<TWIE)	
		RJMP TWIE_SEND
	TWIE_SEND_STOP:
		LDI interrupt,(1<<TWINT)|(1<<TWSTO)|(1<<TWEN)|(1<<TWIE)	
		//RJMP TWIE_SEND
	TWIE_SEND:			
	UOUT TWCR ,interrupt
	TWIE00:
		POP 	interrupt				; Восстанавливаем регистры
		out 	SREG,interrupt			; 
		POP		MacroR
		POP		ZH
		POP		ZL
RETI

UARTEaddr:
		PUSH	ZL
		PUSH	ZH
		PUSH	MacroR
		in 		interrupt,SREG			; Save Sreg
		PUSH	interrupt
		
		LDS		interrupt,UART_O_head
		LDS		MacroR,UART_O_tail	
		CP interrupt,MacroR
		BREQ /*=*/ UART_END_transmit
			LDI 	ZL,low(UART_O_buf)
			LDI 	ZH,high(UART_O_buf)
			ADD		Zl,MacroR //Сдвигаем окошечко
			sbci	ZH, 0
			LD		interrupt,Z
			UOUT	UDR0,interrupt		; Выдача данных в усарт.
			INC		MacroR
			CPI		MacroR,UARTSize
			BRLO /*<*/ UART_end_not_return
				CLR MacroR
			UART_end_not_return:
			STS UART_O_tail,MacroR
			RJMP UARTE00
		UART_END_transmit:
			//Всё, заканчиваем передачу
			LDI 	interrupt, (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)|(0<<TXCIE0)|(0<<UDRIE0)
			UOUT 	UCSR0B, interrupt	
		UARTE00:
		POP 	interrupt				; Восстанавливаем регистры
		out 	SREG,interrupt			; 
		POP		MacroR
		POP		ZH
		POP		ZL
RETI

UARTCaddr:
		PUSH	MacroR
		LDS	interrupt,UDR0		; Взяли число
		STS UART_I,interrupt
		SetTask TS_UART_End
		POP 	MacroR
RETI

EEPROMrdy:
		PUSH	ZL
		PUSH	ZH
		PUSH	MacroR
		in 		interrupt,SREG			; Save Sreg
		PUSH	interrupt

		LDS interrupt,EEPROMPos_c
		TST interrupt
		BREQ /*=*/ EEPROMrdy_end//Предыдущая операция завершилась, а больше операций и нет
			//У нас ещё что то есть для записи/чтения
			DEC interrupt
			STS EEPROMPos_c,interrupt //В следующий раз будет меньше читать

			LDS	Zl,EEPROMPos_s
			CLR Zh
			OUT 	EEARL,ZL 		; Загружаем адрес нужной ячейки
			OUT 	EEARH,ZH  		; старший и младший байт адреса

			INC Zl
			STS EEPROMPos_s,Zl		;В следующий раз адрес будет следующим

			LDI 	ZL,low(EEPROMStr) ;Писать будем в это место
			LDI 	ZH,high(EEPROMStr)			
			TSTFlag EEPROM_write
			BREQ /*=*/ EEPROMrdy_read//Чтение
				//Запись
				LD		MacroR,Z
				OUT 	EEDR,MacroR 	; Записываем, собственно, данные
 
				SBI 	EECR,EEMPE 		; взводим предохранитель
				SBI 	EECR,EEPE 		; записываем байт

				TST interrupt
				BREQ /*=*/ EEPROMrdy_end//Больше писать нечего
					lds 	MacroR, EEPROMPos_c
					EEPROMrdy_left:					;Сдвинули буфер на один шаг
						ldd 	interrupt, Z+1 		;Взяли следующий пункт
						st 		Z+, interrupt		;Сохранили в текущий
						dec 	MacroR		
					brne 	EEPROMrdy_left		; Loop
				RJMP EEPROMrdy_next
			EEPROMrdy_read:
				//Чтение
				SBI  EECR,EERE          ; Запускаем чтение байта
				IN		MacroR, EEDR     ; Читаем данные
				ST		Z,MacroR
				TST interrupt
				BREQ /*=*/ EEPROMrdy_end//Больше читать нечего
					LDI 	ZL,low(EEPROMStr + EEPROMMaxSize - 2)
					LDI 	ZH,high(EEPROMStr + EEPROMMaxSize - 2)	;Взяли предпоследний байт
					ldi 	MacroR, EEPROMMaxSize-1
					EEPROMrdy_right:
						ld	 	interrupt, Z+ 		;Взяли текущий пункт
						st 		Z, interrupt		;Сохранили в следующий
						SBIW	Zl,2				;Перешли к предыдущему
						dec 	MacroR		
					brne 	EEPROMrdy_right		; Loop
					//Выходим ждать, пока прочтётся		
					RJMP EEPROMrdy_next	
		EEPROMrdy_end:
			CBI 	EECR,EERIE	; Сбрасываем прерывание, больше оно нам не нужно
			SetTask TS_EEPROM_End
		EEPROMrdy_next:
		POP 	interrupt				; Восстанавливаем регистры
		out 	SREG,interrupt			; 
		POP		MacroR
		POP		ZH
		POP		ZL
RETI

ADCend:
	PUSH	MacroR
	PUSH	tmp1
	in 		interrupt,SREG			; Save Sreg
	PUSH	interrupt
	
	UIN interrupt,UCSR0B
	ANDI interrupt,(1 << UDRIE0)
	BRNE /*!=*/   ADCend_start //Всё плохо, УАРТ запущен, мы не можем передать показания. Так что просто ещё раз что было
	//Всё отлично, уарт не запущен.
	UIN	MacroR,ADCH
	RCALL UART_Print_withuotCLI
	//Выбираем следующий канал
	UIN interrupt,ADMUX
	ANDI interrupt,0x0F
	BREQ /*=*/ ADCend_end//У нас измерение не каналов, выходим
		LDI tmp1,1
		ADCend_cyc:
			DEC interrupt
			BREQ /*=*/ ADCend_end_cyc
			LSL tmp1
		RJMP ADCend_cyc
		ADCend_end_cyc:

		LDS interrupt,ADC_chanel
		CLC
		ADCend_cyc2:
			ROL tmp1
			BRNE /*!=*/ ADCend_cyc2_notC
				//Круг замкнулся, цикловая синхронизация
				LDI MacroR,0xFF
				RCALL UART_Print_withuotCLI
				ROL MacroR //Начинаем по второму кругу
			ADCend_cyc2_notC:
			MOV MacroR,interrupt
			AND MacroR,tmp1
		BREQ /*=*/ ADCend_cyc2 //Пока равно, начинаем с самого начала
		MOV MacroR,tmp1
		CLR tmp1
		ADCend_cyc3:
			INC tmp1
			LSR MacroR
		BRNE /*!=*/ ADCend_cyc3

		UIN interrupt,ADMUX
		ANDI interrupt,0xF0 //Записываем новый канал
		OR	interrupt,tmp1
		UOUT ADMUX,interrupt
	
	ADCend_start:
		UIN interrupt,AVRISP
		ORI	interrupt,(1 << ADSC) //Запуск!
		UOUT AVRISP,interrupt	

	ADCend_end:

	POP 	interrupt				; Восстанавливаем регистры
	out 	SREG,interrupt			; 
	POP 	tmp1
	POP 	MacroR
RETI

//Единственный тут, кто не из прерываний
SPI:
	LDI 	ZL,low(SPIBUF)
	LDI 	ZH,high(SPIBUF)

	LDS tmp1,SPIBUF_c
	ADD		ZL,tmp1
	SBCI	ZH, 0
	LD tmp1,Z
	LDS MacroR,SPISPEED
	CLC
	ROR MacroR
	CLC
	ROR MacroR
	INC MacroR
	MOV tmp2,MacroR
	LDI Counter,8
	SPI_for:
		SBRC tmp1,7
			RJMP SPI_print1
			CBI	SPI_PORT,SPI_MOSI_PIN
			RJMP SPI_print_e
		SPI_print1:
			SBI	SPI_PORT,SPI_MOSI_PIN
		SPI_print_e:
		SBI	SPI_PORT,SPI_SCK_PIN

		MOV MacroR,tmp2
		CALL Delay

		CLC
		ROL tmp1
		SBIC SPI_PIN,SPI_MISO_PIN
			ORI tmp1,1
		CBI	SPI_PORT,SPI_SCK_PIN

		MOV MacroR,tmp2
		CALL Delay

		DEC Counter
	BRNE /*!=*/ SPI_for
	//Прошли весь байт
	ST Z,tmp1
	LDS tmp1,SPIBUF_c
	INC tmp1
	LDS tmp2,SPIBUF_l
	CP tmp1,tmp2
	BRSH /*>=*/ SPI_end_endRead
		STS SPIBUF_c,tmp1
		SetTask TS_SPI //В конец очереди, мы и так потратили не реальное время
		RJMP SPI_end_00
	SPI_end_endRead:
		STI SPIBUF_c,0
		SetTask TS_SPI_End,1
	SPI_end_00:
RET