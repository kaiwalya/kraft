package kq.flows.daemon.controlpanel.panels;
import javax.swing.JButton;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import javax.swing.JLabel;


@SuppressWarnings("serial")
public class Configuration extends BasePanel implements ActionListener {
	private JLabel lblCurrentConfiguration;
	
	private kq.flows.daemon.controlpanel.treenodes.Configuration controller;
	private JButton btnSave;

	/**
	 * Create the panel.
	 */
	public Configuration(kq.flows.daemon.controlpanel.treenodes.Configuration controllr) {
		super(controllr);
		controller = controllr;
		
		lblCurrentConfiguration = new JLabel();
		add(lblCurrentConfiguration);
		
		btnSave = new JButton("Save");
		btnSave.addActionListener(this);
		add(btnSave);

	}
	
	public void setCurrentConfigurationFileString(String s){
		lblCurrentConfiguration.setText(s);
	}
	
	public void setCurrentConfigurationUnsavedChanges(boolean b){
		btnSave.setEnabled(b);
	}

	@Override
	public void actionPerformed(ActionEvent e){
		if(e.getActionCommand().equals("Save")){
			try{
				controller.saveConfigurationRequest();
			}
			catch(Exception ex){
				ex.printStackTrace();
			}
		}
	}

}
