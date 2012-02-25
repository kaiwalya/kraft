package kq.flows.daemon.controlpanel.panels;
import javax.swing.JButton;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import javax.swing.JRadioButton;


public class Configuration extends BasePanel {

	/**
	 * Create the panel.
	 */
	public Configuration() {
		
		JRadioButton rdbtnBlockedUntilAllowed = new JRadioButton("Blocked until allowed");
		add(rdbtnBlockedUntilAllowed);
		
		JRadioButton rdbtnAllowedUntillBlocked = new JRadioButton("Allowed until blocked");
		add(rdbtnAllowedUntillBlocked);

	}

}
