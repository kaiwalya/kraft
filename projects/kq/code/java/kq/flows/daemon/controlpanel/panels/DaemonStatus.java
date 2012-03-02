package kq.flows.daemon.controlpanel.panels;
import javax.swing.JLabel;


@SuppressWarnings("serial")
public class DaemonStatus extends BasePanel {
	
	

	/**
	 * Create the panel.
	 */
	public DaemonStatus(kq.flows.daemon.controlpanel.treenodes.DaemonStatus controller) {
		super(controller);
		
		JLabel lblDaemonAliveNo = new JLabel("Daemon Alive: No");
		add(lblDaemonAliveNo);

	}

}
