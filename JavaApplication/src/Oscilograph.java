import javax.swing.JPanel;
import java.awt.BorderLayout;
import javax.swing.JCheckBox;
import javax.swing.SwingConstants;
import javax.swing.border.TitledBorder;

import Utils.StatusEvent;
import Utils.StatusEventListener;
import Utils.Utils;

import javax.swing.JLabel;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.List;
import java.awt.event.ActionEvent;
import javax.swing.JComboBox;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JButton;
import java.awt.Color;
import javax.swing.JRadioButton;

public class Oscilograph extends JPanel implements PortEventListener {
	
	/** Запускает АЦП */
	private final int ADEN = 7;
	/** Запускает преобразование */
	private final int ADSC = 6;
	/** Разрешает прерывание от АЦП */
	private final int ADIE = 3;
	/** выравнивать результат преобразования по левому краю ADCH:ADCL [9][8][7][6][5][4][3][2]:[1][0][x][x][x][x][x][x] */
	private final int ADLAR = 5;
	
	private static Color indexToColor(int index) {
		switch (index) {
		case 0: return Color.RED;
		case 1: return Color.YELLOW;
		case 2: return Color.GREEN;
		case 3: return Color.GRAY;
		case 4: return Color.CYAN;
		case 5: return Color.PINK;
		case 6: return Color.MAGENTA;
		case 7: return Color.ORANGE;
		case 8: return Color.LIGHT_GRAY;
		case 9: return Color.LIGHT_GRAY;
		case 10: return Color.LIGHT_GRAY;
		default:
		throw new IllegalArgumentException("Unexpected value: " + index);
		}
	}
	
	private class PIN extends JPanel{
		JCheckBox chckbx_1;
		byte pin;
		PIN(byte pin){
			this.pin=pin;
			setLayout(new BorderLayout(0, 0));
			
			JLabel lblNewLabel = null;
			if(pin < 8)
				lblNewLabel = new JLabel(pin+"");
			else if(pin == 8)
				lblNewLabel = new JLabel("t");
			else if(pin == 9)
				lblNewLabel = new JLabel("1.1");
			else if(pin == 10)
				lblNewLabel = new JLabel("0B");
			lblNewLabel.setHorizontalAlignment(SwingConstants.CENTER);
			add(lblNewLabel, BorderLayout.NORTH);
			
			chckbx_1 = new JCheckBox("");
			add(chckbx_1, BorderLayout.SOUTH);
		}
		
		public void setBackground(Color c) {
			super.setBackground(c);
			if(chckbx_1 != null)
			chckbx_1.setBackground(c);
		}
		
		public byte get() {
			if(!chckbx_1.isSelected() || !isVisible()) return 0;
			else return (byte) (1<<pin);
		}
		
		public boolean isSelected() {
			return chckbx_1.isSelected() && isVisible();
		}
	}
	private class PORT extends JPanel{
		ArrayList<PIN> pins = new ArrayList<>();
		RadioButtonPanel rbp = null;
		REFS AREF = null;
		REFS V = null;
		REFS ION = null;
		PORT(){			
			rbp = new RadioButtonPanel();
			AREF = new REFS("AREF");
			rbp.add(AREF);
			V = new REFS("Пит");
			rbp.add(V);
			ION = new REFS("1.1В");
			rbp.add(ION);
			add(rbp);
			
			for(byte i = 11; i > 0 ; i--) {
				PIN pin = new PIN((byte) (i-1));
				pin.setBackground(indexToColor(i-1));
				pins.add(pin);
				add(pin);
			}

			update();
		}
		
		public void update() {
			pins.get(4).setVisible(false);
			pins.get(3).setVisible(false);
			pins.get(2).setVisible(false);
			pins.get(1).setVisible(false);
			pins.get(0).setVisible(false);
			rbp.setVisible(false);
			switch (mode) {
			case B -> {}
			case C -> {}
			case D -> {
				pins.get(4).setVisible(true);
				pins.get(3).setVisible(true);
			}
			case A -> {
				rbp.setVisible(true);
				pins.get(4).setVisible(true);
				pins.get(3).setVisible(true);
				pins.get(2).setVisible(true);
				pins.get(1).setVisible(true);
				pins.get(0).setVisible(true);
			}
			}
		}

		public byte getPins() {
			byte ret = 0;
			for(int i = 0 ; i < 8; i++)
				ret |= pins.get(i).get();
			return ret;
		}
		
		public byte getREFS() {
			if(mode != Mode.A) return 0x00;
			if(AREF.isSelected())
				return 0x00;
			else if(V.isSelected())
				return 0b01000000;
			else //if(ION.isSelected())
				return (byte) 0b11000000;
		}

		public byte getFirst() {
			if(pins.get(0).isSelected()) return 0b1111; //0B
			else if(pins.get(1).isSelected()) return 0b1110; //1.1B
			else if(pins.get(2).isSelected()) return 0b1000; //t
			for(int i = 3 ; i < 11 ;i++) {
				if(pins.get(i).isSelected()) return (byte) (7 - (i - 3));
			}
			return 0;
		}
	}
	
