package kq.flows.daemon.controlpanel.panels;

import javax.swing.JPanel;

import kq.flows.daemon.controlpanel.treenodes.BaseTreeNode;

@SuppressWarnings("serial")
public class BasePanel extends JPanel {
	
	kq.flows.daemon.controlpanel.treenodes.BaseTreeNode controller;

	/**
	 * Create the panel.
	 */
	public BasePanel(kq.flows.daemon.controlpanel.treenodes.BaseTreeNode controller) {
		this.controller = controller;
	}
	
	public BaseTreeNode getController(){
		return controller;
	}

}
