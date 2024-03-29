/*
 * AVRISP.inc
 *
 *  Created: 18.05.2021 15:50:21
 *   Author: Terran
 */ 

 StartProgrammMode:
	//TODO ����� ���� ����! ��� ��� ������ � �������!
	AVR_ISP_RST
	//�o��� ������ ���������� ��������, ����� � ����������� ��� �� �����
	SBI	SPI_DDR,SPI_SS_PIN

	CBI	SPI_PORT,SPI_SCK_PIN
	CBI	SPI_PORT,SPI_MOSI_PIN

	SBI	SPI_DDR,SPI_SCK_PIN
	SBI	SPI_DDR,SPI_MOSI_PIN
	CBI	SPI_DDR,SPI_MISO_PIN
	
	// ������� ������ ���������� �� ����� 2 �������� ������ �������� ����������,
	// ������� 100 ��� �������� ��� ��������� ���������� ���� 20 ���
	STI AVRISP_c,2 //��� ������ 100 ���

	SetTimerTask TS_StartProgrammMode_State1,20
 RET

StartProgrammMode_State1:
	LDS tmp1,AVRISP_c
	CPI tmp1,2
	BRNE /*!=*/ SPMS_notStart
		//� ��� ������ ������, ��������� ����� � �������� ����� �����
		AVR_ISP_NOT_RST
	SPMS_notStart:
	CPI tmp1,0
	BREQ /*=*/ SPMS_Start
		//����� �����
		MK_Delay (50-10) //-10 ����� �������� �� 23 ��� �����, 23 ��� ��� ����
		DEC tmp1
		STS AVRISP_c,tmp1
		SetTask TS_StartProgrammMode_State1
		RJMP SPMS_end
	SPMS_Start:
		//����� ����� ����������, ���������. � ���������� ���������� ��� 23 ���. �� ��� �� �� �������
		AVR_ISP_RST

		STI SPIBUF,0xAC
		STI SPIBUF+1,0x53
		STI SPIBUF+2,0x00
		STI SPIBUF+3,0x00
	
		STI SPIBUF_l,0x04
	
		SBIFlag SPI_Busy
		SetTimerTask TS_SPI,50
	SPMS_end:
RET


AVRISP_flash_read_page:
	LDS Zh,LOAD_ADDRESS
	LDS Zl,LOAD_ADDRESS+1 //�������!
	LDS tmp3,AVRISP_c+1 //�������, � ������ ������
	
	STS SPIBUF+1,Zh
	STS SPIBUF+2,Zl
	STI SPIBUF+3,0x00

	SBRC tmp3,0
		RJMP AVRISP_flash_read_page_HIGH
	AVRISP_flash_read_page_low:
		//� ��� ������ ����� flash
		STI SPIBUF,0x20
		RJMP AVRISP_flash_read_page_00
	AVRISP_flash_read_page_HIGH:
		//� ��� ������ ����� flash
		STI SPIBUF,0x28
		ADIW Zl,1
		STS LOAD_ADDRESS,Zh
		STS LOAD_ADDRESS+1,Zl
	AVRISP_flash_read_page_00:
	
	STI SPIBUF_l,0x04
	
	SBIFlag SPI_Busy
	SetTask TS_SPI
RET
AVRISP_eeprom_read_page:
	//addr = here * 2 + x;
	LDS tmp1,LOAD_ADDRESS
	LDS tmp2,LOAD_ADDRESS+1 //�������!
	CLC
	ROL tmp2
	ROL tmp1 //*2
	LDS tmp3,AVRISP_c+1 //�������, � ������ ������
	ADD tmp2,tmp3
	LDS tmp3,AVRISP_c
	ADC tmp1,tmp3 //+x

	STI SPIBUF,0xA0
	STS SPIBUF+1,tmp1
	STS SPIBUF+2,tmp2
	STI SPIBUF+3,0xFF

	STI SPIBUF_l,0x04
	
	SBIFlag SPI_Busy
	SetTask TS_SPI
RET

AVRISP_err:
	STI Mode,DefaultMode
	UARTPrint STK_FAILED
RET

AVRISP_write_flash_pages:

	LDS Zh,LOAD_ADDRESS
	LDS Zl,LOAD_ADDRESS+1 //�������!
	STS SPIBUF+1,Zh
	STS SPIBUF+2,Zl
	LDS		tmp1,AVRISP_c
	LDS		tmp2,AVRISP_c+1 //�������!
	LDI 	ZL,low(AVRISP)
	LDI 	ZH,high(AVRISP)
	ADD		ZL,tmp2
	ADC		ZH, tmp1
	LD		tmp2,Z
	STS		SPIBUF+3,tmp2

	LDS Zh,LOAD_ADDRESS
	LDS Zl,LOAD_ADDRESS+1 //�������!
	LDS tmp3,AVRISP_c+1 //�������, � ������ ������
	SBRC tmp3,0
		RJMP AVRISP_write_flash_pages_HIGH
	AVRISP_write_flash_pages_LOW:
		//� ��� ������ ����� flash
		STI SPIBUF,0x20
		RJMP AVRISP_write_flash_pages_00
	AVRISP_write_flash_pages_HIGH:
		//� ��� ������ ����� flash
		STI SPIBUF,0x28
		ADIW Zl,1
		STS LOAD_ADDRESS,Zh
		STS LOAD_ADDRESS+1,Zl
	AVRISP_write_flash_pages_00:

	STI SPIBUF_l,0x04
	
	SBIFlag SPI_Busy
	SetTask TS_SPI
RET


AVRISP_commitPage:
	LDS tmp1,AVRISP_page
	LDS tmp2,AVRISP_page+1 //�������!

	STI SPIBUF,0x4C
	STS SPIBUF+1,tmp1
	STS SPIBUF+2,tmp2
	STI SPIBUF+3,0x00

	STI SPIBUF_l,0x04

	SBIFlag SPI_Busy
	SetTask TS_SPI

	STI Mode,AVRISP_STK_PROG_PAGE_COMMIT
RET