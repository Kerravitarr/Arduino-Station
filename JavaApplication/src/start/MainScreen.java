package start;

import ComPort.RealCOM;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.Graphics;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.text.SimpleDateFormat;
import java.util.Date;

import javax.swing.DefaultComboBoxModel;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.SwingConstants;
import javax.swing.border.EmptyBorder;
import javax.swing.event.PopupMenuEvent;
import javax.swing.event.PopupMenuListener;
import javax.swing.text.AttributeSet;
import javax.swing.text.SimpleAttributeSet;
import javax.swing.text.StyleConstants;
import javax.swing.text.StyleContext;

import ComPort.COMport;
import ComPort.EasyReq;
import ComPort.Settings;
import Utils.StatusEvent;
import Utils.StatusEventListener;
import Utils.Timer;
import WorkingModes.SetupSpeed;
import WorkingModes.WorkingModes;
import javax.swing.JScrollPane;
import javax.swing.JTextPane;
import javax.swing.JMenuBar;

public class MainScreen extends JFrame  implements PortEventListener, StatusEventListener{

	private JPanel contentPane;
	
	JComboBox<RealCOM> portSelected = new JComboBox<RealCOM>();
	JPopupMenu menuAddDelComPort = new JPopupMenu();
	JTextPane textArea = new JTextPane();
	JPanel StatusPanel = new JPanel();
	JLabel lblNewLabel = new JLabel("O");
	JComboBox<String> baudRate = new JComboBox<String>();
	/**Панель с высоким/низким уровнем по каждому пину*/
	private PinsPain painPins = new PinsPain();
	/**Осцилограф*/
	private Oscilograph oscil = new Oscilograph();
	/**Расшритель портов*/
	private ManyPorts maniOutPorts = new ManyPorts();
	/**Модуль I2C*/
	private I2C i2c = new I2C();
	private final String[] UART_speed = new String[] { "300", "1200", "2400", "4800", "9600", "19200", "38400", "57600","115200", "230400", "250000" };

	private Mode mode = Mode.None;
	private String modeHelp = "";
	private PortEventListener translater = null;
	
