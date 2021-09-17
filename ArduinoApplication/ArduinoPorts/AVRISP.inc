/*
 * AVRISP.inc
 *
 *  Created: 18.05.2021 15:50:21
 *   Author: Terran
 */ 

 StartProgrammMode:
	//TODO Нужен тест ноги! Что она правда в воздухе!
	AVR_ISP_RST
	//Кoгда задали правильное значение, можем и переключать его на вывод
	SBI	SPI_DDR,SPI_SS_PIN

	CBI	SPI_PORT,SPI_SCK_PIN
	CBI	SPI_PORT,SPI_MOSI_PIN

	SBI	SPI_DDR,SPI_SCK_PIN
	SBI	SPI_DDR,SPI_MOSI_PIN
	CBI	SPI_DDR,SPI_MISO_PIN
	
	// Импульс должен составлять не менее 2 тактовых циклов целевого процессора,
	// поэтому 100 мкс подходят для скоростей процессора выше 20 кГц
	STI AVRISP_c,2 //Для отмера 100 мкс

	SetTimerTask TS_StartProgrammMode_State1,20
 RET

StartProgrammMode_State1:
	LDS tmp1,AVRISP_c
	CPI tmp1,2
	BRNE /*!=*/ SPMS_notStart
		//У нас первый запуск, запускаем ресет и включаем режим ждуна
		AVR_ISP_NOT_RST
	SPMS_notStart:
	CPI tmp1,0
	BREQ /*=*/ SPMS_Start
		//Режим ждуна
		MK_Delay (50-10) //-10 Чтобы уровнять те 23 мкс снизу, 23 это без этих
		DEC tmp1
		STS AVRISP_c,tmp1
		SetTask TS_StartProgrammMode_State1
		RJMP SPMS_end
	SPMS_Start:
		//Режим ждуна закончился, запускаем. В реальности докинулось ещё 23 мкс. Но это же не страшно
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
	LDS Zl,LOAD_ADDRESS+1 //Младший!
	LDS tmp3,AVRISP_c+1 //Младший, в данном случае
	
	STS SPIBUF+1,Zh
	STS SPIBUF+2,Zl
	STI SPIBUF+3,0x00

	SBRC tmp3,0
		RJMP AVRISP_flash_read_page_HIGH
	AVRISP_flash_read_page_low:
		//У нас низшая часть flash
		STI SPIBUF,0x20
		RJMP AVRISP_flash_read_page_00
	AVRISP_flash_read_page_HIGH:
		//У нас высшая часть flash
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
	LDS tmp2,LOAD_ADDRESS+1 //Младший!
	CLC
	ROL tmp2
	ROL tmp1 //*2
	LDS tmp3,AVRISP_c+1 //Младший, в данном случае
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
	LDS Zl,LOAD_ADDRESS+1 //Младший!
	STS SPIBUF+1,Zh
	STS SPIBUF+2,Zl
	LDS		tmp1,AVRISP_c
	LDS		tmp2,AVRISP_c+1 //Младший!
	LDI 	ZL,low(AVRISP)
	LDI 	ZH,high(AVRISP)
	ADD		ZL,tmp2
	ADC		ZH, tmp1
	LD		tmp2,Z
	STS		SPIBUF+3,tmp2

	LDS Zh,LOAD_ADDRESS
	LDS Zl,LOAD_ADDRESS+1 //Младший!
	LDS tmp3,AVRISP_c+1 //Младший, в данном случае
	SBRC tmp3,0
		RJMP AVRISP_write_flash_pages_HIGH
	AVRISP_write_flash_pages_LOW:
		//У нас низшая часть flash
		STI SPIBUF,0x20
		RJMP AVRISP_write_flash_pages_00
	AVRISP_write_flash_pages_HIGH:
		//У нас высшая часть flash
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
	LDS tmp2,AVRISP_page+1 //Младший!

	STI SPIBUF,0x4C
	STS SPIBUF+1,tmp1
	STS SPIBUF+2,tmp2
	STI SPIBUF+3,0x00

	STI SPIBUF_l,0x04

	SBIFlag SPI_Busy
	SetTask TS_SPI

	STI Mode,AVRISP_STK_PROG_PAGE_COMMIT
RET