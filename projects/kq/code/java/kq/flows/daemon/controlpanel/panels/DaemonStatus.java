package kq.flows.daemon.controlpanel.panels;
import javax.swing.JLabel;


public class DaemonStatus extends BasePanel {

	/**
	 * Create the panel.
	 */
	public DaemonStatus() {
		
		JLabel lblDaemonAliveNo = new JLabel("Daemon Alive: No");
		add(lblDaemonAliveNo);

	}

}
