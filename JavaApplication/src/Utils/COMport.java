package Utils;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.TimerTask;
import java.util.concurrent.locks.LockSupport;

import javax.swing.Timer;

import com.fazecast.jSerialComm.SerialPort;
import com.fazecast.jSerialComm.SerialPortDataListener;
import com.fazecast.jSerialComm.SerialPortEvent;



public abstract class COMport {
	
	/**
	 *
	 */
	private static final long serialVersionUID = 1L;
	private SerialPort serialPortMinor;
	protected static Setings Se;
	private static int baud = 9600;
	
	private int Period;
	protected String name;
	BufferIn buffer;

	private List<StatusEventListener> listeners = new ArrayList<>();

/**
 * 
 * @param aThis
 * @param timeOut - сколько ожидаются байты из порта
 */
	public COMport(Setings aThis, String portName, int timeOut) {
		Se = aThis;
		Period = timeOut;
		name = portName;
		buffer = new BufferIn();
	}
	
	protected static String[] getComPortsName(Setings Se) {
		Set<String> ports = new HashSet<>();
		for(SerialPort sp : SerialPort.getCommPorts()) 
			ports.add(sp.getSystemPortName());
		
		try {
			for(String i : Se.getStringMs("PortList"))
				ports.add(i);
		} catch (EnumConstantNotPresentException e) {}
		
		List<String> sortedPorts = new ArrayList<String>(ports);
		Collections.sort(sortedPorts, (o1, o2) -> {
			if(o1.length() != o2.length()) return o1.length() - o2.length();
			else return o1.compareTo(o2);
		});
		
		return sortedPorts.toArray(new String[sortedPorts.size()]);
	}  
	
	public static String getLastPortName() {
		return Se.getString("LastPort");
	}
	
	public static void delMemoryPort(String name) {
		try {
			String [] ret = Se.getStringMs("PortList");
			if(ret.length == 1) {
				Se.del("PortList");
				return;
			}
			int index = 0;
			String [] newRet = new String[ret.length - 1];
			for(String i : ret) {
				if(i.equals(name)) continue;
				newRet[index++] = i;
				if(index == newRet.length - 1) return;
			}
			Se.set("PortList", newRet);
		} catch (EnumConstantNotPresentException e) {}
	}
	
	public static void addMemoryPort(String name) {
		try {
			String [] ret = Se.getStringMs("PortList");
			String [] newRet = new String[ret.length + 1];
			System.arraycopy(ret, 0, newRet, 0, ret.length);
			newRet[ret.length] = name;
			Se.set("PortList", newRet);
		} catch (EnumConstantNotPresentException e) {
			String [] newRet = new String[1];
			newRet[0] = name;
			Se.set("PortList", newRet);
		}
	}
	
	final public void close() {
		if(serialPortMinor == null) return; 
		try {
			serialPortMinor.closePort();
			serialPortMinor = null;
		} catch (Exception ex) {
			print("Ошибка закрытия порта" + ex.toString());
			dispatchEvent(new StatusEvent("Ошибка закрытия порта", ex, StatusEvent.Type.ERROR));
		}
	}
	
	//Именно тут выставляется таймаут компорта
	private class BufferIn implements ActionListener{
		Timer timer = new Timer(Period, this);
		@Override
		public void actionPerformed(ActionEvent e) {
			timer.stop();
			if(lenght == 0) return;
			
			String data = "";
			StringBuilder sb = new StringBuilder();
			
			for(int i = 0 ; i < lenght ; i++) {
				data += (char) msg[i];
				sb.append(String.format("%02X ", msg[i]));
			}
			lenght = 0;
			msgIn(data, sb.toString());
		}
		public void add(byte[] bytes) {
			if(!timer.isRunning()) LockSupport.parkNanos(1_000_000); //1мс
			timer.restart();
			if (msg.length <= lenght + bytes.length) {
				byte localM[] = new byte[lenght + bytes.length + 10];
				System.arraycopy(msg, 0, localM, 0, msg.length);
				msg = localM;
			}
			for(int i = 0 ; i < bytes.length; i++)
				msg[lenght++] = bytes[i];
		}
		
		byte[] msg = new byte[0];
		int lenght = 0;
		
	}
	
	protected final void init() {
		close();
		serialPortMinor = SerialPort.getCommPort(name);
		try {
			serialPortMinor.openPort();
			if (!serialPortMinor.isOpen()) {
				close();
				dispatchEvent(new StatusEvent("Не смогли открыть порт " + name, StatusEvent.Type.FATAL_ERROR));
				throw new Exception("Не смогли открыть порт");
			}
			serialPortMinor.setBaudRate(baud);
			Se.set("LastPort", name);
			serialPortMinor.addDataListener(new SerialPortDataListener() {
				public int getListeningEvents() {return SerialPort.LISTENING_EVENT_DATA_AVAILABLE;}
				public void serialEvent(SerialPortEvent event) {
					if (event.getEventType() != SerialPort.LISTENING_EVENT_DATA_AVAILABLE)
						return;
					byte[] newData = new byte[serialPortMinor.bytesAvailable()];
					serialPortMinor.readBytes(newData, newData.length);
					buffer.add(newData);
				}
			});
		} catch (Exception ex) {
			println("Возникла ошибка открытия порта, критично -> " + ex.toString());
			dispatchEvent(new StatusEvent("Возникла ошибка открытия порта " + name, ex, StatusEvent.Type.FATAL_ERROR));
		}
	}
	
	public void DTR() {
		if(serialPortMinor == null) return;
		serialPortMinor.clearDTR();
		Utils.pauseMs(Period);
		serialPortMinor.setDTR();
	}
	
	public void setBaud(int baud) {
		if(COMport.baud == baud) return;
		COMport.baud = baud;
		serialPortMinor.setBaudRate(baud);
		//close();
		//init();
	}

	protected abstract void msgIn(String data_S, String string_H);


	public final void print(String msg) {
		System.out.print(msg);
	}

	public final void println(String msg) {
		System.out.println(msg);
	}
	public final void println() {
		println("");
	}
	
	public boolean isActive() {
		return serialPortMinor != null;
	}

	final public boolean send(byte[] buffer) {
		try {
			return serialPortMinor.writeBytes(buffer, buffer.length) != -1;
		} catch (Exception ex) {
			dispatchEvent(new StatusEvent("Возникла ошибка -> ", ex, StatusEvent.Type.ERROR));
		}
		return false;
	}
	
	final public void addListener(StatusEventListener listener) {
		listeners.add(listener);
	}
	
	final protected void dispatchEvent(StatusEvent e) {
		for (StatusEventListener listener : listeners) {
			listener.statusEvent(e);
		}
	}
}


















