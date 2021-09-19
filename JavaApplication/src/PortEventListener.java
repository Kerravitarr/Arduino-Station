

public interface PortEventListener {
	final byte STK_OK = 0x10;

	public void portData(PortEvent e);
	
	/**
	 * Попытка перехватить все сообщения, поступающие от ардуино
	 * @param l - Заватчик, тот, кто будет обрабатывать сообщения
	 * @return true - захват удался, теперь все сообщения будут попадать сразу сюда
	 */
	public boolean capture(PortEventListener l);
	/**
	 * Освобождение, теперь все сообщения будут проходить мимо
	 * @return
	 */
	public void liberation();
}
