import java.util.EventObject;

public class PortEvent extends EventObject{
	private byte[] message;
	private Type _tp;
	
	public static enum Type {
		/**Данные пришли*/
		DATA_IN,
		/**Надо передать данные*/
		DATA_OUT
	}
	public PortEvent(byte[] message, Type tp) {
		super(message);
		this.message = message;
		_tp = tp;
	}
	public byte[] getMessage() {
		return message;
	}
	public Type get_Type() {
		return _tp;
	}
}
