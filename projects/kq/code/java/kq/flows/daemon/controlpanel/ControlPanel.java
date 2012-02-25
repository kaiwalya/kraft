package kq.flows.daemon.controlpanel;

import java.awt.EventQueue;
import java.awt.SystemColor;

import java.net.*;
import java.util.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.*;

import kq.flows.daemon.*;
import java.awt.Dimension;


public class ControlPanel implements TreeSelectionListener {

	private Configuration configuration;
	private JFrame frmDaemonControlPanel;
	private JTree tree;
	private JLabel lblTask;
	private JProgressBar progressBar;
	
	
	List<InterfaceAddress> networkAddresses;
	
	private static ControlPanel gWindow;
	private JScrollPane panel;

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) throws Exception{
		
		EventQueue.invokeAndWait(new Runnable() {
			public void run() {
				try {
					ControlPanel window = new ControlPanel();
					window.frmDaemonControlPanel.setVisible(true);
					gWindow = window;
					
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
		
		gWindow.start(args);
		
	}
	
	private void start(String [] args) throws Exception{

		lblTask.setText("Initializing Configurations...");
		this.configuration = new Configuration(args);
		
		
		//Prepare the tree to interaction
		{

			DefaultMutableTreeNode root = new DefaultMutableTreeNode();
			lblTask.setText("Initializing Daemon Status...");
			root.add(new kq.flows.daemon.controlpanel.treenodes.DaemonStatus());
			lblTask.setText("Initializing Configurations");
			root.add(new kq.flows.daemon.controlpanel.treenodes.Configuration());
			lblTask.setText("Initializing Network");
			root.add(new kq.flows.daemon.controlpanel.treenodes.NetworkAddress());
			tree.setModel(new DefaultTreeModel(root));
			for(int i = 0; i < tree.getRowCount(); i++){
				tree.expandRow(i);
			}
			tree.addTreeSelectionListener(this);
		}
		lblTask.setText("");
		
		frmDaemonControlPanel.validate();
	}

	/**
	 * Create the application.
	 */
	public ControlPanel() throws Exception{
		initialize();
	}

	/**
	 * Initialize the contents of the frame.
	 */
	@SuppressWarnings("serial")
	private void initialize() {
		frmDaemonControlPanel = new JFrame();
		frmDaemonControlPanel.setResizable(false);
		frmDaemonControlPanel.setBackground(SystemColor.window);
		frmDaemonControlPanel.getContentPane().setBackground(SystemColor.window);
		frmDaemonControlPanel.getContentPane().setLayout(null);
		
		JSplitPane splitPane = new JSplitPane();
		splitPane.setBounds(6, 6, 788, 401);
		frmDaemonControlPanel.getContentPane().add(splitPane);
		
		JScrollPane scrollPaneTree = new JScrollPane();
		splitPane.setLeftComponent(scrollPaneTree);
		
		tree = new JTree();
		tree.setRootVisible(false);
		tree.setModel(new DefaultTreeModel(
			new DefaultMutableTreeNode("Settings") {
				{
				}
			}
		));
		scrollPaneTree.setViewportView(tree);
		
		panel = new JScrollPane();
		splitPane.setRightComponent(panel);
		
		progressBar = new JProgressBar();
		progressBar.setBounds(6, 432, 788, 20);
		frmDaemonControlPanel.getContentPane().add(progressBar);
		
		lblTask = new JLabel("Loading...");
		lblTask.setBounds(6, 414, 788, 16);
		frmDaemonControlPanel.getContentPane().add(lblTask);
		frmDaemonControlPanel.setTitle("Daemon Control Panel");
		frmDaemonControlPanel.setBounds(100, 100, 800, 480);
		frmDaemonControlPanel.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	}

	@Override
	public void valueChanged(TreeSelectionEvent e){
		//panel.removeAll();
		
		Object o = tree.getLastSelectedPathComponent();
		if(o != null){
			JPanel panelinner = ((kq.flows.daemon.controlpanel.treenodes.BaseTreeNode)o).getOptionsPanel();
			panelinner.setSize(panel.getSize());
			panel.add(panelinner);
			panel.setViewportView(panelinner);
		}
		
	}
}
