package start;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.ArrayList;
import java.util.List;

import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.SwingConstants;
import javax.swing.event.AncestorEvent;
import javax.swing.event.AncestorListener;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableModel;

import Utils.StatusEvent;
import Utils.StatusEventListener;
import Utils.Utils;

public class I2C extends JPanel  implements PortEventListener{
	private JTable table;
	
	private enum Mode{//Режим, в котором мы находимся
		NOP,
		//Сканирование шины
		SCAN,
		/**Отправили сообщение*/
		WRITE,
		/**Читаем данные*/
		READ,READ_2,
		
	}
	/**Текущий режим*/
	Mode mode = Mode.NOP;
	/**Тестовый канал*/
	private byte[] REQ_TEST = { 0x00, 0x09, 0x02, 0x00 };
	//Отмечаются те устройства, которые есть у нас
	private ArrayList<Byte> has = new ArrayList<>(0x80);
	/**Выбранное устройство*/
	private byte select = -1;
	/**Длина сообщения, которое надо прочитать*/
	private byte readlen = -1;
	
	//Порт
	/**Слушатель наших жалоб*/
	private PortEventListener arduino = null;
	/**Те, кому надо печатать сообщение*/
	private List<StatusEventListener> printer_log = new ArrayList<>();
	
	//Красилка ячеек
	class MyTableRenderer extends DefaultTableCellRenderer {
	    @Override
	    public Component getTableCellRendererComponent(JTable table, Object value,  boolean isSelected, boolean hasFocus, int row, int column) {
			Component c = super.getTableCellRendererComponent(table, value, isSelected, hasFocus, row, column);
			c.setForeground(Color.WHITE);
			setHorizontalAlignment(SwingConstants.CENTER);
			c.setBackground(Color.BLACK);
			if(column == 0 || row == 0) return c;
			column-- ; row --;
			Byte num = (byte) (row * 0x10 + column);
			if (select == num)
				c.setBackground(Color.YELLOW);
			else if (has.contains(num))
				c.setBackground(Color.GREEN);
			else if (num == REQ_TEST[3])
				c.setBackground(Color.BLUE);
			else
				c.setBackground(Color.GRAY);
			return c;
		}
	}

	/**
	 * Create the panel.
	 */
	public I2C() {
		addAncestorListener(new AncestorListener() {
			public void ancestorAdded(AncestorEvent event) {
				scaning();
			}
			public void ancestorMoved(AncestorEvent event) { /**Мы и так ловим сообщение об изменении размера*/}
			public void ancestorRemoved(AncestorEvent event) {
				stop();
			}
		});
		setLayout(new BorderLayout(0, 0));
		
		JPanel panel = new JPanel();
		add(panel, BorderLayout.SOUTH);
		panel.setLayout(new BorderLayout(0, 0));
		
		JPanel panel_5 = new JPanel();
		panel.add(panel_5, BorderLayout.EAST);
		
		JButton scunBut = new JButton("Сканирование");
		scunBut.addActionListener(e->scaning());
		panel_5.add(scunBut);
		
		JPanel panel_1 = new JPanel();
		add(panel_1, BorderLayout.WEST);
		
		JPanel panel_2 = new JPanel();
		add(panel_2, BorderLayout.EAST);
		
		JPanel panel_3 = new JPanel();
		add(panel_3, BorderLayout.NORTH);
		
		JPanel panel_4 = new JPanel();
		add(panel_4, BorderLayout.CENTER);
		
		table = new JTable();
		panel_4.addComponentListener(new ComponentAdapter() {
			@Override
			public void componentResized(ComponentEvent e) {
				var rc = table.getModel().getColumnCount();
				var h = panel_4.getHeight();
				for(var i = 0 ; i < rc ; i++) {
					table.setRowHeight(i, (int) (1 * h/rc));
				}
			}
		});
		table.setModel(new DefaultTableModel(
			new String[][] {
				{"", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f"},
				{"0x0", null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null},
				{"0x1", null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null},
				{"0x2", null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null},
				{"0x3", null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null},
				{"0x4", null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null},
				{"0x5", null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null},
				{"0x6", null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null},
				{"0x7", null, null, null, null, null, null, null, null, null, null, null, null, null, null, null, null},
			},
			new String[] {"", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f"}
		));
		table.setDefaultRenderer(Object.class, new MyTableRenderer());
	    table.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		table.addMouseListener(new MouseAdapter() {
		    @Override
		    public void mouseClicked(MouseEvent e) {
		    	int row = table.rowAtPoint(e.getPoint());
		    	int col = table.columnAtPoint(e.getPoint());
		    	if(row == 0 || col == 0) {
		    		select = -1;
		    	} else {
			    	Byte num = (byte) ((row - 1) * 0x10 + (col - 1));
			    	select = num;
		    	}
				updateUI();
				if(select >= 0) {
					if(javax.swing.SwingUtilities.isLeftMouseButton(e)) {
						String result = JOptionPane.showInputDialog(I2C.this, "Введите команду в шестнадцетеричном формате которую надо отправить");
						if(result != null && !result.isEmpty()) {
							capture();
							mode = Mode.WRITE;
							ArrayList<Byte> msg = new ArrayList<>();
							msg.add((byte) 0x00);
							msg.add((byte) 0x09);
							var split = result.split(" ");
							msg.add((byte) (split.length + 2));
							msg.add(select);
							for(var i : split)
								msg.add((byte)Integer.parseInt(i.replace("0x", ""), 16));
							dispatchEvent(new PortEvent(msg, PortEvent.Type.DATA_OUT));
						}
					} else if(javax.swing.SwingUtilities.isRightMouseButton(e)) {
						var dialog = new I2CRead(select);
						dialog.setVisible(true);
						var write = dialog.getReq();
						var len = dialog.getLenght();
						if(len != 0) {
							capture();
							mode = Mode.READ;
							readlen = len;
							ArrayList<Byte> msg = new ArrayList<>();
							msg.add((byte) 0x00);
							msg.add((byte) 0x09);
							msg.add((byte) (write.size() + 2));
							msg.add(select);
							msg.addAll(write);
							dispatchEvent(new PortEvent(msg, PortEvent.Type.DATA_OUT));
						}
					}
				}
		    }
		});

		GroupLayout gl_panel_4 = new GroupLayout(panel_4);
		gl_panel_4.setHorizontalGroup(
			gl_panel_4.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_panel_4.createSequentialGroup()
					.addContainerGap()
					.addComponent(table, GroupLayout.DEFAULT_SIZE, 410, Short.MAX_VALUE)
					.addContainerGap())
		);
		gl_panel_4.setVerticalGroup(
			gl_panel_4.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_panel_4.createSequentialGroup()
					.addContainerGap()
					.addComponent(table, GroupLayout.DEFAULT_SIZE, 258, Short.MAX_VALUE)
					.addContainerGap())
		);
		panel_4.setLayout(gl_panel_4);

	}
	
