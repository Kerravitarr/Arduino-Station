/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package ComPort;

/**
 *Настройки порта
 * @author Илья
 */
public class Settings {
	/**Режимы, которые знает дуня*/
	public static enum MODE{
		/**Сохранить скорость*/
		SET_BAUND(0x03),
		/**Сохранить данные в порту B*/
		SET_PORTB(0x04),
		/**Сохранить данные в порту С*/
		SET_PORTC(0x05),
		/**Сохранить данные в порту D*/
		SET_PORTD(0x06),
		/**Сохранить настройки АЦП*/
		SET_ADC(0x07),
		/**Получить все регистры всех портов сразу*/
		GET_PORT(0x08),
		/**Передать сообщение через I2C*/
		WRITE_I2C(0x09),
		/**Прочитать данные через I2C*/
		READ_I2C(0x0A),
		/**Прверка связи*/
		GET_STATUS(0x20),
		;
		/**Команда, которая делает то или иное действие*/
		public final byte CMD;
		private MODE(int c){CMD = (byte) c;}
	}
	/**Возвможные ответы модуля*/
	public static enum ANSWER {
		NOT_ANS(0xFF00),
		UNDEFENDED(0xFF01),
		STK_OK(0x10),
		CRC_EOP(0x20),
		STK_FAILED(0x11),
		STK_UNKNOW(0x12),
		STK_INSYNC(0x14),
		STK_NONSYNC(0x15),
		;
		/**Команда, которая делает то или иное действие*/
		public final int CMD;
		public static final ANSWER[] values = ANSWER.values();
		private ANSWER(int c){CMD = (byte) c;}
		public static ANSWER toAns(byte num){
			for(var a : values){
				if(a.CMD == Byte.toUnsignedInt(num))
					return a;
			}
			return UNDEFENDED;
		}
	}
	
}