	enum Mode{
		None, SetSpeed, WaiteRet
	}

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					MainScreen frame = new MainScreen();
					frame.setVisible(true);
					Dimension dim = java.awt.Toolkit.getDefaultToolkit().getScreenSize();    
					int x = (dim.width-frame.getSize().width)/2;
					int y = (dim.height-frame.getSize().height)/2;    
					frame.setLocation(x, y);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	/**
	 * Create the frame.
	 */
	public MainScreen() {
		setTitle("ИЗМЕРИТЕЛЬНЫЙ КОМПЛЕКС");
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setBounds(100, 100, 656, 490);
		
		JMenuBar menuBar = new JMenuBar();
		setJMenuBar(menuBar);
		
		JPanel mainPanel = new JPanel();
		mainPanel.setLayout(new BorderLayout(0, 0));
		
		JMenu mnNewMenu = new JMenu("Режим работы");
		menuBar.add(mnNewMenu);
		
		JMenuItem mntmNewMenuItem = new JMenuItem("Настройка скорости");
		mntmNewMenuItem.addActionListener(e->setupPort());
		mnNewMenu.add(mntmNewMenuItem);
		
		//painPins.setLocationRelativeTo(this);
		JMenuItem setPins = new JMenuItem("Настройка IO ног");
		setPins.addActionListener(e->{
			if(mode != Mode.None) disableMode();
			mainPanel.remove(0);
			mainPanel.add(painPins);
			mainPanel.updateUI();
			});
		mnNewMenu.add(setPins);

		//mainPanel.add(oscil);
		mainPanel.add(painPins);
		JMenuItem Oscilograph = new JMenuItem("Осцилограф");
		Oscilograph.addActionListener(e->{
			mainPanel.remove(0);
			mainPanel.add(oscil);
			mainPanel.updateUI();
		});
		mnNewMenu.add(Oscilograph);
		JMenuItem manyPorts = new JMenuItem("Расширитель выходов");
		manyPorts.addActionListener(e->{
			mainPanel.remove(0);
			mainPanel.add(maniOutPorts);
			mainPanel.updateUI();
		});
		mnNewMenu.add(manyPorts);
		
		JMenuItem menu_i2c = new JMenuItem("I2C");
		menu_i2c.addActionListener(e-> {
			mainPanel.remove(0);
			mainPanel.add(i2c);
			mainPanel.updateUI();
		});
		mnNewMenu.add(menu_i2c);
		contentPane = new JPanel();
		contentPane.setBorder(new EmptyBorder(5, 5, 5, 5));
		setContentPane(contentPane);
		
		JPanel panel = new JPanel();
		
		JScrollPane scrollPane = new JScrollPane();
		GroupLayout gl_contentPane = new GroupLayout(contentPane);
		gl_contentPane.setHorizontalGroup(
			gl_contentPane.createParallelGroup(Alignment.TRAILING)
				.addGroup(Alignment.LEADING, gl_contentPane.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPane.createParallelGroup(Alignment.LEADING)
						.addComponent(mainPanel, GroupLayout.PREFERRED_SIZE, 610, Short.MAX_VALUE)
						.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 681, Short.MAX_VALUE)
						.addComponent(panel, Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, 681, Short.MAX_VALUE))
					.addContainerGap())
		);
		gl_contentPane.setVerticalGroup(
			gl_contentPane.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPane.createSequentialGroup()
					.addComponent(panel, GroupLayout.PREFERRED_SIZE, 29, GroupLayout.PREFERRED_SIZE)
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addComponent(mainPanel, GroupLayout.DEFAULT_SIZE, 316, Short.MAX_VALUE)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(scrollPane, GroupLayout.PREFERRED_SIZE, 57, GroupLayout.PREFERRED_SIZE))
		);
		
		scrollPane.setViewportView(textArea);
		contentPane.setLayout(gl_contentPane);
		
		JMenuItem mnuNew = new JMenuItem("Добавить порт");
		mnuNew.addActionListener(e->{
			String result = JOptionPane.showInputDialog(this,"<html><h2>Введите название COM порта");
			if(result != null) {
				COMport.addMemoryPort(result);
				Utils.Constants.Settings.set("LastPort", result); //Вот тут криво... Мне нужно, чтобы порт автоматом переключался, но как это сделать - я хз :\
				activeComPort();
			}
		});
		menuAddDelComPort.add(mnuNew);
        JMenuItem mnuOpen = new JMenuItem("Удалить текущий порт");
        mnuOpen.addActionListener(e->delComPort());
        menuAddDelComPort.add(mnuOpen);
		

		DefaultComboBoxModel<WorkingModes> def = new DefaultComboBoxModel<>();
		def.addElement(new SetupSpeed()); 
		panel.setLayout(new BorderLayout(0, 0));
		
		JPanel panel_3 = new JPanel();
		panel.add(panel_3, BorderLayout.WEST);
		
		JPanel panel_4 = new JPanel();
		panel.add(panel_4, BorderLayout.EAST);
		panel_4.add(portSelected);
		
		portSelected.addActionListener(e -> SelectCom());
		portSelected.addMouseListener(new MouseAdapter() {public void mouseClicked(MouseEvent e) {
			if(e.getButton() == MouseEvent.BUTTON3)
				menuAddDelComPort.show(portSelected, 0, -menuAddDelComPort.getPreferredSize().height);
		}});
		portSelected.addPopupMenuListener(new PopupMenuListener() {
			public void popupMenuCanceled(PopupMenuEvent e) {}
			public void popupMenuWillBecomeInvisible(PopupMenuEvent e) {}
			public void popupMenuWillBecomeVisible(PopupMenuEvent e) {activeComPort();}
		});
		portSelected.setToolTipText("Выбор COMпорта.\r\nПо ПКМ можно добавить/удалить\r\nэлементы");
		
		baudRate.addActionListener(new ActionListener() {
			@SuppressWarnings("unchecked")
			public void actionPerformed(ActionEvent e) {
				int boud = Integer.parseInt(((JComboBox<String>) e.getSource()).getSelectedItem().toString());
				((RealCOM) portSelected.getSelectedItem()).setBaud(boud);
				oscil.setBoud(boud);
			}
		});
		baudRate.setModel(new DefaultComboBoxModel<String>(UART_speed));
		panel_4.add(baudRate);
		
		JButton btnNewButton = new JButton("RST");
		btnNewButton.addActionListener(e->((RealCOM) portSelected.getSelectedItem()).DTR());
		panel_4.add(btnNewButton);
		btnNewButton.setToolTipText("Послать сигнал сброса");
		
		panel_4.add(StatusPanel);
		StatusPanel.setToolTipText("Статус подключения");
		StatusPanel.setBackground(Color.RED);
		
		lblNewLabel.setHorizontalAlignment(SwingConstants.CENTER);
		StatusPanel.add(lblNewLabel);
		
		//Подключаемся к кому
		activeComPort();
		//Выбираем скорость
		baudRate.setSelectedItem("9600");
		
		painPins.addListener((PortEventListener) this);
		painPins.addListener((StatusEventListener) this);
		
		oscil.addListener((PortEventListener) this);
		oscil.addListener((StatusEventListener) this);
		
		i2c.addListener((PortEventListener) this);
		i2c.addListener((StatusEventListener) this);
	}

	/**
	 * Выключает все моды
	 */
	private void disableMode() {
		switch (mode) {
		case SetSpeed -> {
			mode = Mode.None;
			setStatus(false);
			appendToPane("Устройство не отвечает", Color.RED);
		}
		default ->
		throw new IllegalArgumentException("Unexpected value: " + mode);
		}
	}

	/**
	 * Режим настройки порта адруино
	 */
	private void setupPort() {
		if(!getStatus()) {
			appendToPane("Нет связи с Ардуино", Color.RED);
			return;
		}
		String res = (String) JOptionPane.showInputDialog(null, "Выбор канала",
				"Номер канала:", JOptionPane.PLAIN_MESSAGE, null, UART_speed,
				UART_speed[baudRate.getSelectedIndex()]);
		if (res == null) {
			appendToPane("Установлена скорость " + UART_speed[baudRate.getSelectedIndex()] + " бод", Color.BLACK);
			return;
		}
		
		RealCOM carrentPort = (RealCOM) portSelected.getSelectedItem();
		byte[] req = {0x00,0x03,0x00,0x00};
		
		int F_CPU = 16000000;
		int baudrate = Integer.parseInt(res);
		int bauddivider = F_CPU / (16 * baudrate) - 1;
		req[2] = (byte) (bauddivider&0xFF);
		req[3] = (byte) ((bauddivider >> 8) & 0xFF);
		modeHelp = res;
		carrentPort.send(req);
		mode = Mode.SetSpeed;
		
		new Timer(500, ()-> {
			if(mode == Mode.SetSpeed) {
				mode = Mode.None;
				setStatus(false);
				appendToPane("Устройство не отвечает", Color.RED);
			}
		});
	}

	private void delComPort() {
		RealCOM event = (RealCOM) portSelected.getSelectedItem();
		COMport.delMemoryPort(event.toString());
		activeComPort();
	}

	private void activeComPort() {		
		DefaultComboBoxModel<RealCOM> comPorts = new DefaultComboBoxModel<>();
		for(RealCOM i : RealCOM.getComPorts()) {
			comPorts.addElement(i); 
			//i.addListener((PortEventListener) this);
			//i.addListener((StatusEventListener) this);
		}
		portSelected.setModel(comPorts);
		
		try {
			String name = COMport.getLastPortName();	
			for (int i = 0; i < portSelected.getModel().getSize(); i++) {
				RealCOM element = portSelected.getModel().getElementAt(i);
				if (element.toString().equals(name)) {
					portSelected.setSelectedIndex(i);
					return;
				}
			}
			portSelected.setSelectedIndex(0);
		} catch (java.lang.EnumConstantNotPresentException e) {
			portSelected.setSelectedIndex(0);
		}
	}

	private void SelectCom() {
		
		RealCOM event = (RealCOM) portSelected.getSelectedItem();
		if(event.isActive()) return;
		
		setStatus(false);
		event.start();
		Utils.Constants.COM_PORT.set(event);
		
		new Timer(25, () -> {
			((COMport) portSelected.getSelectedItem()).DTR();
			Utils.Constants.COM_PORT.lock(MainScreen.this);
			setStatus(Utils.Constants.COM_PORT.write(new EasyReq.Ping()).answer == Settings.ANSWER.STK_OK);
			Utils.Constants.COM_PORT.unlock(MainScreen.this);
		});
	}
	
	public void paint(Graphics g) {
		super.paint(g);
	}

	
	
	@Override
	public void statusEvent(StatusEvent e) {
		switch (e.get_Type()) {
		case PRINT ->textArea.setText(textArea.getText() + e.getMessage());
		case PRINTLN ->  appendToPane(e.getMessage(), Color.DARK_GRAY);
		case ERROR -> appendToPane(e.getMessage(), Color.BLUE);
		case FATAL_ERROR -> appendToPane(e.getMessage(), Color.RED);
		}
	}

	@Override
	public void portData(PortEvent e) {
		switch (e.get_Type()) {
		case DATA_IN -> {
			setStatus(true);
			if(translater != null) {
				 translater.portData(e);
				 return;
			}
			switch (mode) {
				case SetSpeed -> {
					if(e.getMessage()[0] == STK_OK) {
						appendToPane("Установлена новая скорость " + modeHelp +" бод", Color.BLACK);
						baudRate.setSelectedItem(modeHelp);
					} else {
						appendToPane("Ошибка настройки на новую скорость", Color.RED);
					}
					mode = Mode.None;
				}
				default -> {
					String data = "Пришло не понятно кому не понятно что: ";
					for (byte theByte : e.getMessage())
						data += String.format("0x%02X ", theByte);
					appendToPane(data, Color.BLACK);
				}
			}
		}
		case DATA_OUT -> {
			if(mode != mode.None) return;
			RealCOM port = (RealCOM) portSelected.getSelectedItem();
			port.send(e.getMessage());
		}
		case SET_LENGHT ->{
			RealCOM port = (RealCOM) portSelected.getSelectedItem();
			port.setLenght(e.getMessage()[0]);
		}

		}
	}

	private void setStatus(boolean isActiv) {
		if(isActiv) {
			StatusPanel.setBackground(Color.GREEN);
			lblNewLabel.setText("I");
		} else {
			StatusPanel.setBackground(Color.RED);
			lblNewLabel.setText("O");
		}
	}
	
	private boolean getStatus() {
		return lblNewLabel.getText().equals("I");
	}
	
	private void appendToPane(String msg, Color c) {
		JTextPane tp = textArea;
		StyleContext sc = StyleContext.getDefaultStyleContext();
		AttributeSet aset = sc.addAttribute(SimpleAttributeSet.EMPTY, StyleConstants.Foreground, c);

		aset = sc.addAttribute(aset, StyleConstants.FontFamily, "Lucida Console");
		aset = sc.addAttribute(aset, StyleConstants.Alignment, StyleConstants.ALIGN_JUSTIFIED);

		int len = tp.getDocument().getLength();
		tp.setCaretPosition(len);
		tp.setCharacterAttributes(aset, false);
		
		Date dateNow = new Date();
	    SimpleDateFormat formatForDateNow = new SimpleDateFormat("hh:mm:ss");
	      
		tp.replaceSelection("\n" + formatForDateNow.format(dateNow) + " -> " +  msg);
	}

	@Override
	public boolean capture(PortEventListener l) {
		if(translater != null && translater != l) return false;
		translater = l;
		return true;
	}

	@Override
	public void liberation() {
		translater = null;
	}
}
