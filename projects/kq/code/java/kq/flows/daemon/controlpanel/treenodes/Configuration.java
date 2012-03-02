package kq.flows.daemon.controlpanel.treenodes;

import javax.swing.JPanel;

public class Configuration extends BaseTreeNode {

	kq.flows.daemon.Configuration configModel;
	kq.flows.daemon.controlpanel.panels.Configuration configView;
	
	
	public Configuration(kq.flows.daemon.Configuration config) {
		super(config, "Configuration Policy");
		configModel = config;
	}

	@Override
	public JPanel getOptionsPanel() {
		if(configView == null) configView = new kq.flows.daemon.controlpanel.panels.Configuration(this);
		configView.setCurrentConfigurationFileString(configModel.getCurrentConfigFilePath());
		configView.setCurrentConfigurationUnsavedChanges(configModel.hadUnsavedChanges());
		return configView;
	}
	
	public void saveConfigurationRequest() throws Exception{
		configModel.saveConfigToCurrentFile();
	}
}
