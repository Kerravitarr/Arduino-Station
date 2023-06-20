/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package ComPort;

/**
 *Запросы на ответ
 * @author Илья
 */
public class EasyReq {
	public static class Ping extends Request{
		public Ping() {
			super(Settings.MODE.GET_STATUS);
		}		
	}
}
