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
import java.util.TimerTask;

import javax.swing.DefaultComboBoxModel;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JDialog;
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

import Utils.COMport;
import Utils.Setings;
import Utils.StatusEvent;
import Utils.StatusEventListener;
import Utils.Timer;
import WorkingModes.SetupSpeed;
import WorkingModes.WorkingModes;
import javax.swing.JScrollPane;
import javax.swing.JTextPane;
import javax.swing.JMenuBar;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

public class MainScreen extends JFrame  implements PortEventListener, StatusEventListener{

	private JPanel contentPane;
	
	public final Setings Se = new Setings();
	JComboBox<ComPorts> portSelected = new JComboBox<ComPorts>();
	JPopupMenu menuAddDelComPort = new JPopupMenu();
	JTextPane textArea = new JTextPane();
	JPanel StatusPanel = new JPanel();
	JLabel lblNewLabel = new JLabel("O");
	JComboBox<String> baudRate = new JComboBox<String>();
	private PinsPain painPins = new PinsPain();
	private Oscilograph oscil = new Oscilograph();
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
		setTitle("?????????????????????????? ????????????????");
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setBounds(100, 100, 656, 490);
		
		JMenuBar menuBar = new JMenuBar();
		setJMenuBar(menuBar);
		
		JPanel mainPanel = new JPanel();
		mainPanel.setLayout(new BorderLayout(0, 0));
		
		JMenu mnNewMenu = new JMenu("?????????? ????????????");
		menuBar.add(mnNewMenu);
		
		JMenuItem mntmNewMenuItem = new JMenuItem("?????????????????? ????????????????");
		mntmNewMenuItem.addActionListener(e->setupPort());
		mnNewMenu.add(mntmNewMenuItem);
		
		painPins.setLocationRelativeTo(this);
		JMenuItem setPins = new JMenuItem("?????????????????? IO ??????");
		setPins.addActionListener(e->{
			if(mode != Mode.None) disableMode();
			painPins.setVisible(true);
			});
		mnNewMenu.add(setPins);

		//oscil.add(oscil);
		mainPanel.add(oscil);
		JMenuItem Oscilograph = new JMenuItem("????????????????????");
		Oscilograph.addActionListener(e->{
			mainPanel.add(oscil);
		});
		mnNewMenu.add(Oscilograph);
		contentPane = new JPanel();
		contentPane.setBorder(new EmptyBorder(5, 5, 5, 5));
		setContentPane(contentPane);
		
		JPanel panel = new JPanel();
		
