package kq.flows.daemon.controlpanel.panels;

import javax.swing.JCheckBox;

@SuppressWarnings("serial")
public class NetworkAddress extends BasePanel {
	
	kq.flows.daemon.controlpanel.treenodes.NetworkAddress controller;

	/**
	 * Create the panel.
	 */
	public NetworkAddress(kq.flows.daemon.controlpanel.treenodes.NetworkAddress controller) {
		this.controller = controller;
		setLayout(null);
		
		JCheckBox chckbxEnabledForUse = new JCheckBox("Enabled for use by Daemon");
		chckbxEnabledForUse.setBounds(6, 6, 303, 23);
		add(chckbxEnabledForUse);
	}

}
