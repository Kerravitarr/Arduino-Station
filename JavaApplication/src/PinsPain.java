import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.FlowLayout;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.ArrayList;
import java.util.List;
import java.util.TimerTask;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextPane;
import javax.swing.SwingConstants;
import javax.swing.border.EmptyBorder;
import javax.swing.border.EtchedBorder;
import javax.swing.border.TitledBorder;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;
import javax.swing.text.StyledDocument;

import Utils.StatusEvent;
import Utils.StatusEventListener;
import Utils.Timer;
import Utils.Utils;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

public class PinsPain extends JDialog implements PortEventListener{
	
	private static class BYTE_3 {
		private byte B = 0x00;
		private byte C = 0x00;
		private byte D = 0x00;
		
		BYTE_3(byte B,byte C,byte D){
			this.B=B;
			this.C=C;
			this.D=D;
		}
		
		public byte getState(char p, byte pin) {
			if(pin < 0 && pin > 7) return 0x00;
			switch (p) {
			case 'B':return (byte) ((B & (1<<pin)) == 0 ? 0 : 1);
			case 'C':return (byte) ((C & (1<<pin)) == 0 ? 0 : 1);
			case 'D':return (byte) ((D & (1<<pin)) == 0 ? 0 : 1);
			default:return 0x00;
			}
		}
		
		public void setState(char p, byte pin, byte state) {
			if(pin < 0 && pin > 7) return;
			switch (p) {
			case 'B' -> B = (byte) ((B & ~(1<<pin)) | (state<<pin));
			case 'C' -> C = (byte) ((C & ~(1<<pin)) | (state<<pin));
			case 'D' -> D = (byte) ((D & ~(1<<pin)) | (state<<pin));
			}
		}
		
		public byte get(char p) {
			switch (p) {
			case 'B':return B;
			case 'C':return C;
			case 'D':return D;
			default:return 0x00;
			}
		}
		
		public void update(char p, BYTE_3 byte_3) {
			switch (p) {
			case 'B':B = byte_3.get(p);
			case 'C':C = byte_3.get(p);
			case 'D':D = byte_3.get(p);
			}
		}
	}
	
	
	/**
	 * ?????????????????? ???????????????????? ??????????????. ???? ???????? ?????????????????????? ???????????? ?????????? ?? 0 ?????? +5
	 * ???????? ???????? ???? ???????? - ?????????? ?????? ?? +5 ?????? ?????????????????? ???????????? ?? ??????????????
	 * @author Terran
	 *
	 */
	private static class PORT extends BYTE_3 {		
		PORT(byte PORTB,byte PORTC,byte PORTD){
			super(PORTB, PORTC, PORTD);
		}
		/**
		 * 
		 * @param p
		 * @param pin
		 * @return 1 - ???????? ???????????????????? ?? +5?? (?????? ?????????????????? ?? ??????)
		 */
		public byte getState(char p, byte pin) {
			return super.getState(p, pin);
		}
		
		public String toString() {
			String ret = "???????? ??: ";
			for(byte i = 0 ; i < 8 ; i++)
				ret += (i == 0 ? "" : ",") + "???????? "+ i + " ???????????????????????? ?? " + (getState('B',i) == 0 ? "0??" : "+5B") + " ";
			ret += ". ???????? ??: ";
			for(byte i = 0 ; i < 8 ; i++)
				ret += (i == 0 ? "" : ",") + "???????? "+ i + " ???????????????????????? ?? " + (getState('C',i) == 0 ? "0??" : "+5B") + " ";
			ret += ". ???????? D: ";
			for(byte i = 0 ; i < 8 ; i++)
				ret += (i == 0 ? "" : ",") + "???????? "+ i + " ???????????????????????? ?? " + (getState('D',i) == 0 ? "0??" : "+5B") + " ";
			ret += ".";
			return ret;
		}
	}

	/**
	 * ?????????????????? ???????????????????????? ???????????? ??????????
	 * ???????? ?????? 0, ???? ???????? ???????????????? ???? ????????
	 * @author Terran
	 *
	 */
	private static class DDR extends BYTE_3{
		