	private void stop() {
		arduino.liberation();
		mode = Mode.NOP;
	}
	
	private void scaning() {
		capture();
		mode = Mode.SCAN;
		has.clear();
		this.updateUI();
		REQ_TEST[3] = 0x00;
		dispatchEvent(new PortEvent(REQ_TEST, PortEvent.Type.DATA_OUT));
	}

	/**Обмен с дуней*/
	public void portData(PortEvent e) {
		if(e.get_Type() != PortEvent.Type.DATA_IN) return;
		byte[] msg = e.getMessage();
		dispatchEvent(new StatusEvent("Вот: " + Utils.to_string(msg), StatusEvent.Type.PRINTLN));
		switch (mode) {
			case SCAN -> {
				if (msg.length == 1 && msg[0] == STK_OK) {
					has.add( REQ_TEST[3]);
				} else if(msg.length == 2 && msg[0] == 0x20 && msg[1] == STK_FAILED) { //Не ответил!
					has.remove(Byte.valueOf(REQ_TEST[3]));
				} else {
					StringBuilder sb = new StringBuilder();
					sb.append("Ошибка от устройства, сообщение: ");
					for(var i : msg) 
						sb.append(String.format("%02X ", i));
					dispatchEvent(new StatusEvent(sb.toString(), StatusEvent.Type.PRINTLN));
				}
				if(REQ_TEST[3] < 0x7F) {
					REQ_TEST[3]++;
					dispatchEvent(new PortEvent(REQ_TEST, PortEvent.Type.DATA_OUT));
				} else {
					REQ_TEST[3] = -1;
					mode = Mode.NOP;
				}
				this.updateUI();
			}
			case WRITE -> {
				if (msg.length == 1 && msg[0] == STK_OK) {
					dispatchEvent(new StatusEvent("Отправлено", StatusEvent.Type.PRINTLN));
				} else {
					dispatchEvent(new StatusEvent("Не отправлено", StatusEvent.Type.ERROR));
				}
				mode = Mode.NOP;
			}
			case READ -> {
				if (msg.length == 1 && msg[0] == STK_OK) {
					ArrayList<Byte> msgOut = new ArrayList<>();
					msgOut.add((byte) 0x00);
					msgOut.add((byte) 0x0A);
					msgOut.add(select);
					msgOut.add(readlen);
					mode = Mode.READ_2;
					dispatchEvent(new PortEvent(msgOut, PortEvent.Type.DATA_OUT));
				} else {
					dispatchEvent(new StatusEvent("Не смогли связаться с утсройством", StatusEvent.Type.ERROR));
					mode = Mode.NOP;
				}
			}
			case READ_2 -> {
				if (msg.length >= 1 && msg[0] == STK_OK) {
					StringBuilder ans = new StringBuilder();
					ans.append("Приняли: ");
					for(var i = 1 ; i < msg.length ; i++ ) 
						ans.append(" 0x"+Integer.toHexString(0xFF & msg[i]));
					dispatchEvent(new StatusEvent(ans.toString(), StatusEvent.Type.PRINTLN));
				} else {
					dispatchEvent(new StatusEvent("Не смогли принять данные", StatusEvent.Type.ERROR));
				}
				mode = Mode.NOP;
			}
			
			
			
			default -> {
				System.out.print("Приняли: ");
				for(var i : e.getMessage()) {
					System.out.print(" 0x"+Integer.toHexString(0xFF&i));
				}
				System.out.println();
			}
		}
	}
	
	public final void addListener(PortEventListener listener) {
		this.arduino = listener;
	}

	protected final void dispatchEvent(PortEvent e) {
		arduino.portData(e);
	}
	
	public final void addListener(StatusEventListener listener) {
		printer_log.add(listener);
	}
	
	protected final void dispatchEvent(StatusEvent e) {
		for (StatusEventListener listener : printer_log) {
			listener.statusEvent(e);
		}
	}
	private void capture(){
		while(!arduino.capture(this))
			Utils.pauseMs(10);
	}
	
	public boolean capture(PortEventListener l) {return false;}
	public void liberation() {/**Нам нечего освобождать, мы и так свободны!*/}
}
