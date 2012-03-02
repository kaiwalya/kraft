package kq.flows.daemon.controlpanel.panels;

import javax.swing.JCheckBox;
import java.awt.event.ItemListener;
import java.awt.event.ItemEvent;

@SuppressWarnings("serial")
public class NetworkAddress extends BasePanel implements ItemListener {
	private JCheckBox checkBox;
	kq.flows.daemon.controlpanel.treenodes.NetworkAddress  controller;
	/**
	 * Create the panel.
	 */
	public NetworkAddress(kq.flows.daemon.controlpanel.treenodes.NetworkAddress control) {
		super(control);
		controller = control;
		setLayout(null);
		
		checkBox = new JCheckBox("Enabled for use by Daemon");
		checkBox.addItemListener(this);
		checkBox.setBounds(6, 6, 303, 23);
		add(checkBox);
	}

	@Override
	public void itemStateChanged(ItemEvent e) {
		controller.changeUsage(checkBox.isSelected());
	}

}
