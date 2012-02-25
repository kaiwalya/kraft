package kq.flows.daemon.controlpanel.treenodes;

import javax.swing.JPanel;

public class Configuration extends BaseTreeNode {

	public Configuration() {
		super("Configuration Policy");
	}

	@Override
	public JPanel getOptionsPanel() {
		return new kq.flows.daemon.controlpanel.panels.Configuration();
	}
}