	private class Source extends JPanel{
		PORT port = new PORT();
		
		Source(){
			setBorder(new TitledBorder(null, "Источник сигнала", TitledBorder.CENTER, TitledBorder.TOP, null, null));
			setLayout(new BorderLayout(0, 0));
			
			JComboBox<String> comboBox = new JComboBox<String>();
			comboBox.setModel(new DefaultComboBoxModel<String>(new String[] {"АЦП", "PORTB", "PORTC", "PORTD"}));
			comboBox.addActionListener(new ActionListener() {public void actionPerformed(ActionEvent e) {
				switch (comboBox.getSelectedIndex()) {
				case 0 ->mode = Mode.A;
				case 1 ->mode = Mode.B;
				case 2 ->mode = Mode.C;
				case 3 ->mode = Mode.D;
				default ->
					throw new IllegalArgumentException("Unexpected value: " + comboBox.getSelectedIndex());
				}
				port.update();
			}});
			add(comboBox, BorderLayout.NORTH);
			add(port, BorderLayout.CENTER);
			
			JButton btnNewButton = new JButton("Запуск");
			add(btnNewButton, BorderLayout.EAST);
			btnNewButton.addActionListener(e->{
				if(btnNewButton.getText().equals("Запуск")) {
					btnNewButton.setText("Стоп");
					capture();
					if (mode == Mode.A) {
						byte[] req = { 0x00, 0x07, 0x00, 0x00, 0x00 };
						req[2] = (byte) ((1 << ADEN) | (1 << ADSC) | (1 << ADIE));
						if (F_con < 125_000)
							req[2] |= 0b111;
						else if (F_con < 250_000)
							req[2] |= 0b110;
						else if (F_con < 500_000)
							req[2] |= 0b101;
						else if (F_con < 1_00_000)
							req[2] |= 0b100;
						else if (F_con < 2_00_000)
							req[2] |= 0b011;
						else if (F_con < 4_00_000)
							req[2] |= 0b010;
						else// if(F_con < 8_00_000)
							req[2] |= 0b001;
						req[3] = (byte) (port.getREFS() | (1 << ADLAR) | port.getFirst());
						req[4] = port.getPins();
					}
				} else {
					btnNewButton.setText("Запуск");
					if (mode == Mode.A) {
						byte[] req = { 0x00, 0x07, 0x00, 0x00, 0x00 };
						Oscilograph.this.dispatchEvent(new PortEvent(req, PortEvent.Type.DATA_OUT));
						listener.liberation();
					}
				}
			});
		}
	}
	
	private class REFS extends JPanel{
		JRadioButton rb = null;
		REFS(String name){
			setLayout(new BorderLayout(0, 0));
			
			rb = new JRadioButton("");
			add(rb, BorderLayout.SOUTH);
			
			JLabel lblNewLabel_1 = new JLabel(name);
			add(lblNewLabel_1, BorderLayout.NORTH);
			
		}
		
		public void setSelected(boolean isSelected){
			rb.setSelected(isSelected);
		}
		
		public void addActionListener(ActionListener l) {
			rb.addActionListener(l);
		}
		
		public boolean isSelected() {
			return rb.isSelected();
		}
	}
	
	private class RadioButtonPanel extends JPanel{
		ArrayList<REFS> buttons = new ArrayList<>();
		REFS activ = null;
		
		public void add(REFS r) {
			if(buttons.size() == 0) {
				r.setSelected(true);
				activ = r;
			}
			buttons.add(r);
			super.add(r);
			
			r.addActionListener(e->{
				if(activ != r) {
					activ.setSelected(false);
					r.setSelected(true);
					activ = r;
				} else {
					r.setSelected(true);
				}
			});
		}
	}
	
	enum Mode {
		A,B,C,D
	}

	Mode mode = Mode.A;
	/** Частота, в Гц, с которой один байт данных передаётся */
	int F_con = 0;
	Source suersePanel;
	private PortEventListener listener = null;
	private List<StatusEventListener> listeners = new ArrayList<>();
	/**
	 * Create the panel.
	 */
	public Oscilograph() {
		setLayout(new BorderLayout(0, 0));
		

		suersePanel = new Source();
		add(suersePanel, BorderLayout.NORTH);
		
		JPanel panel = new JPanel();
		add(panel, BorderLayout.EAST);
		
		JPanel panel_1 = new JPanel();
		add(panel_1, BorderLayout.WEST);
		
		JPanel GraphPanel = new JPanel();
		GraphPanel.setBackground(Color.WHITE);
		add(GraphPanel, BorderLayout.CENTER);
	}
	

	public void portData(PortEvent e) {}
	
	public void setBoud(int boud) {
		F_con = (boud * 8) / 10;  // 8 - бит в байте, 10 - бит в посылке (8 данных, старт + стоп)
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
		while(!listener.capture(this))
			Utils.pauseMs(10);
	}
	
	public boolean capture(PortEventListener l) {return false;}
	public void liberation() {}

}
