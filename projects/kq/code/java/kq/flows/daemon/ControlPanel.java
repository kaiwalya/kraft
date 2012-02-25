package kq.flows.daemon;


import java.io.File;
import java.util.concurrent.*;

import javax.swing.*;

import kq.net.NetworkManager;


public class ControlPanel {
	
	public static void main(String[] args) {
		String homeDirectoryString = System.getProperty("user.home");
		System.out.println("Home Directory: " + homeDirectoryString);
		File homeDirectory = new File(homeDirectoryString);
		if(homeDirectory.exists() && homeDirectory.isDirectory() && homeDirectory.canRead()){
			File configDirectory = new File(homeDirectory, ".kq");
			//If config directory doesnt exist create one
			if(configDirectory.exists() || (homeDirectory.canWrite() && configDirectory.mkdir())){
				String namespaceString = ControlPanel.class.getName();
				namespaceString = namespaceString.substring(0, namespaceString.lastIndexOf("."));
				File daemonConfig = new File(configDirectory, namespaceString);
				try{
					if((daemonConfig.exists() || daemonConfig.createNewFile()) && daemonConfig.canRead() && daemonConfig.canWrite()){
						System.out.println("Config File: " + daemonConfig.getAbsolutePath());
						new ControlPanel(daemonConfig);
					}
				}
				catch(Exception e){
					e.printStackTrace(System.err);
				}
			}
		}
		for(String s: args){
			System.out.println(s);
		}
	}
	
	class ParametrizedCallable<Out, In> implements Callable<Out>{
		
		In in;
		
		public ParametrizedCallable(In in){
			this.in = in;
		}
		
		public Out call(In in){return null;}
		
		@Override
		public Out call(){
			return call(in);
		}
	};
	
	private class MainFrame extends JFrame{
		ControlPanel cpl;
		MainFrame(ControlPanel cpl){
			super("Daemon Control Panel");
			this.cpl = cpl;
			setSize(640, 480);
			
			JPanel panel = new JPanel();
			
			panel.add(new JButton("Quit"));
			
			getContentPane().add(panel);
			setDefaultCloseOperation(DISPOSE_ON_CLOSE);
			
		}
		
		
	}


	private kq.net.NetworkManager networkManager;
	private MainFrame mainFrame;
	
	
	protected ControlPanel(File configFile){
		try{
			ExecutorService async = Executors.newCachedThreadPool();
			
			Future<NetworkManager> networkManagerFuture = async.submit(new ParametrizedCallable<NetworkManager, ControlPanel>(this) {
				@Override
				public NetworkManager call(ControlPanel in) {
					try{
						return kq.net.NetworkManager.getInstance();
					}
					catch(Exception e){
						e.printStackTrace();
					}
					return null;
				}
			});
			
			Future<MainFrame> mainFrameFuture = async.submit(new ParametrizedCallable<MainFrame, ControlPanel>(this){
				@Override
				public MainFrame call(ControlPanel in) {
					MainFrame f = new MainFrame(in);
					return f;
				}
			});
			
			networkManager = networkManagerFuture.get();
			mainFrame = mainFrameFuture.get();
			async.shutdown();
			if(networkManager == null){
				throw new Exception("Something went wrong");
			}
			
			mainFrame.setVisible(true);
			
			
		}
		catch(Exception e){
			e.printStackTrace(System.err);
		}
	}
	
}
