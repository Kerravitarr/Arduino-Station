package start;


import ComPort.SET_PORT;
import Utils.StatusEventListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.util.ArrayList;
import java.util.List;
import javax.swing.DefaultListModel;

/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/GUIForms/JPanel.java to edit this template
 */

/**
 *
 * @author Илья
 */
public class ManyPorts extends javax.swing.JPanel{

	/** Creates new form ManyPorts */
	public ManyPorts() {
		initComponents();
		_countBytes.setValue(Utils.Constants.Settings.getInt("COUNT_OUT_PORTS", 1));
		_DS.setValue(Utils.Constants.Settings.getInt("OUT_PORTS_M_DS", 0));
		_ST.setValue(Utils.Constants.Settings.getInt("OUT_PORTS_M_ST", 0));
		_SH.setValue(Utils.Constants.Settings.getInt("OUT_PORTS_M_SH", 0));
		keyListener(_countBytes);
		keyListener(_DS);
		keyListener(_ST);
		keyListener(_SH);
		
		_countBytes.addChangeListener((e) -> Utils.Constants.Settings.set("COUNT_OUT_PORTS", ((Number)_countBytes.getValue()).intValue()));
		_DS.addChangeListener((e) -> Utils.Constants.Settings.set("OUT_PORTS_M_DS", ((Number)_DS.getValue()).intValue()));
		_ST.addChangeListener((e) -> Utils.Constants.Settings.set("OUT_PORTS_M_ST", ((Number)_ST.getValue()).intValue()));
		_SH.addChangeListener((e) -> Utils.Constants.Settings.set("OUT_PORTS_M_SH", ((Number)_SH.getValue()).intValue()));
	}
			/**
	 * Функция заставляет спинер срабатывать от каждого нажатия кнопки!
	 * @param spiner 
	 */
	private void keyListener(javax.swing.JSpinner spiner){
       final var jtf = ((javax.swing.JSpinner.DefaultEditor) spiner.getEditor()).getTextField();
		var val = spiner.getValue();
		if (val instanceof Integer) {
			jtf.addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				String text = jtf.getText().replace(",", "");
				int oldCaretPos = jtf.getCaretPosition();
				try {
					Integer newValue = Integer.valueOf(text);
					spiner.setValue(newValue);
					jtf.setCaretPosition(oldCaretPos);
				} catch (NumberFormatException ex) {
					//Not a number in text field -> do nothing
				} catch(java.lang.IllegalArgumentException ex){ //Удалили цифру, каретка уехала
					jtf.setCaretPosition(oldCaretPos - 1);
				}
			}
			});
		} else if (val instanceof Double) {
			jtf.addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				String text = jtf.getText().replace(",", "");
				int oldCaretPos = jtf.getCaretPosition();
				try {
					Double newValue = Double.valueOf(text);
					spiner.setValue(newValue);
					jtf.setCaretPosition(oldCaretPos);
				} catch (NumberFormatException ex) {
					//Not a number in text field -> do nothing
				} catch(java.lang.IllegalArgumentException ex){ //Удалили цифру, каретка уехала
					jtf.setCaretPosition(oldCaretPos - 1);
				}
			}
			});
		}  else if (val instanceof Long) {
			jtf.addKeyListener(new KeyAdapter() {
			@Override
			public void keyReleased(KeyEvent e) {
				String text = jtf.getText().replace(",", "");
				int oldCaretPos = jtf.getCaretPosition();
				try {
					Long newValue = Long.valueOf(text);
					spiner.setValue(newValue);
					jtf.setCaretPosition(oldCaretPos);
				} catch (NumberFormatException ex) {
					//Not a number in text field -> do nothing
				} catch(java.lang.IllegalArgumentException ex){ //Удалили цифру, каретка уехала
					jtf.setCaretPosition(oldCaretPos - 1);
				}
			}
			});
		} 
	}

	/** This method is called from within the constructor to
	 * initialize the form.
	 * WARNING: Do NOT modify this code. The content of this method is
	 * always regenerated by the Form Editor.
	 */
	@SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jPanel1 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        _countBytes = new javax.swing.JSpinner();
        jLabel2 = new javax.swing.JLabel();
        _DS = new javax.swing.JSpinner();
        jLabel3 = new javax.swing.JLabel();
        _ST = new javax.swing.JSpinner();
        jLabel4 = new javax.swing.JLabel();
        _SH = new javax.swing.JSpinner();
        jPanel2 = new javax.swing.JPanel();
        jLabel5 = new javax.swing.JLabel();
        _command = new javax.swing.JTextField();
        jButton1 = new javax.swing.JButton();
        jScrollPane1 = new javax.swing.JScrollPane();
        jList1 = new javax.swing.JList<>();

        jPanel1.setLayout(new javax.swing.BoxLayout(jPanel1, javax.swing.BoxLayout.LINE_AXIS));

        jLabel1.setText("Плат:");
        jPanel1.add(jLabel1);

        _countBytes.setModel(new javax.swing.SpinnerNumberModel(1, 1, null, 1));
        jPanel1.add(_countBytes);

        jLabel2.setText("DS:");
        jPanel1.add(jLabel2);

        _DS.setModel(new javax.swing.SpinnerNumberModel(0, 0, 13, 1));
        jPanel1.add(_DS);

        jLabel3.setText("ST:");
        jPanel1.add(jLabel3);

        _ST.setModel(new javax.swing.SpinnerNumberModel(0, 0, 13, 1));
        jPanel1.add(_ST);

        jLabel4.setText("SH:");
        jPanel1.add(jLabel4);

        _SH.setModel(new javax.swing.SpinnerNumberModel(0, 0, 13, 1));
        jPanel1.add(_SH);

        jPanel2.setLayout(new javax.swing.BoxLayout(jPanel2, javax.swing.BoxLayout.LINE_AXIS));

        jLabel5.setText("Записать:");
        jPanel2.add(jLabel5);

        _command.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                _commandActionPerformed(evt);
            }
        });
        jPanel2.add(_command);

        jButton1.setText("Отправить");
        jButton1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButton1ActionPerformed(evt);
            }
        });
        jPanel2.add(jButton1);

        jList1.setModel(_list);
        jScrollPane1.setViewportView(jList1);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, 445, Short.MAX_VALUE)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jPanel2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jScrollPane1))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 237, Short.MAX_VALUE)
                .addContainerGap())
        );
    }// </editor-fold>//GEN-END:initComponents

    private void jButton1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButton1ActionPerformed
        var cmd = _command.getText();
		var commands = cmd.split(" ");
		var cmdBytes = new int[((Number)_countBytes.getValue()).intValue()];
		for (int i = 0; i < cmdBytes.length; i++) {
			if(i < commands.length && !commands[i].isEmpty()){
				cmdBytes[i] = Integer.decode(commands[i]);
			} else {
				cmdBytes[i] = 0;
			}
		}
		var port = Utils.Constants.COM_PORT;
		port.lock(this);
		//Первым делом мы инициализируем порты
		var g = new ComPort.SET_PORT.Generator();
		var DS = SET_PORT.Generator.PIN.values[((Number)_DS.getValue()).intValue()];
		var ST = SET_PORT.Generator.PIN.values[((Number)_ST.getValue()).intValue()];
		var SH = SET_PORT.Generator.PIN.values[((Number)_SH.getValue()).intValue()];
		port.write(g.pinMode(DS, SET_PORT.Generator.PIN_MODE.OUTPUT));
		port.write(g.pinMode(ST, SET_PORT.Generator.PIN_MODE.OUTPUT));
		port.write(g.pinMode(SH, SET_PORT.Generator.PIN_MODE.OUTPUT));
		port.write(g.pinWrite(DS, SET_PORT.Generator.PIN_STATE.LOW));
		port.write(g.pinWrite(ST, SET_PORT.Generator.PIN_STATE.LOW));
		port.write(g.pinWrite(SH, SET_PORT.Generator.PIN_STATE.LOW));
		
		//Защёлка - выкл
		port.write(g.pinWrite(ST, SET_PORT.Generator.PIN_STATE.LOW));
		for (int bt = cmdBytes.length - 1; bt >= 0; bt--) {
			int i = cmdBytes[bt];
			for(var bit = 7 ; bit >=0 ; bit--){
				port.write(g.pinWrite(SH, SET_PORT.Generator.PIN_STATE.LOW));
				port.write(g.pinWrite(DS, ((i & (1 << bit)) == 0) ? SET_PORT.Generator.PIN_STATE.LOW : SET_PORT.Generator.PIN_STATE.HIGH));
				port.write(g.pinWrite(SH, SET_PORT.Generator.PIN_STATE.HIGH));
			}
		}
		//Обновляем
		port.write(g.pinWrite(ST, SET_PORT.Generator.PIN_STATE.HIGH));
		
		String row = "";
		for (var i : cmdBytes) {
			for(var bit = 7 ; bit >=0 ; bit--){
				row += ((i & (1 << bit)) == 0) ? "0" : "1";
			}
			row += " ";
		}
		_list.addElement(row);
		
		port.unlock(this);
    }//GEN-LAST:event_jButton1ActionPerformed

    private void _commandActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event__commandActionPerformed
       jButton1ActionPerformed(null);
    }//GEN-LAST:event__commandActionPerformed

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JSpinner _DS;
    private javax.swing.JSpinner _SH;
    private javax.swing.JSpinner _ST;
    private javax.swing.JTextField _command;
    private javax.swing.JSpinner _countBytes;
    private javax.swing.JButton jButton1;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JList<String> jList1;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JScrollPane jScrollPane1;
    // End of variables declaration//GEN-END:variables

	//Порт
	/**Слушатель наших жалоб*/
	private PortEventListener arduino = null;
	/**Те, кому надо печатать сообщение*/
	private List<StatusEventListener> printer_log = new ArrayList<>();
	/**Все состояния, что у нас когда либо были*/
	private DefaultListModel<String> _list = new DefaultListModel<>();
}