		JScrollPane scrollPane = new JScrollPane();
		GroupLayout gl_contentPane = new GroupLayout(contentPane);
		gl_contentPane.setHorizontalGroup(
			gl_contentPane.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPane.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_contentPane.createParallelGroup(Alignment.LEADING)
						.addComponent(mainPanel, Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, 610, Short.MAX_VALUE)
						.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 610, Short.MAX_VALUE)
						.addComponent(panel, Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, 610, Short.MAX_VALUE))
					.addContainerGap())
		);
		gl_contentPane.setVerticalGroup(
			gl_contentPane.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPane.createSequentialGroup()
					.addComponent(panel, GroupLayout.PREFERRED_SIZE, 29, GroupLayout.PREFERRED_SIZE)
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addComponent(mainPanel, GroupLayout.PREFERRED_SIZE, 316, GroupLayout.PREFERRED_SIZE)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 57, Short.MAX_VALUE))
		);
		
		scrollPane.setViewportView(textArea);
		contentPane.setLayout(gl_contentPane);
		
		JMenuItem mnuNew = new JMenuItem("???????????????? ????????");
		mnuNew.addActionListener(e->{
			String result = JOptionPane.showInputDialog(this,"<html><h2>?????????????? ???????????????? COM ??????????");
			if(result != null) {
				COMport.addMemoryPort(result);
				Se.set("LastPort", result); //?????? ?????? ??????????... ?????? ??????????, ?????????? ???????? ?????????????????? ????????????????????????, ???? ?????? ?????? ?????????????? - ?? ???? :\
				activeComPort();
			}
		});
		menuAddDelComPort.add(mnuNew);
        JMenuItem mnuOpen = new JMenuItem("?????????????? ?????????????? ????????");
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
		portSelected.setToolTipText("?????????? COM??????????.\r\n???? ?????? ?????????? ????????????????/??????????????\r\n????????????????");
		
		baudRate.addActionListener(new ActionListener() {@SuppressWarnings("unchecked")
		public void actionPerformed(ActionEvent e) {
			((ComPorts) portSelected.getSelectedItem()).setBaud(Integer.parseInt(((JComboBox<String>)e.getSource()).getSelectedItem().toString()));
		}});
		baudRate.setModel(new DefaultComboBoxModel<String>(UART_speed));
		panel_4.add(baudRate);
		
		JButton btnNewButton = new JButton("RST");
		btnNewButton.addActionListener(e->((ComPorts) portSelected.getSelectedItem()).DTR());
		panel_4.add(btnNewButton);
		btnNewButton.setToolTipText("?????????????? ???????????? ????????????");
		
		panel_4.add(StatusPanel);
		StatusPanel.setToolTipText("???????????? ??????????????????????");
		StatusPanel.setBackground(Color.RED);
		
		lblNewLabel.setHorizontalAlignment(SwingConstants.CENTER);
		StatusPanel.add(lblNewLabel);
		
		//???????????????????????? ?? ????????
		activeComPort();
		//???????????????? ????????????????
		baudRate.setSelectedItem("9600");
		
		painPins.addListener((PortEventListener) this);
		painPins.addListener((StatusEventListener) this);
		
		oscil.addListener((PortEventListener) this);
		oscil.addListener((StatusEventListener) this);
	}

	/**
	 * ?????????????????? ?????? ????????
	 */
	private void disableMode() {
		switch (mode) {
		case SetSpeed -> {
			mode = Mode.None;
			setStatus(false);
			appendToPane("???????????????????? ???? ????????????????", Color.RED);
		}
		default ->
		throw new IllegalArgumentException("Unexpected value: " + mode);
		}
	}

	/**
	 * ?????????? ?????????????????? ?????????? ??????????????
	 */
	private void setupPort() {
		if(!getStatus()) {
			appendToPane("?????? ?????????? ?? ??????????????", Color.RED);
			return;
		}
		String res = (String) JOptionPane.showInputDialog(null, "?????????? ????????????",
				"?????????? ????????????:", JOptionPane.PLAIN_MESSAGE, null, UART_speed,
				UART_speed[baudRate.getSelectedIndex()]);
		if (res == null) {
			appendToPane("?????????????????????? ???????????????? " + UART_speed[baudRate.getSelectedIndex()] + " ??????", Color.BLACK);
			return;
		}
		
		ComPorts carrentPort = (ComPorts) portSelected.getSelectedItem();
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
				appendToPane("???????????????????? ???? ????????????????", Color.RED);
			}
		});
	}

	private void delComPort() {
		ComPorts event = (ComPorts) portSelected.getSelectedItem();
		COMport.delMemoryPort(event.toString());
		activeComPort();
	}

	private void activeComPort() {		
		DefaultComboBoxModel<ComPorts> comPorts = new DefaultComboBoxModel<>();
		for(ComPorts i : ComPorts.getComPorts(Se)) {
			comPorts.addElement(i); 
			i.addListener((PortEventListener) this);
			i.addListener((StatusEventListener) this);
		}
		portSelected.setModel(comPorts);
		
		try {
			String name = COMport.getLastPortName();	
			for (int i = 0; i < portSelected.getModel().getSize(); i++) {
				ComPorts element = portSelected.getModel().getElementAt(i);
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
		
		ComPorts event = (ComPorts) portSelected.getSelectedItem();
		if(event.isActive()) return;
		
		setStatus(false);
		event.start();
		
		new Timer(25, ()->{((ComPorts) portSelected.getSelectedItem()).DTR();});
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
					appendToPane("?????????????????????? ?????????? ???????????????? " + modeHelp +" ??????", Color.BLACK);
					baudRate.setSelectedItem(modeHelp);
				} else {
					appendToPane("???????????? ?????????????????? ???? ?????????? ????????????????", Color.RED);
				}
				mode = Mode.None;
			}
			default -> {
				String data = "???????????? ???? ?????????????? ???????? ???? ?????????????? ??????: ";
				for (byte theByte : e.getMessage())
					data += String.format("0x%02X ", theByte);
				appendToPane(data, Color.BLACK);
			}
			}
		}
		case DATA_OUT -> {
			if(mode != mode.None) return;
			ComPorts port = (ComPorts) portSelected.getSelectedItem();
			port.send(e.getMessage());
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
