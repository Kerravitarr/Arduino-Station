/*
 * macro.inc
 *
 *  Created: 11.05.2021 22:47:03
 *   Author: Terran
 */ 
 //������� ���� �� ������ ������
 .MACRO CBIFlag
	LDS MacroR,Flag
	ANDI MacroR, ~(1 << @0)
	STS Flag,MacroR
 .ENDMACRO
 //��������� ������� ����� � ������ ������
  .MACRO TSTFlag
	LDS MacroR,Flag
	ANDI MacroR,(1 << @0)
 .ENDMACRO
 //������������� ���� � ������ ������
 .MACRO SBIFlag
	LDS MacroR,Flag
	ORI MacroR, (1 << @0)
	STS Flag,MacroR
 .ENDMACRO

 
  .MACRO 	OUTI				; ��� �������� �������. ����� ��� outi ��������� � ����, �� ��������� �� 
	LDI 	MacroR,@1 			; ���� ����� ����, ������ @0,@1 ��� ���������, ��� �������� ���������� �����������
	OUT 	@0,MacroR 			; �������. ?����� ������ ���� �������� ��������� ����� ������� � ������� R16, � �� ���� 
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
//��������� ����� � ������. ������=�����
.MACRO STI
	LDI MacroR,@1
	STS @0,MacroR
.ENDMACRO
//���������� ����� � ��������. ������� += �����
.MACRO		ADI
	SUBI	@0,-@1
.ENDMACRO
//��������� ����� � ������� (���� ��� ������ �������!!!) �������=�����
 .MACRO LDIl
	LDI MacroR,@1
	MOV @0,MacroR
 .ENDMACRO
 //�������� � ���� ������
 .MACRO UARTPrint
	LDI MacroR,@0
	CALL UART_Print
 .ENDM
 //�������� � ���� ������ �� ������
 .MACRO UARTPrint_m
	LDS MacroR,@0
	CALL UART_Print
 .ENDM
 //�������� � ���� ������ �� ��������
 .MACRO UARTPrint_r
	MOV MacroR,@0
	CALL UART_Print
 .ENDM
 //�������� � ���� ������ �� ����������� ����
 .MACRO UARTPrint_rm
	LD MacroR,@0
	CALL UART_Print
 .ENDM
 //������ ��� ������� I2� ������� � ����� ���������� - ������ ������
.MACRO I2CStart
		STI TWI_IOc, 0			//�������� ����� ���������
		STI TWI_IOl, @0			//����� ������������� ���������
		SetTask TS_StartTWI		//��������� ������
.ENDMACRO
 //������ ��� ������� I2� ������� � ����� ���������� - ���������, ��� ����� ����� ������
.MACRO I2CStart_r
		STI TWI_IOc, 0			//�������� ����� ���������
		STS TWI_IOl, @0			//����� ������������� ���������
		SetTask TS_StartTWI		//��������� ������
.ENDMACRO
 //������ ��� ������� I2� ������� � ����� ����������� - ��������� ����� ������ � ���������-�������
.MACRO I2CRead_r
		STI TWI_IOc, 0			//�������� ����� ���������
		STS TWI_IOl, @0			//����� ������������� ���������
		STS TWI_IO, @1			//����� ����� ����
		SetTask TS_StartTWI		//��������� ������
.ENDMACRO

.MACRO SetTask
		ldi MacroR, @0			; ��������� � ��������� ����� ���������� ���������
		call SendTask				; 
.ENDMACRO

.MACRO SetTimerTask
		LDI 	MacroR, @0
		LDI 	XL, Low(@1)				;
		LDI 	XH, High(@1)			; �������� � ������������
		CALL 	SetTimer
.ENDM

.MACRO WriteInBuf_Z
		LDI 	ZL,low(@0)
		LDI 	ZH,high(@0)
		MOV		MacroR,@1
.ENDM

//������� � �������������
.MACRO MK_Delay
	.if (@0) > 0                  ; ���� @0 <= 0 -- ��� ��������� ��������
		.if (@0*F_CPU/1000000)/4 > 250          ; ������ ������ ������, ������� ������
			.error "@0 must be less or equal 250*4"
		.else
			.if (@0*F_CPU/1000000)/4 >= 1                ; ����� �� 4, �.�. 1 �������� ����� -- 4 �����
				ldi     MacroR, (@0*F_CPU/1000000)/4
				WDR						  ; 1 ����
				subi    MacroR, 1         ; 1 ����
				brcc    (PC-2)            ; 2 �����
				; �� ��������� ���� brcc ���������� �� 1 ����, �� � ������ ldi �����-����
				; ��������� 2 �����
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
		//����� � ��� ������ �������  �����������, � ��� ��������
		SBI	SPI_PORT,SPI_SS_PIN
		RJMP PC+1
		CBI	SPI_PORT,SPI_SS_PIN
.ENDM
.MACRO AVR_ISP_NOT_RST
	TSTFlag rst_active_high
	BRNE /*!=*/ PC+3
		//����� � ��� ������ �������  �����������, � ��� ��������
		CBI	SPI_PORT,SPI_SS_PIN
		RJMP PC+1
		SBI	SPI_PORT,SPI_SS_PIN
.ENDM