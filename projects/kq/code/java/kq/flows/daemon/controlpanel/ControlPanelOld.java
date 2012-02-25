package kq.flows.daemon.controlpanel;


import java.io.File;
import java.util.concurrent.*;
import kq.net.NetworkManager;


public class ControlPanelOld {
	
	public static void main(String[] args) {
		String homeDirectoryString = System.getProperty("user.home");
		System.out.println("Home Directory: " + homeDirectoryString);
		File homeDirectory = new File(homeDirectoryString);
		if(homeDirectory.exists() && homeDirectory.isDirectory() && homeDirectory.canRead()){
			File configDirectory = new File(homeDirectory, ".kq");
			//If config directory doesnt exist create one
			if(configDirectory.exists() || (homeDirectory.canWrite() && configDirectory.mkdir())){
				String namespaceString = ControlPanelOld.class.getName();
				namespaceString = namespaceString.substring(0, namespaceString.lastIndexOf("."));
				File daemonConfig = new File(configDirectory, namespaceString);
				try{
					if((daemonConfig.exists() || daemonConfig.createNewFile()) && daemonConfig.canRead() && daemonConfig.canWrite()){
						System.out.println("Config File: " + daemonConfig.getAbsolutePath());
						new ControlPanelOld(daemonConfig);
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


	private kq.net.NetworkManager networkManager;
	
	protected ControlPanelOld(File configFile){
		try{
			ExecutorService async = Executors.newCachedThreadPool();
			
			Future<NetworkManager> networkManagerFuture = async.submit(new ParametrizedCallable<NetworkManager, ControlPanelOld>(this) {
				@Override
				public NetworkManager call(ControlPanelOld in) {
					try{
						return kq.net.NetworkManager.getInstance();
					}
					catch(Exception e){
						e.printStackTrace();
					}
					return null;
				}
			});
			
			
			
			networkManager = networkManagerFuture.get();
			
			async.shutdown();
			if(networkManager == null){
				throw new Exception("Something went wrong");
			}
			
		}
		catch(Exception e){
			e.printStackTrace(System.err);
		}
	}
	
}
