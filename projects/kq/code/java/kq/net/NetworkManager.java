package kq.net;

import java.net.*;
import java.util.Enumeration;

//TODO: Network interfaces might change at runtime?

public class NetworkManager {

	private static NetworkManager gManager = null; 
	public static NetworkManager getInstance() throws SocketException{
		if(gManager == null) gManager = new NetworkManager();
		return gManager;
	}
	
	Enumeration<NetworkInterface> ifaces;
	
	private int visitInterfaces(Enumeration<NetworkInterface> ifaces) throws SocketException{
		int ifaceCount = 0;
		NetworkInterface iface;
		
		System.out.println();
		while(ifaces.hasMoreElements() && (iface = ifaces.nextElement()) != null){
			System.out.println("Display Name:" + iface.getDisplayName());
			System.out.println("Name:" + iface.getName());
			
			for(InterfaceAddress addr : iface.getInterfaceAddresses()){
				
				System.out.println("\tInterface: " + addr.getAddress().getHostAddress()/* + " | " + addr.getAddress().getCanonicalHostName() + " | " + addr.getAddress().getHostName()*/);				
			}
			
			ifaceCount += visitInterfaces(iface.getSubInterfaces());
			System.out.println();
			ifaceCount++;
		}
		
		return ifaceCount;
	}
	
	private NetworkManager() throws SocketException{
		System.out.println("Enumerating interfaces...");
		int ifaceCount = visitInterfaces(NetworkInterface.getNetworkInterfaces());
		System.out.println("Interfaces Found: " + ifaceCount);
	}
	
}