		DDR(byte B,byte C,byte D){
			super(B,C,D);
		}
		/**
		 * 
		 * @param p
		 * @param pin
		 * @return 1 - ???????? ???????????????????????? ???? ??????????, 0 - ???? ????????
		 */
		public byte getState(char p, byte pin) {
			return super.getState(p, pin);
		}
		
		public String toString() {
			String ret = "???????? ??: ";
			for(byte i = 0 ; i < 8 ; i++)
				ret += (i == 0 ? "" : ",") + "???????? "+ i + " ???????????????? " + (getState('B',i) == 0 ? "????????????" : "??????????????") + " ";
			ret += ". ???????? ??: ";
			for(byte i = 0 ; i < 8 ; i++)
				ret += (i == 0 ? "" : ",") + "???????? "+ i + " ???????????????? " + (getState('C',i) == 0 ? "????????????" : "??????????????") + " ";
			ret += ". ???????? D: ";
			for(byte i = 0 ; i < 8 ; i++)
				ret += (i == 0 ? "" : ",") + "???????? "+ i + " ???????????????? " + (getState('D',i) == 0 ? "????????????" : "??????????????") + " ";
			ret += ".";
			return ret;
		}
	}
	
	/**
	 * ???????????????? ???????????????????? ?????????????? ???? ????????
	 * 1 - ???? ???????? +5??
	 * @author Terran
	 *
	 */
	private static class PIN  extends BYTE_3{
		
		PIN(byte B,byte C,byte D){
			super(B,C,D);
		}
		/**
		 * 
		 * @param p
		 * @param pin
		 * @return 1 - ???? ???????? +5??, 0 - 0??
		 */
		public byte getState(char p, byte pin) {
			return super.getState(p, pin);
		}
		
		public String toString() {
			String ret = "???????? ??: ";
			for(byte i = 0 ; i < 8 ; i++)
				ret += (i == 0 ? "" : ",") + "???? ???????? "+ i + " " + (getState('B',i) == 0 ? "0??" : "+5B") + " ";
			ret += ". ???????? ??: ";
			for(byte i = 0 ; i < 8 ; i++)
				ret += (i == 0 ? "" : ",") + "???? ???????? "+ i +" " +  (getState('C',i) == 0 ? "0??" : "+5B") + " ";
			ret += ". ???????? D: ";
			for(byte i = 0 ; i < 8 ; i++)
				ret += (i == 0 ? "" : ",") + "???? ???????? "+ i +" " +  (getState('D',i) == 0 ? "0??" : "+5B") + " ";
			ret += ".";
			return ret;
		}
	}
	
	
	private static class IOstate extends JLabel{
		private char port;
		private byte pin;
		private PinStatus pinStatus = null;
		private RzStatus rzStatus = null;
		public IOstate(char port, byte pin,PinStatus pinStatus,RzStatus rzStatus){
			super();
			this.port=port;
			this.pin=pin;
			this.pinStatus=pinStatus;
			this.rzStatus=rzStatus;
			update();
			setHorizontalAlignment(SwingConstants.CENTER);
			
			addMouseListener(new MouseAdapter() {public void mouseClicked(MouseEvent e) {
				nowDDR.setState(port, pin,(byte) (nowDDR.getState(port, pin) == 0 ? 1 : 0));
				update();
			}});
		}
		
		public void update() {
			if (nowDDR.getState(port, pin) == oldDDR.getState(port, pin)) {
				setForeground(Color.BLACK);
				SetState(oldDDR);
			} else {
				setForeground(Color.BLUE);
				SetState(nowDDR);
			}
		}
		
		public void SetState(DDR ddr) {
			pinStatus.update();
			rzStatus.update();
			if (ddr.getState(port, pin) == 0) {
				setText("???");
				setToolTipText("????????");
			} else {
				setText("???");
				setToolTipText("??????????");
			}
		}
	}
	
	private static class PinStatus extends JTextPane{
		private char port;
		private byte pin;
		public PinStatus(char port, byte pin){
			super();
			this.port=port;
			this.pin=pin;
			update();
			setEnabled(false);
			StyledDocument doc = getStyledDocument();
			SimpleAttributeSet center = new SimpleAttributeSet();
			StyleConstants.setAlignment(center, StyleConstants.ALIGN_CENTER);
			doc.setParagraphAttributes(0, doc.getLength(), center, false);
			
			addMouseListener(new MouseAdapter() {public void mouseClicked(MouseEvent e) {
				if(nowDDR.getState(port, pin) != 0)
					nowPORT.setState(port, pin,(byte) (nowPORT.getState(port, pin) == 0 ? 1 : 0));
				update();
			}});
		}
		
