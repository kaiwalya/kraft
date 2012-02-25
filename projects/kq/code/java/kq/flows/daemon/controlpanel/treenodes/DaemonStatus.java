package kq.flows.daemon.controlpanel.treenodes;

import javax.swing.JPanel;

public class DaemonStatus extends BaseTreeNode {

	/**
	 * Create the panel.
	 */
	public DaemonStatus() {
		super("Start/Stop Service");
	}

	@Override
	public JPanel getOptionsPanel() {
		return new kq.flows.daemon.controlpanel.panels.DaemonStatus();
	}
}
