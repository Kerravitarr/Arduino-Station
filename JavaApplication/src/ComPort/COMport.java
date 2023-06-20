package ComPort;

import Utils.Setings;
import Utils.StatusEvent;
import Utils.StatusEventListener;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.locks.LockSupport;

import javax.swing.Timer;

import com.fazecast.jSerialComm.SerialPort;


/**
 * Обобщающий любой ком порт класс
 * @author Илья
 */
public abstract class COMport {
	/**Слушатель данных из UART*/
	private class PortListener extends Thread {
		boolean isStart = false;
		
		public void run() {
			while (true) {
				try {
					while (!isStart)
						Thread.sleep(1);
					while (true) {
						byte[] readBuffer = new byte[1024];
						int numRead = serialPortMinor.readBytes(readBuffer, readBuffer.length);
						buffer.add(readBuffer, numRead);
					}
				} catch (InterruptedException | java.lang.NullPointerException e) {
					isStart = false;
				}
			}
		}
		
		public void restart() {
			isStart = true;
		}
	}
	private SerialPort serialPortMinor;
	private static int baud = 9600;
	private PortListener listener = new PortListener();
	
	private int Period;
	/** Длина сообщения, которое ожидаем получить из порта */
	private int LenghtMsgIn = -1;
	protected String name;
	BufferIn buffer;

	private List<StatusEventListener> listeners = new ArrayList<>();

/**
 * @param portName - имя этого порта
 * @param timeOut - сколько ожидаются байты из порта
 */
	public COMport(String portName, int timeOut) {
		Period = timeOut;
		name = portName;
		buffer = new BufferIn();
		listener.start();
	}
	
	protected static String[] getComPortsName() {
		Set<String> ports = new HashSet<>();
		for(SerialPort sp : SerialPort.getCommPorts()) 
			ports.add(sp.getSystemPortName());
		try {
			for(String i : Utils.Constants.Settings.getStringMs("PortList"))
				ports.add(i);
		} catch (EnumConstantNotPresentException e) {}
		
		List<String> sortedPorts = new ArrayList<String>(ports);
		Collections.sort(sortedPorts, (o1, o2) -> {
			if(o1.length() != o2.length()) return o1.length() - o2.length();
			else return o1.compareTo(o2);
		});
		
		return sortedPorts.toArray(new String[sortedPorts.size()]);
	}  
	/**Возвращает имя последнего использующегося порта
	 * @return 
	 */
	public static String getLastPortName() {
		return Utils.Constants.Settings.getString("LastPort");
	}
	/**Удаляет порт из памяти портов
	 * @param name имя порта
	 */
	public static void delMemoryPort(String name) {
		try {
			String [] ret = Utils.Constants.Settings.getStringMs("PortList");
			if(ret.length == 1) {
				Utils.Constants.Settings.del("PortList");
				return;
			}
			int index = 0;
			String [] newRet = new String[ret.length - 1];
			for(String i : ret) {
				if(i.equals(name)) continue;
				newRet[index++] = i;
				if(index == newRet.length - 1) return;
			}
			Utils.Constants.Settings.set("PortList", newRet);
		} catch (EnumConstantNotPresentException e) {}
	}
	/**Добавляет порт, даже если его не существует, ко всем известным портам
	 * @param name имя порта
	 */
	public static void addMemoryPort(String name) {
		try {
			String [] ret = Utils.Constants.Settings.getStringMs("PortList");
			String [] newRet = new String[ret.length + 1];
			System.arraycopy(ret, 0, newRet, 0, ret.length);
			newRet[ret.length] = name;
			Utils.Constants.Settings.set("PortList", newRet);
		} catch (EnumConstantNotPresentException e) {
			String [] newRet = new String[1];
			newRet[0] = name;
			Utils.Constants.Settings.set("PortList", newRet);
		}
	}
	/**Закрывает порт*/
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
			println("Read from comport: " + sb.toString());
			msgIn(data, sb.toString());
		}
		public void add(byte[] bytes, int lenghtBt) {
			if(!timer.isRunning()) LockSupport.parkNanos(1_000_000); //1мс
			timer.restart();
			if (msg.length <= lenght + lenghtBt) {
				byte localM[] = new byte[lenght + lenghtBt + 10];
				System.arraycopy(msg, 0, localM, 0, msg.length);
				msg = localM;
			}
			for(int i = 0 ; i < lenghtBt; i++)
				msg[lenght++] = bytes[i];
			
			if(LenghtMsgIn != -1 && LenghtMsgIn <= lenght)
				actionPerformed(null);
		}
		
		public void add(byte[] bytes) {
			add(bytes, bytes.length);
		}
		
		byte[] msg = new byte[0];
		int lenght = 0;
		
	}
	/**Инициализирует порт*/
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
			serialPortMinor.setComPortTimeouts(SerialPort.TIMEOUT_READ_SEMI_BLOCKING, 0, 0);
			Utils.Constants.Settings.set("LastPort", name);

			listener.restart();
		} catch (Exception ex) {
			println("Возникла ошибка открытия порта, критично -> " + ex.toString());
			dispatchEvent(new StatusEvent("Возникла ошибка открытия порта " + name, ex, StatusEvent.Type.FATAL_ERROR));
		}
	}
	/**Посылает сигнал перезагрузки через порт*/
	public void DTR() {
		if(serialPortMinor == null) return;
		serialPortMinor.clearDTR();
		Utils.Utils.pauseMs(Period);
		serialPortMinor.setDTR();
	}
	/**Устанавливает скорость работы порта*/
	public void setBaud(int baud) {
		if(COMport.baud == baud) return;
		COMport.baud = baud;
		serialPortMinor.setBaudRate(baud);
		//close();
		//init();
	}
	/**Функция вызывается, когда из порта придут данные
	 * @param data_S данные в формате ASIIC
	 * @param string_H Данные в формате HEX
	 */
	protected abstract void msgIn(String data_S, String string_H);
	
	private void print(String msg) {
		System.out.print(msg);
	}

	private void println(String msg) {
		System.out.println(msg);
	}
	private void println() {
		println("");
	}
	/**Проверяет - работает порт или нет
	 * @return true, если порт работает
	 */
	public boolean isActive() {
		return serialPortMinor != null;
	}
	/**Передаёт данные через порт
	 * @param buffer данные, которые надо передать
	 * @return true, если данные ушли и всё хорошо
	 */
	public boolean send(byte[] buffer) {
		try {
			print("Send to comport: ");
			for(var i : buffer) {
				System.out.print(" 0x"+Integer.toHexString(0xFF&i));
			}
			System.out.println();
			return serialPortMinor.writeBytes(buffer, buffer.length) != -1;
		} catch (Exception ex) {
			dispatchEvent(new StatusEvent("Возникла ошибка -> ", ex, StatusEvent.Type.ERROR));
		}
		return false;
	}
	/**Устанавливает количество байт, через которые закончится приём
	 * @param lenght количество ожидаемых байт
	 */
	final public void setLenght(byte lenght) {
		LenghtMsgIn = lenght;
	}
	
	final public void addListener(StatusEventListener listener) {
		listeners.add(listener);
	}
	/**Всё для печати ошибок*/
	final protected void dispatchEvent(StatusEvent e) {
		for (StatusEventListener listener : listeners) {
			listener.statusEvent(e);
		}
	}
}


