		public void update() {
			if (nowDDR.getState(port, pin) == oldDDR.getState(port, pin)
					&& nowPORT.getState(port, pin) == oldPORT.getState(port, pin))
				SetState(oldDDR, oldPIN, oldPORT, true);
			else {
				setBackground(Color.BLUE);

				SetState(nowDDR, nowPIN, nowPORT, false);
			}
		}
		
		public void SetState(DDR ddr, PIN pin,PORT port, boolean updateColor) {
			if (ddr.getState(this.port, this.pin) == 1 && port.getState(this.port, this.pin) == 0
					|| ddr.getState(this.port, this.pin) == 0 && pin.getState(this.port, this.pin) == 0) {
				setText("-");
				if(updateColor)
				setBackground(Color.BLACK);
				super.setDisabledTextColor(Color.WHITE);
				setToolTipText("???? ???????? 0 ??");
			} else {
				setText("+");
				if(updateColor)
				setBackground(Color.RED);
				super.setDisabledTextColor(Color.GREEN);
				setToolTipText("???? ???????? +5 ??");
			}
		}
	}
	
	private static class RzStatus extends JTextPane{
		private char port;
		private byte pin;
		public RzStatus(char port, byte pin){
			super();
			this.port=port;
			this.pin=pin;
			update();
			setEnabled(false);
			StyledDocument doc = getStyledDocument();
			SimpleAttributeSet center = new SimpleAttributeSet();
			StyleConstants.setAlignment(center, StyleConstants.ALIGN_CENTER);
			doc.setParagraphAttributes(0, doc.getLength(), center, false);
			super.setDisabledTextColor(Color.GREEN);
			
			addMouseListener(new MouseAdapter() {public void mouseClicked(MouseEvent e) {
				if(nowDDR.getState(port, pin) != 1)
					nowPORT.setState(port, pin,(byte) (nowPORT.getState(port, pin) == 0 ? 1 : 0));
				update();
			}});
		}
				
		public void update(){
			if (nowDDR.getState(port, pin) == oldDDR.getState(port, pin)
					&& nowPORT.getState(port, pin) == oldPORT.getState(port, pin)) {
				setBackground(Color.GRAY);
				SetState(oldDDR, oldPORT);
			} else {
				setBackground(Color.BLUE);
				SetState(nowDDR, nowPORT);
			}
		}
		
		public void SetState(DDR ddr, PORT port) {
			if (ddr.getState(this.port, this.pin) == 0 && port.getState(this.port, this.pin) == 1) {
				setText("???");
				setToolTipText("?????????????????? ?????????????????????????? ????????????????");
			} else {
				setText("_");
				setToolTipText("???????????????? ???? ??????????????????");
			}
		}
	}
	
	private static class PINpanel extends JPanel {
		private char port;
		private byte pin;
		
		private IOstate state;
		private PinStatus status;
		private RzStatus rz;
		PINpanel(char port, byte pin){
			super();
			this.pin=pin;
			this.port=port;
			setBorder(new TitledBorder(null, pin+"", TitledBorder.CENTER, TitledBorder.TOP, null, null));
			setLayout(new BorderLayout(0, 0));
			
			status = new PinStatus(port,pin);
			add(status, BorderLayout.CENTER);
			
			rz = new RzStatus(port,pin);
			add(rz, BorderLayout.SOUTH);
			
			state = new IOstate(port,pin,status, rz);
			add(state, BorderLayout.NORTH);
			
		}
		
		public void update() {
			state.update();
			status.update();
			rz.update();
		}
		
		public String toString() {
			return "?????? " + pin + " ?????????? " + port;
		}
	}
	
