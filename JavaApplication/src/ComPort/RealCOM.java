package ComPort;

import Utils.Utils;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;

import start.PortEvent;
import start.PortEventListener;

/**Реальный компорт дуни
 * @author Илья
 */
public class RealCOM extends COMport {
	//Сколько ждать данные из порта
	static final int timeOut = 2; 
	static HashMap<String, RealCOM> allPorts = new HashMap<>();
	
	private List<PortEventListener> listeners = new ArrayList<>();
	
	public class Data{public final String data_S;public final String string_H; private Data(String s, String h){data_S=s; string_H=h;}}
	/**Текущие данные на приём*/
	private Data data;
	
	private RealCOM(String name) {
		super(name, timeOut);
	}

	@Override
	protected void msgIn(String data_S, String string_H) {
		data = new Data(data_S, string_H);
	}
	
	public static Collection<RealCOM> getComPorts() {
		String[] names = COMport.getComPortsName();
		Map<String, RealCOM> local = new HashMap<>();
		for (String name1 : names) {
			local.put(name1, new RealCOM(name1));
		}
		
		if(!local.keySet().equals(allPorts.keySet())) {
			//Порты, которых больше не существует
			var mapSet = new HashSet<>(allPorts.keySet());
			var localSet = new HashSet<>(local.keySet());
			mapSet.removeAll(localSet);
			for(String i : mapSet) {
				allPorts.get(i).close();
				allPorts.remove(i);
			}
			//Порты, которых раньше не было, а теперь есть
			mapSet = new HashSet<String>(allPorts.keySet());
			localSet = new HashSet<String>(local.keySet());
			localSet.removeAll(mapSet);
			for(String i : localSet)
				allPorts.put(i, local.get(i));
		}
		
		List<RealCOM> ret = new ArrayList<>(allPorts.values());
		Collections.sort(ret, (o1, o2) -> {
			if(o1.toString().length() != o2.toString().length()) return o1.toString().length() - o2.toString().length();
			else return o1.toString().compareTo(o2.toString());
		});
		
		return ret;
	}
	/**Имя порта*/
	public String toString() {
		return name;
	}
	/**Запустить порт*/
	public void start() {
		if(isActive()) return;
		for(RealCOM i : allPorts.values()) {
			i.close();
		}
		init();
	}
	/**Отправляет данные в порт и возващает ответ
	 * @param buffer данные для отправки
	 * @param lenght сколько данных ожидаем обратно
	 * @param timeuot сколько ждём данные по времени, мс
	 * @return пришедшие данные или null
	 */
	public Data send(byte[] buffer, int lenght, int timeuot) {
		setLenght((byte) lenght);
		var d = new java.util.Date();
		data = null;
		super.send(buffer);
		while(data == null && (new java.util.Date().getTime() - d.getTime() < timeuot))
			Utils.pauseMs(10);
		return data;
	}
}
