package start;

import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.util.ArrayList;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;
import javax.swing.text.AttributeSet;
import javax.swing.text.BadLocationException;
import javax.swing.text.DocumentFilter;
import javax.swing.text.PlainDocument;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.LayoutStyle.ComponentPlacement;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

public class I2CRead extends JDialog {

	private final JPanel contentPanel = new JPanel();
	/**Команда на чтение*/
	private JTextField fieldReq;
	/**Длина ответа*/
	private JTextField countAns;
	
	/**Фильтр только числовых значений в поле!*/
	public class DigitFilter extends DocumentFilter {
	    private static final String DIGITS = "\\d+";

	    @Override
	    public void insertString(FilterBypass fb, int offset, String string, AttributeSet attr) throws BadLocationException {
	        if (string.matches(DIGITS)) {
	            super.insertString(fb, offset, string, attr);
	        }
	    }
	        
	    @Override
	    public void replace(FilterBypass fb, int offset, int length, String string, AttributeSet attrs) throws BadLocationException {
	        if (string.matches(DIGITS)) {
	            super.replace(fb, offset, length, string, attrs);
	        }
	    }
	}

	/**
	 * Create the dialog.
	 */
	public I2CRead(byte adr) {
		setModal(true);
		setAlwaysOnTop(true);
		setTitle("Запрос к устройству 0х" + Integer.toHexString(0xFF&adr));
		setBounds(100, 100, 450, 155);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);
		contentPanel.setLayout(new BorderLayout(0, 0));
		{
			JPanel panel = new JPanel();
			contentPanel.add(panel, BorderLayout.CENTER);
			
			JLabel lblNewLabel = new JLabel("Запрос: ");
			
			fieldReq = new JTextField();
			fieldReq.setColumns(10);
			
			JLabel lblNewLabel_1 = new JLabel("Длина ответа");
			
			countAns = new JTextField();
			PlainDocument doc = (PlainDocument) countAns.getDocument();
			doc.setDocumentFilter(new DigitFilter());
			countAns.setColumns(10);
			GroupLayout gl_panel = new GroupLayout(panel);
			gl_panel.setHorizontalGroup(
				gl_panel.createParallelGroup(Alignment.LEADING)
					.addGroup(gl_panel.createSequentialGroup()
						.addContainerGap()
						.addGroup(gl_panel.createParallelGroup(Alignment.TRAILING)
							.addComponent(lblNewLabel_1)
							.addComponent(lblNewLabel))
						.addPreferredGap(ComponentPlacement.RELATED)
						.addGroup(gl_panel.createParallelGroup(Alignment.LEADING)
							.addComponent(fieldReq, GroupLayout.DEFAULT_SIZE, 358, Short.MAX_VALUE)
							.addComponent(countAns, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
						.addContainerGap())
			);
			gl_panel.setVerticalGroup(
				gl_panel.createParallelGroup(Alignment.LEADING)
					.addGroup(gl_panel.createSequentialGroup()
						.addContainerGap()
						.addGroup(gl_panel.createParallelGroup(Alignment.BASELINE)
							.addComponent(lblNewLabel)
							.addComponent(fieldReq, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
						.addPreferredGap(ComponentPlacement.RELATED)
						.addGroup(gl_panel.createParallelGroup(Alignment.BASELINE)
							.addComponent(lblNewLabel_1)
							.addComponent(countAns, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
						.addContainerGap(167, Short.MAX_VALUE))
			);
			panel.setLayout(gl_panel);
		}
		{
			JPanel buttonPane = new JPanel();
			buttonPane.setLayout(new FlowLayout(FlowLayout.RIGHT));
			getContentPane().add(buttonPane, BorderLayout.SOUTH);
			{
				JButton okButton = new JButton("OK");
				okButton.setActionCommand("OK");
				okButton.addActionListener(event->{dispose();});
				buttonPane.add(okButton);
				getRootPane().setDefaultButton(okButton);
			}
			{
				JButton cancelButton = new JButton("Cancel");
				cancelButton.addActionListener(event->{countAns.setText("");dispose();});
				cancelButton.setActionCommand("Cancel");
				buttonPane.add(cancelButton);
			}
		}
	}
	
	public ArrayList<Byte> getReq(){
		if(fieldReq.getText().isEmpty()) return new ArrayList<>();
		ArrayList<Byte> msg = new ArrayList<>();
		var split = fieldReq.getText().split(" ");
		for(var i : split)
			msg.add((byte)Integer.parseInt(i.replace("0x", ""), 16));
		return msg;
	}
	
	public byte getLenght() {
		if(countAns.getText().isEmpty())
			return 0;
		else 
			return Byte.parseByte(countAns.getText());
	}
}
