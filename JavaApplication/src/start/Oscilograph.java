package start;

import javax.swing.JPanel;

import java.awt.BasicStroke;
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
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Stroke;

import javax.swing.JRadioButton;
import javax.swing.JTextField;
import javax.swing.JScrollBar;
import java.awt.event.AdjustmentListener;
import java.awt.event.AdjustmentEvent;
import javax.swing.event.AncestorListener;
import javax.swing.event.AncestorEvent;
import java.awt.Button;
import java.awt.TextField;
import java.awt.Font;

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
		
		public String toString() {
			return "Пин " + pin;
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
			V = new REFS("Пит");
			rbp.add(V);
			AREF = new REFS("AREF");
			rbp.add(AREF);
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
			for(int i = 0 ; i < pins.size(); i++)
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
			for(int i = 10 ; i >= 3 ;i--) {
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
					Oscilograph.this.dispatchEvent(new PortEvent((byte)1,PortEvent.Type.SET_LENGHT));
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
						for(int i = 0 ; i <  8 ;i++) {
							if((req[4] & (1 << i)) != 0)
								chanels.add(new Chanel(indexToColor(i)));
						}
						Oscilograph.this.dispatchEvent(new PortEvent(req, PortEvent.Type.DATA_OUT));
						GraphPanel.repaint();
					}
				} else {
					btnNewButton.setText("Запуск");
					if (mode == Mode.A) {
						byte[] req = { 0x00, 0x07, 0x00, 0x00, (byte) 0xFF };
						Oscilograph.this.dispatchEvent(new PortEvent(req, PortEvent.Type.DATA_OUT));
						listener.liberation();
					}
					chanels.clear();
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
	private class Axis{
		public double minVal = 0;
		public double maxVal = 5;
		public double step = 1;
		public double lenght = maxVal - minVal;
		
		public double setValue(double Value) {
			return GraphPanel.upTab + (1.0 - (Value - minVal) / lenght) * GraphPanel.yLenghth;
		}
	}
	
	private class Chanel{
		/**Цвет графика*/
		Color color;
		/** Вообще все возможные точки графика */
		private long points[][] = new long[2][0];
		/**Реальная длинна виртуального массива точек*/
		private int points_lenght = 0;
		/**Массив для временных точек */
		private int timePoints[] = new int[65536];
		/**Длина временного массива*/
		private int timeLenght = 0;
		/**Шаг роста массива точек (отбора памяти)*/
		private final static int step = 256;
		/**Размер шага для построения графика*/
		private final static int delEps = 0;
		/**Предыдущая секунда графика*/
		private long oldTime = 0;
		/** Максимальная длинна буфера, после стольки итераций всё начнётся с самого начала */
		private final static int lenghtBufer = 1_000_000;
		public double averageVoltage = 0;
		
		public Chanel(Color color) {
			super();
			this.color = color;
		}

		private void addPoint(long time, int V) {
			timePoints[timeLenght++] = V;
			if (time == oldTime)
				return;
			
			oldTime = time;

			while (points[0].length <= points_lenght + timeLenght) {
				long localxPoints[][] = new long[2][points_lenght + timeLenght + step];
				System.arraycopy(points[0], 0, localxPoints[0], 0, points[0].length);
				System.arraycopy(points[1], 0, localxPoints[1], 0, points[1].length);
				points = localxPoints;
			}

			for (double i = 0; i < timeLenght; i++) {
				points[0][points_lenght] = (long) ((oldTime + i / timeLenght) * F_con);
				points[1][points_lenght] = timePoints[(int) i];

				points_lenght++;
				if (points_lenght == lenghtBufer)
					points_lenght = 0;
			}
			timeLenght = 0;
		}
		
		private int[][] update() {
			int[][] pointsRet = new int[2][(int) (GraphPanel.xLenghth)];
			int now_points_lenght = points_lenght;
			if(now_points_lenght == 0) return pointsRet;
			long pref = GraphPanel.leftTab;
			int j = pointsRet[0].length - 1;
			double middleV = 0;
			int oldI = now_points_lenght, i;
			averageVoltage = 0;
			for (i = oldI - 1; i >= 0 && j >= 0 && pref >= GraphPanel.leftTab; i--, j--) {
				pointsRet[0][j] = round(GraphPanel.leftTab + 100.0 / GraphPanel.stepT * ((points[0][i] / (F_con) - GraphPanel.offsetT) / 1000.0));
				middleV += points[1][i];
				if(pointsRet[0][j] == pref) {
					j++;
				}else {
					pref = pointsRet[0][j];
					double valV = Oscilograph.this.aref * (middleV / (oldI - i)) / 0xFF;
					averageVoltage += valV*valV;
					pointsRet[1][j] = round(Oscilograph.this.V.setValue(valV));
					oldI = i;
					middleV = 0;
				}
			}
			int[][] ret = new int[2][pointsRet[0].length - 1 - j];
			System.arraycopy(pointsRet[0], j+1, ret[0], 0, ret[0].length);
			System.arraycopy(pointsRet[1], j+1, ret[1], 0, ret[1].length);
			averageVoltage /= ret[0].length;
			averageVoltage = Math.sqrt(averageVoltage);
			averageVoltage = round(Oscilograph.this.V.setValue(averageVoltage));
			return ret;
		}
	}
	
	private class GraphPanel extends JPanel{
		
		/** Отступ от левого края */
		private static final int leftTab = 30;
		/** Отступ от правого края */
		private static final int rightTab = 10;
		/** Отступ снизу */
		private static final int downTab = 20;
		/** Отступ сверху */
		private static final int upTab = 10;
		/** Шаг времени, с/100пк */
		private double stepT = 1;
		private double scaleT = 1;
		/** Смещение времени */
		private static long offsetT = System.currentTimeMillis();
		private boolean drawGrid = true;
		public int yLenghth = 0;
		public int xLenghth = 0;
		
		public GraphPanel(){
			super();
		}
		
		protected void paintComponent(Graphics g) {
	        super.paintComponent(g);
	        xLenghth = getWidth() - leftTab - rightTab;
	        yLenghth = getHeight() - downTab - upTab;
	        
	       	g.setColor(Color.BLACK);
	       	g.fillRect(leftTab, upTab, xLenghth, yLenghth);
	        
	        //Ось напряжения
	       	{
	       		double lenght = V.maxVal - V.minVal;
	        	int x1 = rightTab;
	        	int x2 = x1 + xLenghth;
	       		for(double i = V.minVal ; i < V.maxVal; i+=V.step ) {
		        	if(i == 0) continue;
		        	int y = (int) (upTab + yLenghth * (1 - (i - V.minVal) / lenght));
			       	g.setColor(Color.WHITE);
			       	if(drawGrid)
					drawDashedLine(g, x1, y, x2, y);
			       	g.setColor(Color.RED);
					g.drawString(i + "В", 0, y);
		        }
	        	int y = (int) (upTab + yLenghth * (1 - (0 - V.minVal) / lenght));
	       		g.drawLine(x1, y, x2, y);
	       	}
	        //Ось времени
	       	if(drawGrid){
		       	g.setColor(Color.WHITE);
	        	int y1 = upTab;
	        	int y2 = y1 + yLenghth;
				for (double i = 0;; i += stepT * scaleT) {
					int x = (int) (upTab + 100 * i);
					if (x > upTab + xLenghth)
						break;
					drawDashedLine(g, x, y1, x, y2);
				}
	       	}
	       	g.setColor(Color.BLACK);
	       	g.drawString(stepT * scaleT + "c/дел", xLenghth / 2, upTab + yLenghth + downTab / 2);
	       	

	        if(chanels.size() != 0) {
				offsetT = (long) (System.currentTimeMillis() - stepT * xLenghth / 100 * 1000);
	        	repaint();
	        }
	       	for(Chanel ch : chanels) {
	       		g.setColor(ch.color);
	       		int [][] points = ch.update();
	       		g.drawPolyline(points[0], points[1], points[0].length);
	       		g.drawLine(rightTab,round(ch.averageVoltage), rightTab + xLenghth, round(ch.averageVoltage));
	       	}
	    }

		private void drawDashedLine(Graphics g, int x1, int y1, int x2, int y2){
	        //creates a copy of the Graphics instance
	        Graphics2D g2d = (Graphics2D) g.create();
	        //set the stroke of the copy, not the original 
	        Stroke dashed = new BasicStroke(1, BasicStroke.CAP_BUTT, BasicStroke.JOIN_BEVEL, 0, new float[]{10,25}, 0);
	        g2d.setStroke(dashed);
	        g2d.drawLine(x1, y1, x2, y2);
	        //gets rid of the copy
	        g2d.dispose();
	}
	}
	
	enum Mode {
		A,B,C,D
	}

	Mode mode = Mode.A;
	/** Частота, в Гц, с которой один байт данных передаётся */
	private static int F_con = 0;
	Source suersePanel;
	private PortEventListener listener = null;
	private List<StatusEventListener> listeners = new ArrayList<>();
	private int offsetSyn = 0;
	private int numByte = 0;
	private ArrayList<Chanel> chanels = new ArrayList<>();
	/** Линия напряжения */
	private Axis V = new Axis();
	/**Опорное напряжение*/
	private double aref = 5.0;
	/**Панель для рисования*/
	GraphPanel GraphPanel;
	/**
	 * Create the panel.
	 */
	public Oscilograph() {
		setLayout(new BorderLayout(0, 0));
		

		suersePanel = new Source();
		add(suersePanel, BorderLayout.NORTH);
		
		JPanel panel = new JPanel();
		add(panel, BorderLayout.EAST);
		
		JPanel panel_3 = new JPanel();
		panel.add(panel_3);
		panel_3.setLayout(new BorderLayout(0, 0));
		
		Button button = new Button("<");
		button.addActionListener(e->{GraphPanel.stepT /= 1.5;});
		button.setFont(new Font("Times New Roman", Font.PLAIN, 8));
		panel_3.add(button, BorderLayout.WEST);
		
		Button button_1 = new Button(">");
		button_1.addActionListener(e->{GraphPanel.stepT *= 1.5;});
		button_1.setFont(new Font("Times New Roman", Font.PLAIN, 8));
		panel_3.add(button_1, BorderLayout.EAST);
		
		TextField textField = new TextField();
		textField.setText("1.0");
		textField.setFont(new Font("Times New Roman", Font.PLAIN, 8));
		panel_3.add(textField, BorderLayout.CENTER);
		
		JLabel lblNewLabel_3 = new JLabel("T");
		lblNewLabel_3.setHorizontalAlignment(SwingConstants.CENTER);
		panel_3.add(lblNewLabel_3, BorderLayout.NORTH);
		
		JPanel panel_1 = new JPanel();
		add(panel_1, BorderLayout.WEST);
		
		GraphPanel = new GraphPanel();
		GraphPanel.setBackground(Color.WHITE);
		add(GraphPanel, BorderLayout.CENTER);
	}

	public void portData(PortEvent e) {
		if (mode == Mode.A) {
			long time = System.currentTimeMillis();
			for (byte b : e.getMessage())
				newByte(b & 0xFF,time);
		}
	}
	
	private void newByte(int b, long time) {
		int numChanel = (numByte + offsetSyn) % (chanels.size() + 1);
		if(numChanel == 0) {
			if(b != 0xFF)
				offsetSyn++;
			numByte++;
			return;
		}
		chanels.get(numChanel - 1).addPoint(time,b);
		numByte++;
	}

	private static int round(double d) {
		return (int) Math.round(d);
	}
	private static double ceilAll(double x) {
		return (x > 0) ? Math.ceil(x) : Math.floor(x);
	}
	private String round(double d,int dig) {
		return String.format("%."+dig+"f", d);
	}

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
