package Utils;

import java.awt.Point;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.locks.LockSupport;

public class Utils {
	public static abstract class Object {
		public abstract String toString(int i);
	}

	public enum OS {
		WINDOWS, LINUX, MAC, SOLARIS
	};// Operating systems.

	private static OS os = null;

	public static OS getOS() {
		if (os == null) {
			String operSys = System.getProperty("os.name").toLowerCase();
			if (operSys.contains("win")) {
				os = OS.WINDOWS;
			} else if (operSys.contains("nix") || operSys.contains("nux")
					|| operSys.contains("aix")) {
				os = OS.LINUX;
			} else if (operSys.contains("mac")) {
				os = OS.MAC;
			} else if (operSys.contains("sunos")) {
				os = OS.SOLARIS;
			}
		}
		return os;
	}
	public static void lineAveraging(ArrayList<Point> graph) {
		if(graph.size() < 3) return;
		Point A = graph.get(graph.size() - 1);
		Point B = graph.get(graph.size() - 2);
		Point C = graph.get(graph.size() - 3);
		if (Math.abs((A.x - B.x) * (A.y + B.y) + (B.x - C.x) * (B.y + C.y) + (C.x - A.x) * (C.y + A.y)) <= 0) {
			graph.remove(graph.size() - 2);
		}
	}
	
	/**
	 * Функция паузы, нужна для отрисовки
	 */
	public static void pause(boolean isRepaint) {
		if (Utils.getOS() == Utils.OS.WINDOWS) {
			if (isRepaint) {
				LockSupport.parkNanos(1 * 1_000_000);
			} else {
				// Только для отладки
				LockSupport.parkNanos(1 * 1_000_000);
			}
		} else {
			if (isRepaint) {
				try {
					Thread.sleep(1);
				} catch (InterruptedException e) {
				}
			} else {
				LockSupport.parkNanos(200000);
			}
		}
	}
	/**
	 * Функция паузы, нужна для отрисовки
	 */
	static public void pause(long sec) {
		pauseMs(sec * 1_000);
	}
	static public void pauseMs(long ms) {
		LockSupport.parkNanos(ms * 1_000_000);
	}
	/**
	 * Функция ищет номер объекта в массиве объектов
	 * @param val - искомое значение
	 * @param mass - массив поиска
	 * @return Индекс элемента или вызывется ошибка RuntimeException
	 */
	public static int getNum(java.lang.Object val, java.lang.Object[] mass) {
		if(val != null)
			for (int i = 0; i < mass.length; i++)
				if (val.equals(mass[i]))
					return i;
		throw new RuntimeException("Ошибка в поиске истины");
	}
	
	public static java.lang.Object[] hashMapToArray(HashMap<?, ?> in) {
		java.lang.Object[] result = new Object[in.size()];
		int count = 0;
		for (var i : in.values())
			result[count++] = i;
		return result;
	}
	
	// Преобразует число в строку с определённым числом нулей
	public static String toStrWithZero(int num, int zer) {
		String out = "";
		for (int i = 0; i < zer; i++) {
			out = num % 10 + out;
			num = num / 10;
		}
		return out;
	}

	public static String toStrWithZero(String num, int zer) {
		String out = "";
		if (num.length() > zer) {
			num = num.substring(num.length() - zer);
		}
		for (int i = 0; i < zer; i++) {
			if (num.length() > i) {
				out = out + num.substring(i, i + 1);
			} else {
				out = "0" + out;
			}
		}
		return out;
	}

	public static String toHexStrWithZero(int num, int zer) {
		return toStrWithZero(Integer.toHexString(num).toUpperCase(), zer);
	}

	// Преобразует число в строку с определённым числом нулей
	public static String toBeautifulStr(long num) {
		return toBeautifulStr(num,(int)Math.ceil(Math.log10(num)));
	}

	public static String toBeautifulStr(long num, int zer) {
		String out = "";
		for (int i = 0; zer > 0; i++, zer--) {
			if (i % 3 == 0 && i > 0)
				out = " " + out;
			out = num % 10 + out;
			num = num / 10;
		}
		return out;
	}
	
	public static String to_string(byte array_hex[]) {
		String data = "";
		for (byte element : array_hex) {
			data += String.format("%02X ", element);
		}
		return data;
	}

	/**
	 * Преобразует строку формата HH HH HH в массив байт
	 * @param string_H - строка с данными
	 * @return - готовый массив
	 */
	static public byte[] to_hex(String string_H) {
		String bytes[] = string_H.split(" ");
		byte[] array_h = new byte[bytes.length];
		for (int i = 0; i < bytes.length; i++)
			array_h[i] = (byte) Integer.parseInt(bytes[i], 16);
		return array_h;
	}
	/**
	 * Преобразует строку формата SSSSS в массив байт
	 * @param string_S - строка с данными
	 * @return - готовый массив
	 */
	static public byte[] Ascii_to_hex(String string_S) {
		String bytes[] = string_S.split("");
		byte[] array_h = new byte[bytes.length / 2];
		for (int i = 0; i < bytes.length/ 2; i++)
			array_h[i] = (byte) (Integer.parseInt(bytes[i*2 + 0] + bytes[i*2 + 1], 16));
		return array_h;
	}
}
