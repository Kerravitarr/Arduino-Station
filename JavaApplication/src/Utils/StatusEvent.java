package Utils;

import java.util.EventObject;

public class StatusEvent extends EventObject {
	/**Что за сообщение*/
	private String message;
	/**Его тип*/
	private Type _tp;
	/**Ошибка, если есть*/
	private Exception exc = null;
	/**Тип сообщений*/
	public enum Type {
		/**Напечатать сообщение в окошечке*/
		PRINT, 
		/**Пустота, ничего*/
		NON, 
		/**Напечатать сообщение с добавлением ентера*/
		PRINTLN, 
		/**Собщение ошибки*/
		ERROR, 
		/**Сообщение критической ошибки*/
		FATAL_ERROR
	}
	public StatusEvent(String message, Type tp) {
		super(message);
		this.message = message;
		_tp = tp;
	}
	public StatusEvent(String message, Exception e,Type tp) {
		super(message);
		this.message = message;
		_tp = tp;
		exc = e;
	}
	public String getMessage() {
		return message;
	}
	public Type get_Type() {
		return _tp;
	}

}
