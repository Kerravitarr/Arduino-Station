import java.awt.Container;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import java.util.Set;

import Utils.COMport;
import Utils.Setings;

public class ComPorts extends COMport {
	static final int timeOut = 2; 
	static HashMap<String, ComPorts> allPorts = new HashMap<>();
	
	private List<PortEventListener> listeners = new ArrayList<>();

	private ComPorts(Setings aThis, String name) {
		super(aThis, name, timeOut);
	}

	@Override
	protected void msgIn(String data_S, String string_H) {
		dispatchEvent(new PortEvent(Utils.Utils.to_hex(string_H), PortEvent.Type.DATA_IN));
	}
	
	public static Collection<ComPorts> getComPorts(Setings aThis) {
		String[] names = getComPortsName(aThis);
		Map<String, ComPorts> local = new HashMap<>();
		for(int i = 0; i < names.length; i++) {
			local.put(names[i], new ComPorts(aThis, names[i]));
		}
		
		if(!local.keySet().equals(allPorts.keySet())) {
			//Порты, которых больше не существует
			Set<String> mapSet = new HashSet<String>(allPorts.keySet());
			Set<String> localSet = new HashSet<String>(local.keySet());
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
		
		List<ComPorts> ret = new ArrayList<>(allPorts.values());
		Collections.sort(ret, (o1, o2) -> {
			if(o1.toString().length() != o2.toString().length()) return o1.toString().length() - o2.toString().length();
			else return o1.toString().compareTo(o2.toString());
		});
		
		return ret;
	}
	
	public String toString() {
		return name;
	}
	
	public void start() {
		if(isActive()) return;
		for(ComPorts i : allPorts.values()) {
			i.close();
		}
		init();
	}

	final public void addListener(PortEventListener listener) {
		listeners.add(listener);
	}
	
	final protected void dispatchEvent(PortEvent e) {
		for (PortEventListener listener : listeners) {
			listener.portData(e);
		}
	}
}
