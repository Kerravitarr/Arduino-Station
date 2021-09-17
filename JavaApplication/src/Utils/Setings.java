package Utils;


import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.HashMap;

/* To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor. */
/**
 *
 * @author Terran
 */
public class Setings {
	private enum ErrSeting {
		NotKey
	}

	HashMap<String, String> hm;

	public Setings() {
		this.hm = new HashMap<>();
		File ConfigIliaProg = new File("ConfigIliaProg.txt");
		if (!ConfigIliaProg.isFile()) {
			try {
				new OutputStreamWriter(new FileOutputStream(ConfigIliaProg), "UTF-8").close();
			} catch (IOException e) {
				e.printStackTrace();
				//dispatchEvent(new EmusEvent(e.toString(), EmusEvent.Type.PRINTLN));
			}
		}
		try (BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(ConfigIliaProg), "UTF-8"))) {
			String line = reader.readLine();
			String[] keyVal;
			while (line != null) {
				keyVal = line.split(" = ");
				if(keyVal.length == 2)
					hm.put(keyVal[0], keyVal[1]);
				line = reader.readLine();
			}
		} catch (Exception ex) {
			ex.printStackTrace();
			//dispatchEvent(new EmusEvent(ex.toString(), EmusEvent.Type.PRINTLN));
		}
	}
// ConfigIliaProg.txt

	public int getInt(String key) {
		return Integer.parseInt(getString(key));
	}


	public int getInt(String key, int def) {
		try {
			return getInt(key);
		} catch (EnumConstantNotPresentException e) {
			return def;
		}
	}
	public String getString(String key) throws EnumConstantNotPresentException {
		if (!hm.containsKey(key)) { throw new EnumConstantNotPresentException(ErrSeting.NotKey.getDeclaringClass(), key); }
		return hm.get(key);
	}
	
	public String getString(String key, String def) {
		try {
			return getString(key);
		} catch (EnumConstantNotPresentException e) {
			return def;
		}
	}
	public String[] getStringMs(String key) throws EnumConstantNotPresentException {
		String ret = getString(key);
		return ret != null ? ret.split("<<!!!>>") : null;
	}

	public void set(String key, int val) {
		set(key, Integer.toString(val));
	}

	public void set(String key, String val) {
		hm.put(key, val);
		try (BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream("ConfigIliaProg.txt"), "UTF-8"))) {
			for (HashMap.Entry<String, String> entry : hm.entrySet()) {
				writer.write(entry.getKey() + " = " + entry.getValue() + "\n");
				writer.flush();
			}
		} catch (Exception ex) {
			ex.printStackTrace();
			//dispatchEvent(new EmusEvent(ex.toString(), EmusEvent.Type.PRINTLN));
		}
	}

	public void set(String key, String[] val) {
		String valS = null;
		for (String i : val) {
			if (valS == null) {
				valS = i;
			} else {
				valS += "<<!!!>>" + i;
			}
		}
		set(key, valS);
	}

	public void del(String key) {
		hm.remove(key);
		try (BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream("ConfigIliaProg.txt"), "UTF-8"))) {
			for (HashMap.Entry<String, String> entry : hm.entrySet()) {
				writer.write(entry.getKey() + " = " + entry.getValue() + "\n");
				writer.flush();
			}
		} catch (Exception ex) {
			ex.printStackTrace();
			//dispatchEvent(new EmusEvent(ex.toString(), EmusEvent.Type.PRINTLN));
		}
	}

	

}