	private static class PORTPanel extends JPanel {
		private char port;
		private byte maxPins;
		ArrayList<PINpanel> pins = new ArrayList<>();
		PORTPanel(char port, byte maxPins){
			super();
			this.maxPins=maxPins;
			this.port=port;
			
			setBorder(new TitledBorder(
					new EtchedBorder(EtchedBorder.LOWERED, new Color(255, 255, 255), new Color(160, 160, 160)), "PORT" + port,
					TitledBorder.CENTER, TitledBorder.TOP, null, new Color(0, 0, 0)));
			setLayout(new FlowLayout(FlowLayout.CENTER, 5, 5));
			
			for(byte i = maxPins; i > 0 ; i--){
				PINpanel PIN0 = new PINpanel(port,(byte) (i-1));
				pins.add(PIN0);
				add(PIN0);
			}
		}
		
		public void update() {
			for(PINpanel i : pins) {
				i.update();
			}
		}
		
		public String toString() {
			return "???????? " + port;
		}
	}
	
	enum Mode{
		None,Syn, Update
	}

	private final JPanel contentPanel = new JPanel();
	private PORTPanel PORTB;
	private PORTPanel PORTC;
	private PORTPanel PORTD;
	
	private static PORT nowPORT = new PORT((byte)0,(byte)0,(byte)0);
	private static PORT oldPORT = new PORT((byte)0,(byte)0,(byte)0);
	private static DDR nowDDR = new DDR((byte)0,(byte)0,(byte)0);
	private static DDR oldDDR = new DDR((byte)0,(byte)0,(byte)0);
	private static PIN nowPIN = new PIN((byte)0,(byte)0,(byte)0);
	private static PIN oldPIN = new PIN((byte)0,(byte)0,(byte)0);
	private JPanel panel_1;
	private JButton btnNewButton;
	private JPanel panel_2;
	private JPanel panel_3;
	private JCheckBox autoCyn;
	private JButton btnNewButton_1;
	private PortEventListener listener = null;
	private List<StatusEventListener> listeners = new ArrayList<>();
	private Mode mode = Mode.None;

	private final byte[] REQ_GETSTATUS = { 0x00, 0x08 };

	
	/**
	 * Create the dialog.
	 */
	public PinsPain() {
		addWindowListener(new WindowAdapter() {
			public void windowOpened(WindowEvent e) {
				capture();
				mode = Mode.Syn;
				dispatchEvent(new PortEvent(REQ_GETSTATUS, PortEvent.Type.DATA_OUT));
			}
			public void windowDeactivated(WindowEvent e) {
				listener.liberation();
				autoCyn.setSelected(false);
				mode = Mode.None;
			}
		});

        setAlwaysOnTop(true);
		setResizable(false);
		setBounds(100, 100, 510, 329);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		contentPanel.setLayout(new BorderLayout(0, 0));
		
		JPanel panel = new JPanel();
		contentPanel.add(panel, BorderLayout.CENTER);
		panel.setLayout(new FlowLayout(FlowLayout.CENTER, 5, 5));
		{
			PORTB = new PORTPanel('B',(byte) 6);
			panel.add(PORTB);
			PORTC = new PORTPanel('C',(byte) 6);
			panel.add(PORTC);
			PORTD = new PORTPanel('D',(byte) 8);
			panel.add(PORTD);
		}
		
		panel_1 = new JPanel();
		contentPanel.add(panel_1, BorderLayout.SOUTH);
		panel_1.setLayout(new BorderLayout(0, 0));
		
		panel_2 = new JPanel();
		panel_1.add(panel_2, BorderLayout.EAST);
		
		btnNewButton_1 = new JButton("????????????????????????");
		btnNewButton_1.setToolTipText("?????????????????? ?????? ???????????? ?? ?????????????? ?????????????? ?? ????????????????");
		btnNewButton_1.addActionListener(e->{
			capture();
			mode = Mode.Syn;
			dispatchEvent(new PortEvent(REQ_GETSTATUS, PortEvent.Type.DATA_OUT));
		});
		panel_2.add(btnNewButton_1);
		
		btnNewButton = new JButton("????????????????");
		btnNewButton.addActionListener(e->update());
		panel_2.add(btnNewButton);
		
		panel_3 = new JPanel();
		panel_1.add(panel_3, BorderLayout.WEST);
		
		autoCyn = new JCheckBox("??????????????????????????????????");
		autoCyn.addActionListener(e->{if(autoCyn.isSelected())update();});
		autoCyn.setToolTipText("?????????????????????????? ???????????????????? ???? ???????? ?????????????????? ?? ???????????? ??????????????????");
		panel_3.add(autoCyn);
	}

