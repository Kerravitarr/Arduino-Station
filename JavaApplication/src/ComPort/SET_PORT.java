/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package ComPort;

/**Сохраняет данные по порту B
 * @author Илья
 */
public class SET_PORT extends Request{	
	private SET_PORT(Settings.MODE mode, byte PORT, byte DDR) {
		super(mode, new byte[]{PORT, DDR});
	}
	/**Сохраняет данные по порту B
	 * @param PORT состояние выходов - вкл/выкл
	 * @param DDR направление выходов вход/выход
	 * @return 
	 */
	public static SET_PORT B(byte PORT, byte DDR){
		return new SET_PORT(Settings.MODE.SET_PORTB, PORT, DDR);
	}
	/**Сохраняет данные по порту C
	 * @param PORT состояние выходов - вкл/выкл
	 * @param DDR направление выходов вход/выход
	 * @return 
	 */
	public static SET_PORT C(byte PORT, byte DDR){
		return new SET_PORT(Settings.MODE.SET_PORTC, PORT, DDR);
	}
	/**Сохраняет данные по порту D
	 * @param PORT состояние выходов - вкл/выкл
	 * @param DDR направление выходов вход/выход
	 * @return 
	 */
	public static SET_PORT D(byte PORT, byte DDR){
		return new SET_PORT(Settings.MODE.SET_PORTD, PORT, DDR);
	}
	/**Генератор состояний.
	 Прослойка, которая автоматом генерирует нужные данные на основании изменений входа*/
	public static class Generator {
		/**Все пины платы*/
		public static enum PIN{PIN0,PIN1,PIN2,PIN3,PIN4,PIN5,PIN6,PIN7,PIN8,PIN9,PIN10,PIN11,PIN12,PIN13,A0,A1,A2,A3,A4,A5,A6,A7;public static final PIN[]values = PIN.values();}
		public static enum PIN_MODE{INPUT,OUTPUT};
		public static enum PIN_STATE{HIGH,LOW};
		private class PORT {
			/**состояние выходов - вкл/выкл*/
			private byte PORT = 0;
			/**направление выходов вход/выход*/
			private byte DDR = 0;
			private void pinMode(int pin, PIN_MODE mode){
				switch (mode) {
					case OUTPUT -> DDR = (byte) (DDR | 1 << pin);
					case INPUT -> DDR = (byte) (DDR & ~(1 << pin));
					default -> throw new AssertionError();
				}
			}
			private void pinWrite(int pin, PIN_STATE state){
				switch (state) {
					case HIGH -> PORT = (byte) (PORT | 1 << pin);
					case LOW -> PORT = (byte) (PORT & ~(1 << pin));
					default -> throw new AssertionError();
				}
			}
		}
		
		public Generator(){}
		public SET_PORT pinMode(PIN pin, PIN_MODE mode){
			switch (pin) {
				case PIN0,PIN1,PIN2,PIN3,PIN4,PIN5,PIN6,PIN7 -> {
					D.pinMode(pin.ordinal() - PIN.PIN0.ordinal(), mode);
					return SET_PORT.D(D.PORT, D.DDR);
				}
				case PIN8,PIN9,PIN10,PIN11,PIN12,PIN13 -> {
					B.pinMode(pin.ordinal() - PIN.PIN8.ordinal(), mode);
					return SET_PORT.B(B.PORT, B.DDR);
				}
				case A0,A1,A2,A3,A4,A5,A6,A7 -> {
					C.pinMode(pin.ordinal() - PIN.A0.ordinal(), mode);
					return SET_PORT.C(C.PORT, C.DDR);
				}
				default -> throw new AssertionError();
			}
		}
		public SET_PORT pinWrite(PIN pin, PIN_STATE state){
			switch (pin) {
				case PIN0,PIN1,PIN2,PIN3,PIN4,PIN5,PIN6,PIN7 -> {
					D.pinWrite(pin.ordinal() - PIN.PIN0.ordinal(), state);
					return SET_PORT.D(D.PORT, D.DDR);
				}
				case PIN8,PIN9,PIN10,PIN11,PIN12,PIN13 -> {
					B.pinWrite(pin.ordinal() - PIN.PIN8.ordinal(), state);
					return SET_PORT.B(B.PORT, B.DDR);
				}
				case A0,A1,A2,A3,A4,A5,A6,A7 -> {
					C.pinWrite(pin.ordinal() - PIN.A0.ordinal(), state);
					return SET_PORT.C(C.PORT, C.DDR);
				}
				default -> throw new AssertionError();
			}

		}
		private PORT B = new PORT();
		private PORT C = new PORT();
		private PORT D = new PORT();
	}
}
