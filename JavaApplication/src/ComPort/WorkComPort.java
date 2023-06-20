/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package ComPort;

import Utils.Utils;
import start.PortEventListener;

/**Текущий рабочий компорт
 * @author Илья
 */
public class WorkComPort {
	private RealCOM port = null;
	/**Занят текущий порт или нет?*/
	Object bisy = null;
	
	/**Занять текущий порт*/
	public void lock(Object who){
		while(bisy != null && bisy != who)
			Utils.pauseMs(10);
		bisy = who;
	}
	/**Занять текущий порт*/
	public void unlock(Object who){
		if(who == bisy)
			bisy = null;
		else
			throw new IllegalArgumentException("Попытался разблокировать порт не тот, кто его взял!!!");
	}

	public Request.Answer write(Request req) {
		return req.answer(port.send(req.getMsg(), req.lenghtAns, req.timeout));
	}

	public void set(RealCOM event) {
		port = event;
	}
}
