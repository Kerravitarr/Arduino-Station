/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package ComPort;

import ComPort.Settings.ANSWER;
import java.util.Arrays;

/**
 *Общий класс запроса к плате
 * @author Илья
 */
public class Request {
	public final class Answer{
		public final Settings.ANSWER answer;
		private Answer(Settings.ANSWER a){
			answer = a;
		}
	}
	/**Сообщение*/
	private final byte[] message;
	/**Длина ответа*/
	public final byte lenghtAns;
	/**Ожидание ответа, мс*/
	public final int timeout;
	/**Создаёт запрос к плате
	 * @param mode режим, который спрашиваем
	 */
	protected Request(Settings.MODE mode){
		this(mode, new byte[0]);
	}
	/**Создаёт запрос к плате
	 * @param mode запрос, который нас интересует
	 * @param data данные, что мы послыаем
	 */
	protected Request(Settings.MODE mode, byte[] data){
		this(mode, data, (byte)1, 100);
	}
	/**Создаёт запрос к плате
	 * @param mode запрос, который нас интересует
	 * @param data данные, что мы послыаем
	 */
	protected Request(Settings.MODE mode, byte[] data, byte l, int to){
		message = new byte[1 + 1 + data.length]; //байт на заголовок, байт на режим и данные
		message[0] = 0x00; //Означате, что это мой режим
		message[1] = mode.CMD; //Означате, что это мой режим
		for (int i = 0; i < data.length; i++) {
			message[2 + i] = data[i];
		}
		lenghtAns = l;
		timeout = to;
	}
	public byte[] getMsg(){return message;}
	
	public Answer answer(RealCOM.Data data) {
		if (data == null) {
			return new Answer(Settings.ANSWER.NOT_ANS);
		} else {
			var split = data.string_H.split(" ");
			var mass = new byte[split.length - 1];
			for (int i = 1; i < split.length; i++) {
				mass[i-1] = Integer.decode("0x"+split[i]).byteValue();
			}
			return generateAnswer(Settings.ANSWER.toAns(Integer.decode("0x"+split[0]).byteValue()), mass);
		}
	}

	protected Answer generateAnswer(ANSWER ans, byte[] data) {
		return new Answer(ans);
	}
	
}
