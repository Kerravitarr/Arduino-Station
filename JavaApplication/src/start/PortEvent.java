package start;

import java.util.ArrayList;
import java.util.EventObject;

import Utils.Utils;

public class PortEvent extends EventObject{
	private byte[] message;
	private Type _tp;
	
	public static enum Type {
		/**Данные пришли*/
		DATA_IN,
		/**Надо передать данные*/
		DATA_OUT,
		/**Настройка порта, сохранение длинны сообщения*/
		SET_LENGHT
	}
	public PortEvent(byte[] message, Type tp) {
		super(message);
		this.message = message;
		_tp = tp;
	}
	public PortEvent(byte message, Type tp) {
		super(message);
		this.message = new byte[1];
		this.message[0] = message;
		_tp = tp;
	}
	/**
	 * Отправляет сообщение в порт
	 * @param msg - данные, которые надо передать
	 * @param dataOut	- тип сообщения
	 */
	public PortEvent(ArrayList<Byte> message, PortEvent.Type tp) {
		super(message);
		_tp = tp;
		this.message = new byte[message.size()];
		for(var i = 0 ; i < message.size() ; i++)
			this.message[i] = message.get(i);
	}
	public byte[] getMessage() {
		return message;
	}
	public Type get_Type() {
		return _tp;
	}
	
	public String toString() {
		return _tp.name() + " -> " + Utils.to_string(message);
	}
}
