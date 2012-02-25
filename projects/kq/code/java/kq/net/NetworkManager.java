package kq.net;

import java.net.*;
import java.util.Enumeration;
import java.util.List;

//TODO: Network interfaces might change at runtime?

public class NetworkManager {

	private static NetworkManager gManager = null; 
	public static NetworkManager getInstance() throws SocketException{
		if(gManager == null) gManager = new NetworkManager();
		return gManager;
	} 
	
	public void appendCurrentChildInterfaces(NetworkInterface parent, List<NetworkInterface> ifacelist) throws SocketException{
		Enumeration<NetworkInterface> ifaces;
		if(parent == null){
			ifaces = NetworkInterface.getNetworkInterfaces();
		}
		else{
			ifaces = parent.getSubInterfaces();
		}
		
		while(ifaces.hasMoreElements()){
			ifacelist.add(ifaces.nextElement());
		}
	}
	
	public void appendCurrentAddresses(NetworkInterface iface, List<InterfaceAddress> addrs) throws SocketException{
		addrs.addAll(iface.getInterfaceAddresses());
	}
	
	private NetworkManager(){
		
	}
	
}
