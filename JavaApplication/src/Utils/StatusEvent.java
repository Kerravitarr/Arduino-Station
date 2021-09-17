package Utils;

import java.util.EventObject;

public class StatusEvent extends EventObject {
	private String message;
	private Type _tp;
	private Exception exc = null;
	
	public static enum Type {
		PRINT,NON,PRINTLN, ERROR, FATAL_ERROR
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
