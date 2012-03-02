package kq.flows.daemon;

import java.net.InterfaceAddress;
import java.net.NetworkInterface;
import java.util.ArrayList;
import java.util.List;

import kq.flows.daemon.controlpanel.ControlPanel;
import kq.net.NetworkManager;

class Daemon{
	
	class DaemonWorker implements Runnable{
		public void run(){
		
		}
	}	
	
	Daemon(Configuration p) throws Exception{
		if(!p.loadConfigFromDefaultFile()){
			String command = "java -cp " + p.getGlobalClassPath() + " " + ControlPanel.class.getCanonicalName();
			System.err.println("Looks like the first run, starting the Control Panel: \"" + command +"\"");
			Runtime.getRuntime().exec(command);
		}
		
		if(!p.getLocalNetworkUsage()){
			System.err.println("Unable to access network no rights.");
			return;
		}
		
		List<InterfaceAddress> addresses = new ArrayList<InterfaceAddress>();
		{
			List<NetworkInterface> ifaces = new ArrayList<NetworkInterface>();
			NetworkManager.appendCurrentChildInterfaces(null, ifaces);
			while(ifaces.size() != 0){
				NetworkInterface iface = ifaces.remove(0);
				NetworkManager.appendCurrentChildInterfaces(iface, ifaces);
				NetworkManager.appendCurrentAddresses(iface, addresses);
			}
		}
		
		NetworkManager networkManager = NetworkManager.getInstance(4784);
		networkManager.startManaging(addresses);
		
	}
	
	public static void main(String [] arrParams) throws Exception{
		new Daemon(new Configuration(arrParams));
	}
}