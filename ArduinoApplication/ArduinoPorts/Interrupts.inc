/*
 * Interrupts.inc
 *
 *  Created: 18.05.2021 9:27:22
 *   Author: Terran
 */ 

 OneMsTimer:
			in 		interrupt,SREG			; Save Sreg
			PUSH 	interrupt				; ���������� �������� OSRG � �������� ��������� SREG

			PUSH 	ZL	
			PUSH 	ZH						; ���������� �������� Z
			PUSH 	Counter					; ���������� �������� Counter
			PUSH 	MacroR					; ���������� �������� MacroR
	
			LDI 	ZL,low(TimersPool)		; �������� � ������� Z ������ RAM, 
			LDI 	ZH,high(TimersPool)		; �� �������� ��������� ���������� � ��������

			LDI 	Counter,TimersPoolSize	; ������������ ���������� ��������
	
Comp1L01:	ld 		interrupt,Z				; interrupt = [Z] ; �������� ����� �������
			cpi 	interrupt,$FF			; ��������� �� "NOP"
			breq 	Comp1L03				; ���� NOP �� ������� � ��������� �������

			clt								; ���� T ������������ ��� ���������� ���������� �� ��������� �����
			ldd 	interrupt,Z+1			; 
			subi 	interrupt,Low(1) 		; ���������� ������� ����� �������� �� 1
			std 	Z+1,interrupt			;
			breq 	Comp1L02				; ���� 0 �� ���� T �� �������������
			set								; 

Comp1L02:	ldd 	interrupt,Z+2			;
			sbci 	interrupt,High(1) 		; ���������� ������� ����� �������� �� 1
			std 	Z+2,interrupt			;
			brne 	Comp1L03				; ���� �� �������
			brts 	Comp1L03				; ���� �� ������� (�� T)	
	
			ld 		MacroR,Z				; �������� ����� �������
			CALL 	SendTask				; ������� � ��������� ������� �����
	
			LDI 	interrupt,$FF			; = NOP (������ ���������, ������ �������������)
			st 		Z, interrupt			; Clear Event

Comp1L03:	subi 	ZL,Low(-3)				; Skip Counter
			sbci 	ZH,High(-3)				; Z+=3 - ������� � ���������� �������
			dec 	Counter					; ������� ��������
			brne 	Comp1L01				; Loop	


			LDS 	ZL,ST_Timer				; �������� � ������� Z ������� � ��
			LDS 	ZH,ST_Timer+1				; �������� � ������� Z ������� � ��

			cpi   ZL,low(60000)				;��������� ������� ����
			ldi   interrupt,high(60000)		;
			cpc   ZH,interrupt				;��������� �������� �����
			BREQ /*=*/ OCI_01
				ADIW ZL,1
				STS ST_Timer,ZL
				STS ST_Timer+1,ZH
				RJMP OCI_00
			OCI_01:
			;������ 60000 ����������, ��� 60�, ��� 1 ������
				//SlowTimerService
				CLR interrupt
				STS ST_Timer,interrupt
				STS ST_Timer+1,interrupt
			OCI_00:
			
			pop 	MacroR					; ��������������� ����������
			pop 	Counter					; 
			pop 	ZH
			pop 	ZL

			pop 	interrupt				; ��������������� ��������
			out 	SREG,interrupt			; 
RETI						; ������� �� ����������

DataIN:
			//SetTask TS_I
			LDI 	interrupt, 1
			OUT		EIFR,interrupt
RETI

WDT:
	//�������� �������
	wdr
	//� ������ ������ �� ������ ����� �� ������ � ��� ������� ����
	POP Zh
	POP Zl
	LDI Yl, low(WDRM)
	LDI Yh, high(WDRM)
	CP		Zl, YL
	CPC		Zh, YH
	BRNE WDT01 //���� �� �����, �� �� �������
		//�� �� �������, ��� ��� �� �� ����, �� �� ������ �������� �� ������� ��� ������������
JMP Background
	WDT01:
		//�� ���������� ��� �������, ���� ���������������
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
		CPI	OSRG,0x00			//Bus Fail ������� ��������� ��� � ������ ���������� ������ ����. ��������, ��������� ����� ������� �������� ����.
		BRNE	TWIE01
				//���������
		TWIE01:
*/
		CPI	interrupt,0x08			//Start ��� ������ �����. ������ �� ������ ��� ������ ������, �������� ������� ����� ��������
		BRNE	TWIE02
			LDS		interrupt,TWI_IO
			UOUT TWDR,interrupt			//����� �����
			LDI		interrupt,1	
			STS		TWI_IOc,interrupt	//�������� ���� ������
			RJMP TWIE_SEND_WRITE
		TWIE02:
/*
		CPI	OSRG,0x10			//ReStart ��� ��������� ��������� �����. ����� ������������� � ������ �� ������ ��� ��������. �� ������ �������.
		BRNE	TWIE10
				//���������
				RJMP TWIE00
		TWIE10:
*/
		CPI	interrupt,0x18 			//SLA+W+ACK �� ��������� ����� � ����� ������, � � ����� �������� ACK �� ��������. ������ ����� ����������.
		BRNE	TWIE18
			RJMP TWIE28_st //��������� �������� ��� ��� 0�28
		TWIE18:

		CPI	interrupt,0x20			//SLA+W+NACK �� ��������� ����� � ����� ������, � ��� ������� NACK. ������, �������� ������ ��� �������� ��� ���.
		BRNE	TWIE20
				RJMP TWIE_SEND_ERROR
		TWIE20:


		CPI	interrupt,0x28 			//Byte+ACK �� ������� ���� � �������� �������������, ��� ������� ��� ������. ����������.
		BRNE	TWIE28
			TWIE28_st:
			LDS		interrupt,TWI_IOc		//������� �� �������
			LDS		MacroR,TWI_IOl			//������� �� ������ �������
			CP interrupt,MacroR
			BREQ /*=*/ TWIE2802 //��, �������, �� ���������, ���!
				//�� �������, ����� ������
				LDI 	ZL,low(TWI_IO)
				LDI 	ZH,high(TWI_IO)
				ADD		Zl,interrupt //�������� ��������
				sbci	ZH, 0
				INC		interrupt				//+1, � ��� ���������
				STS		TWI_IOc,interrupt

				LD		interrupt,Z
				UOUT	TWDR,interrupt //������� �����
				RJMP	TWIE_SEND_ACK
			TWIE2802:
				SetTask TS_TWI_IO_End
			RJMP TWIE_SEND_STOP
		TWIE28:
/*
		CPI	OSRG,0x30			//Byte+NACK �� ������� ����, �� ������������� �� ��������. ������ ������� ��� ��� �� ����� ������ ��������� ��� �� ����������� � ������. ���� ��� �������� ������� �������� ������ ������ ������������.
		BRNE	TWIE30
				//���������
				RJMP TWIE00
		TWIE30:

		CPI	OSRG,0x38			//Collision � � ��� ��� �������� �������� � ������ ������ ������, �� ������ ��� �������, �� ���, ��� �� �� ���������� �� ����������. ������ I�l be back! �� ������� ����� n ������!
		BRNE	TWIE38
				//���������
				RJMP TWIE00
		TWIE38:
*/
		CPI	interrupt,0x40		//SLA+R+ACK ������� ����� � ����� �� ������, � ������� ���������. ������! ����� ������.
		BRNE	TWIE40
			TSTFlag TWI_Busy
			BREQ /*=*/ TWIE4002//��� ������� �� �����, �� ���� �� �����, ����� ��� ��, �� ���� �� �����
				//��� �� ������� ���� ��, �������. ������!
				LDI		interrupt,0	
				STS		TWI_IOc,interrupt	//�������� ���� ������
				RJMP TWIE_SEND_ACK
			TWIE4002:
				//�� ���� �� �����, ��� �� ��
			RJMP TWIE_SEND_NACK
		TWIE40:

		CPI	interrupt,0x48			//SLA+R+NACK �������� � ���� ��� ��, � ������� ���, ������� ��� ������ � � ����� ���� NACK!� � ������ �� ������ ������ � ����� ������ ����� �� �����������. ������ �� ����� ��� ������. ����� ����� ���� ������ ��� ����.
		BRNE	TWIE48
				RJMP TWIE_SEND_ERROR
		TWIE48:

		CPI	interrupt,0x50 		//Receive Byte �� ������� ����. � ������ ��� �� �������� ��������. ACK ��� NACK.
		BRNE	TWIE50	
			LDS		interrupt,TWI_IOc		//������� �� ���������
			CPI		interrupt,TWISize
			BREQ /*=*/ TWIE_SEND_NACK//���������, � ��� ���������� � �� ��� �� ���� �� ������. ����������� ����
				//�� ��, ������, ����� ����
				LDI 	ZL,low(TWI_IO)
				LDI 	ZH,high(TWI_IO)
				ADD		Zl,interrupt //�������� ��������
				sbci	ZH, 0
				INC		interrupt				//+1, � ��� ���������
				STS		TWI_IOc,interrupt
				LDS		MacroR,TWDR
				ST		Z,MacroR		//��� �� � ��������� ��� ����
				LDS		MacroR,TWI_IOl
				CP		interrupt,MacroR
				BRNE /*!=*/ TWIE_SEND_ACK //�� ��, ����� ����
					//���������, ��� ��� ���������. ���� ��� �� �������, ������. ��� ����� ����
					SetTask TS_TWI_IO_End	
					RJMP TWIE_SEND_NACK		//����� �� NACK
			RJMP TWIE_SEND_ACK
		TWIE50:

		CPI	interrupt,0x58			//Receive Byte+NACK �� ������� ���� �� �������� � ������� ��� ���� NACK!� � �� �������� ����, ��������� ����.
		BRNE	TWIE58
				RJMP TWIE_SEND_STOP
		TWIE58:

/*		CPI	OSRG,0x60			//Receive SLA+W ����� �� ����, ������ �� �������, �� � ��� �� ��������. � ��� ��� �� ����� ������� ��������� :)
		BRNE	TWIE60
				//���������
				RJMP TWIE00
		TWIE60:

		CPI	OSRG,0x68			//Receive SLA+W LP �� ���� ��������, ��������� � ������������ �� ����. � ��� ���������� �� ���� ������, ����� ������, ������, ���������� ��� � ������ ���������� ��, � �� �������� �� �� ��� ��� ������腻 ����, �� �����. �������� ������� �������� � ����� ��� ����� ����.
		BRNE	TWIE68
				//���������
				RJMP TWIE00
		TWIE68:

		CPI	OSRG,0x70			//Receive SLA+W Broadcast �������� �����������, �� �� ���� ��� ������ ��� �������� ��� ������, ������. ���? � ����? ���������� Broadcast �������? ��� ���? ������� �� ��������� ������� ���������.
		BRNE	TWIE70
				//���������
				RJMP TWIE00
		TWIE70:

		CPI	OSRG,0x78			//Receive SLA+W LP Broadcast ���� ��, ��� ��� ��������, ������ ������� ��������� �� ����. ���� �� �������� ������ ������ � ������� ��� ����������� �� ��� ���� ���, ����� ����. ������ ���� �������-����, � �������������� ������������������ ��������� �����������. ��������� �������������� � ������ Broadcast-����� ����������.
		BRNE	TWIE78
				//���������
				RJMP TWIE00
		TWIE78:

		CPI	OSRG,0x80			//Receive Byte & 0x90 Receive Byte Broadcast ��������� �����. �� ���� � � ����� ���� �� �����. ������ ��� ������� ����� ��� (ACK) ��� ���� NACK�. ��� ��� �� ���������������.
		BRNE	TWIE80
				//���������
				RJMP TWIE00
		TWIE80:

		CPI	OSRG,0x88			//Receive Last Byte & 0x98 Receive Last Byte Broadcast ������� ��������� ���� � ����������� �� ��������.
		BRNE	TWIE88
				//���������
				RJMP TWIE00
		TWIE88:

		CPI	OSRG,0xA0			//Receive ReStart �� � ��� ��������� �����. ������ �� ��� ������ � ������ ��� ��� ���� ����� ��������. � ������ ������ ������
		BRNE	TWIEA0
				//���������
				RJMP TWIE00
		TWIEA0:

		CPI	OSRG,0xA8			//Receive SLA+R ���� ������ � ��� ����� �� ������ ������ �� ����� ���������� � ������� ��� �������� ��������.
		BRNE	TWIEA8
				//���������
				RJMP TWIE00
		TWIEA8:

		CPI	OSRG,0xB0			//Receive SLA+R LP ����� �� ��� �� �����, � ��� ��� ���������� ������ ������, ���������� �� ����� � ������� �� �� �� ������� ��� ��� ������ �� ������� ��� ������, ���������� ������.
		BRNE	TWIEB0
				//���������
				RJMP TWIE00
		TWIEB0:

		CPI	OSRG,0xB8			//Send Byte Receive ACK �� ���� �� ��� ����. �� ��� ACK. � �� ��� �������� ������ ����� ��� ��� ���� (���������) � �������� ���� NACK�. ��� �� � ��� ������ �� � ����� ��� ����������.
		BRNE	TWIEB8
				//���������
				RJMP TWIE00
		TWIEB8:

		CPI	OSRG,0xC0			//Send Last Byte Receive NACK ���� ������� ��������� ��������� ����, � �� ��� ���� NACK�. �����. �� � ���� � ���. ������ � ����.
		BRNE	TWIEC0
				//���������
				RJMP TWIE00
		TWIEC0:

		CPI	OSRG,0xC8			//Send Last Byte Receive ACK ���� ������� ��������� ��������� ����, � �� ������� ���. �� � ��� ����, ��� ��� ��������������� � ������ � ����. � �� ����� ������� �������� ��������� (� ���� ������ ������ ������ �������� ����� �� slave 0xFF �����, �� ����� ���� ��� ������ ������ ������� ����).
		BRNE	TWIEC8
				//���������
				RJMP TWIE00
		TWIEC8:
*/

	/*
		TWINT � ���� ����������. ����� ��� �������� ��� �������� ������� ����������� ������, � ���������� ����� ����� ��������.
		TWEA � Enable ACK. ���������� ������ ACK. ���� ��� ��������, �� ������� TWI ����� ���������� �� ���� �����, � ����� ������ ACK �� ���� ������� ����� ����� ������� ��������. ������, ����� ����� �����. ���� ��� ���� ������� NACK �� ��� �� ������.
		TWSTA � ������������ �����. ������ �� ����, ��� ����� ������. �� �������� ��������� �������. ��������, ���� ���� ��� ��������� � 0�38 ���������, �� ����� ����� ����������� ����� ���� �����������. ���������� TWI ����� � ��� ��� ������ �����.
		TWSTO � ������� Stop. ����� �� ����������. ����� ������ � ���� ��� ������ ������������ �� ������ ��������. �� ���������� �������������� �������.
		TWWC � �������� ������. ������������� � ���, ��� ��� �� �� ���������� ��������� ������� � TWDR ������. ����� ��� � TWDR ���������� ������ ������ ��� �������� TWINT (� ����� ������ ��� ����� ������ � ����������� ����������).
		TWEN � ���� TWI �������. ������ � �����.
		TWIE � ���������� �� TWI ���������.
	*/
	//���� �� ���� �� ������� - �� �� ������� ������ �����!

	//������!
	TWIE_SEND_ERROR:
		SetTask TS_TWI_IO_Error
		STS	TWI_IOl, interrupt	//�������� ��� ������!
	RJMP TWIE_SEND_STOP


	TWIE_SEND_ACK:
		LDI interrupt,(1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE)
		RJMP TWIE_SEND
	TWIE_SEND_WRITE: //������ ����� � ����, ��� ������� ���
	TWIE_SEND_NACK:
		LDI interrupt,(1<<TWINT)|(1<<TWEN)|(1<<TWIE)	
		RJMP TWIE_SEND
	TWIE_SEND_STOP:
		LDI interrupt,(1<<TWINT)|(1<<TWSTO)|(1<<TWEN)|(1<<TWIE)	
		//RJMP TWIE_SEND
	TWIE_SEND:			
	UOUT TWCR ,interrupt
	TWIE00:
		POP 	interrupt				; ��������������� ��������
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
			ADD		Zl,MacroR //�������� ��������
			sbci	ZH, 0
			LD		interrupt,Z
			UOUT	UDR0,interrupt		; ������ ������ � �����.
			INC		MacroR
			CPI		MacroR,UARTSize
			BRLO /*<*/ UART_end_not_return
				CLR MacroR
			UART_end_not_return:
			STS UART_O_tail,MacroR
			RJMP UARTE00
		UART_END_transmit:
			//��, ����������� ��������
			LDI 	interrupt, (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)|(0<<TXCIE0)|(0<<UDRIE0)
			UOUT 	UCSR0B, interrupt	
		UARTE00:
		POP 	interrupt				; ��������������� ��������
		out 	SREG,interrupt			; 
		POP		MacroR
		POP		ZH
		POP		ZL