	private void update() {
		if(mode == Mode.Update) return;
		capture();
		mode = Mode.Update;
		byte[] req = { 0x00, 0x00, 0x00, 0x00 };
		if (nowPORT.get('B') != oldPORT.get('B') || nowDDR.get('B') != oldDDR.get('B')) {
			req[1] = 0x04;
			req[2] = nowPORT.get('B');
			req[3] = nowDDR.get('B');
		} else if (nowPORT.get('C') != oldPORT.get('C') || nowDDR.get('C') != oldDDR.get('C')) {
			req[1] = 0x05;
			req[2] = nowPORT.get('C');
			req[3] = nowDDR.get('C');
		} else if (nowPORT.get('D') != oldPORT.get('D') || nowDDR.get('D') != oldDDR.get('D')) {
			req[1] = 0x06;
			req[2] = nowPORT.get('D');
			req[3] = nowDDR.get('D');
		}
		if (req[1] != 0x00) {
			dispatchEvent(new PortEvent(req, PortEvent.Type.DATA_OUT));
		} else if (autoCyn.isSelected()) {
			dispatchEvent(new PortEvent(REQ_GETSTATUS, PortEvent.Type.DATA_OUT));
			new Timer(25, () -> update());
		} else {
			mode = Mode.None;
		}
	}

	public void portData(PortEvent e) {
		if(e.get_Type() != PortEvent.Type.DATA_IN) return;
		byte[] msg = e.getMessage();
		dispatchEvent(new StatusEvent("??????: " + Utils.to_string(msg), StatusEvent.Type.PRINTLN));
		switch (mode) {
		case Update ->{
			if (msg.length == 1 && msg[0] == STK_OK) {
				capture();
				dispatchEvent(new PortEvent(REQ_GETSTATUS, PortEvent.Type.DATA_OUT));
				return;
			} else if(msg.length == 10 && msg[9] == STK_OK) {
				updateState(msg, false);
			}
			listener.liberation();
			mode = Mode.None;
			new Timer(1, ()-> update());
		}
		
		case Syn ->{
			if(msg.length == 10 && msg[9] == STK_OK) {
				updateState(msg, true);
				listener.liberation();
				mode = Mode.None;
				new Timer(1, ()-> update());
			} else {
				dispatchEvent(new PortEvent(REQ_GETSTATUS, PortEvent.Type.DATA_OUT));
			}
		}
		default -> {
			if(msg.length != 1 && msg[0] != STK_OK) return;
			dispatchEvent(new StatusEvent("???????????? ??????????, ???????????????? ???? ??????????: " + Utils.to_string(msg), StatusEvent.Type.ERROR));
			listener.liberation();
		}
		}
	}
	
	private void updateState(byte[] validMsg, boolean All) {
		if (All) {
			nowPORT = new PORT(validMsg[2], validMsg[5], validMsg[8]);
			nowDDR = new DDR(validMsg[1], validMsg[4], validMsg[7]);
		}
		oldPORT = new PORT(validMsg[2], validMsg[5], validMsg[8]);
		oldDDR = new DDR(validMsg[1], validMsg[4], validMsg[7]);
		oldPIN = new PIN(validMsg[0], validMsg[3], validMsg[6]);

		PORTB.update();
		PORTC.update();
		PORTD.update();
	}
	
	
	
	final public void addListener(PortEventListener listener) {
		this.listener = listener;
	}

	final protected void dispatchEvent(PortEvent e) {
		listener.portData(e);
	}
	
	final public void addListener(StatusEventListener listener) {
		listeners.add(listener);
	}
	
	final protected void dispatchEvent(StatusEvent e) {
		for (StatusEventListener listener : listeners) {
			listener.statusEvent(e);
		}
	}
	private void capture(){
		while(!listener.capture(PinsPain.this))
			Utils.pauseMs(10);
	}
	
	public boolean capture(PortEventListener l) {return false;}
	public void liberation() {}
}
