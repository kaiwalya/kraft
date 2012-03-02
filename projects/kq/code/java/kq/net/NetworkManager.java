package kq.net;

import java.awt.peer.TextComponentPeer;
import java.net.*;
import java.nio.channels.Channel;
import java.nio.channels.Selector;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

//TODO: Network interfaces might change at runtime?

public class NetworkManager {
	private class ManagementInfo{
		public Channel channel;
		public InterfaceAddress ifaceaddr;
		public InetSocketAddress inetaddr;
	}
	
	private Map<InterfaceAddress, ManagementInfo> info;  
	int port;
	Selector selector;
	
	
	private static Map<Integer, NetworkManager> gManagerMap = null; 
	public static NetworkManager getInstance(int iPort) throws Exception{
		if(gManagerMap == null) gManagerMap = new HashMap<Integer, NetworkManager>();
		Integer port = Integer.valueOf(iPort);
		if(gManagerMap.containsKey(port)){
			return gManagerMap.get(port);
		}
		else{
			gManagerMap.put(port, new NetworkManager(iPort));
			return getInstance(iPort);
		}
	} 
	
	static public void appendCurrentChildInterfaces(NetworkInterface parent, List<NetworkInterface> ifacelist) throws SocketException{
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
	
	static public void appendCurrentAddresses(NetworkInterface iface, List<InterfaceAddress> addrs) throws SocketException{
		addrs.addAll(iface.getInterfaceAddresses());
	}
	 
	public boolean changeManagement(InterfaceAddress addrnet, boolean manage){
		ManagementInfo minfo;
		if(info.containsKey(addrnet) && !manage){
			minfo = info.get(addrnet);
		}
		else if(manage){
			minfo = new ManagementInfo();
		}
		else{
			//Nothing to do!!!
			System.err.println("Nothing to do!! in changeManagement");
			return true;
		}
		
		InetSocketAddress saddr = new InetSocketAddress(addrnet.getAddress(), port); 
		InetAddress addr = saddr.getAddress();
		try{
			DatagramSocket s = new DatagramSocket(port, addr);
			String hello = "hello\n";
			DatagramPacket p = new DatagramPacket(hello.getBytes(), 1);
			p.setAddress(addr);
			s.send(p);
			minfo.ifaceaddr = addrnet;
			minfo.inetaddr = saddr;
			info.put(addrnet, minfo);
			return true;
		}
		catch(Exception e){
			e.printStackTrace();
		}
		return false;
	}
	
	public void startManaging(List<InterfaceAddress> addrs){
		for(InterfaceAddress addrnet:addrs){
			changeManagement(addrnet, true);
		}
	}
	
	public void stopManaging(List<InterfaceAddress> addrs){
		for(InterfaceAddress addrnet:addrs){
			changeManagement(addrnet, false);
		}
	}
	
	private NetworkManager(int iPort) throws Exception{
		port = iPort;
		info = new HashMap<InterfaceAddress, NetworkManager.ManagementInfo>();
		selector = Selector.open();
	}
	
}