RETI

UARTCaddr:
		PUSH	MacroR
		LDS	interrupt,UDR0		; ����� �����
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
		BREQ /*=*/ EEPROMrdy_end//���������� �������� �����������, � ������ �������� � ���
			//� ��� ��� ��� �� ���� ��� ������/������
			DEC interrupt
			STS EEPROMPos_c,interrupt //� ��������� ��� ����� ������ ������

			LDS	Zl,EEPROMPos_s
			CLR Zh
			OUT 	EEARL,ZL 		; ��������� ����� ������ ������
			OUT 	EEARH,ZH  		; ������� � ������� ���� ������

			INC Zl
			STS EEPROMPos_s,Zl		;� ��������� ��� ����� ����� ���������

			LDI 	ZL,low(EEPROMStr) ;������ ����� � ��� �����
			LDI 	ZH,high(EEPROMStr)			
			TSTFlag EEPROM_write
			BREQ /*=*/ EEPROMrdy_read//������
				//������
				LD		MacroR,Z
				OUT 	EEDR,MacroR 	; ����������, ����������, ������
 
				SBI 	EECR,EEMPE 		; ������� ��������������
				SBI 	EECR,EEPE 		; ���������� ����

				TST interrupt
				BREQ /*=*/ EEPROMrdy_end//������ ������ ������
					lds 	MacroR, EEPROMPos_c
					EEPROMrdy_left:					;�������� ����� �� ���� ���
						ldd 	interrupt, Z+1 		;����� ��������� �����
						st 		Z+, interrupt		;��������� � �������
						dec 	MacroR		
					brne 	EEPROMrdy_left		; Loop
				RJMP EEPROMrdy_next
			EEPROMrdy_read:
				//������
				SBI  EECR,EERE          ; ��������� ������ �����
				IN		MacroR, EEDR     ; ������ ������
				ST		Z,MacroR
				TST interrupt
				BREQ /*=*/ EEPROMrdy_end//������ ������ ������
					LDI 	ZL,low(EEPROMStr + EEPROMMaxSize - 2)
					LDI 	ZH,high(EEPROMStr + EEPROMMaxSize - 2)	;����� ������������� ����
					ldi 	MacroR, EEPROMMaxSize-1
					EEPROMrdy_right:
						ld	 	interrupt, Z+ 		;����� ������� �����
						st 		Z, interrupt		;��������� � ���������
						SBIW	Zl,2				;������� � �����������
						dec 	MacroR		
					brne 	EEPROMrdy_right		; Loop
					//������� �����, ���� ��������		
					RJMP EEPROMrdy_next	
		EEPROMrdy_end:
			CBI 	EECR,EERIE	; ���������� ����������, ������ ��� ��� �� �����
			SetTask TS_EEPROM_End
		EEPROMrdy_next:
		POP 	interrupt				; ��������������� ��������
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
	BRNE /*!=*/   ADCend_start //�� �����, ���� �������, �� �� ����� �������� ���������. ��� ��� ������ ��� ��� ��� ����
	//�� �������, ���� �� �������.
	UIN	MacroR,ADCH
	RCALL UART_Print_withuotCLI
	//������������� ���
	UIN interrupt,AVRISP
	ANDI interrupt,~(1 << ADSC) //����!
	UOUT AVRISP,interrupt	
	//�������� ��������� �����
	UIN interrupt,ADMUX
	ANDI interrupt,0x0F
	CPI interrupt,8
	BRSH /*>=*/ ADCend_stop//� ��� ��������� �� �������, �������
		LDI tmp1,1
		INC interrupt //��� ��� ������ ���������� � 0
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
				//���� ���������, �������� �������������
				LDI MacroR,0xFF
				RCALL UART_Print_withuotCLI
				ROL MacroR //�������� �� ������� �����
			ADCend_cyc2_notC:
			MOV MacroR,interrupt
			AND MacroR,tmp1
		BREQ /*=*/ ADCend_cyc2 //���� �����, �������� � ������ ������
		MOV MacroR,tmp1
		SER tmp1
		ADCend_cyc3:
			INC tmp1
			LSR MacroR
		BRNE /*!=*/ ADCend_cyc3

		UIN interrupt,ADMUX
		ANDI interrupt,0xF0 //���������� ����� �����
		OR	interrupt,tmp1
		UOUT ADMUX,interrupt
	
	ADCend_start:
		UIN interrupt,AVRISP
		ORI	interrupt,(1 << ADSC) //������!
		UOUT AVRISP,interrupt	
		RJMP ADCend_end
	ADCend_stop:
		CLR	interrupt //������ ���������
		UOUT ADCSRA,interrupt	
	ADCend_end:

	POP 	interrupt				; ��������������� ��������
	out 	SREG,interrupt			; 
	POP 	tmp1
	POP 	MacroR
RETI

//������������ ���, ��� �� �� ����������
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
	//������ ���� ����
	ST Z,tmp1
	LDS tmp1,SPIBUF_c
	INC tmp1
	LDS tmp2,SPIBUF_l
	CP tmp1,tmp2
	BRSH /*>=*/ SPI_end_endRead
		STS SPIBUF_c,tmp1
		SetTask TS_SPI //� ����� �������, �� � ��� ��������� �� �������� �����
		RJMP SPI_end_00
	SPI_end_endRead:
		STI SPIBUF_c,0
		SetTask TS_SPI_End,1
	SPI_end_00:
RET