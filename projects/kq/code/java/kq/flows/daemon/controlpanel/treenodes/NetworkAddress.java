package kq.flows.daemon.controlpanel.treenodes;

import java.net.InterfaceAddress;
import java.net.NetworkInterface;
import java.util.ArrayList;
import java.util.List;

import javax.swing.JPanel;

import kq.net.NetworkManager;


public class NetworkAddress extends BaseTreeNode {
	
	enum Type{
		typeRoot,
		typeIFace,
		typeAddress,
	}
	
	Type type;
	
	InterfaceAddress addr;
	NetworkInterface iface;
	
	private void populateChildren(NetworkInterface iface) throws Exception{

		//Get child interfaces
		{
			List<NetworkInterface> networkAddresses = new ArrayList<NetworkInterface>();
			//NetworkManager networkManager = NetworkManager.getInstance(4784);
			NetworkManager.appendCurrentChildInterfaces(iface, networkAddresses);
			for(NetworkInterface addr : networkAddresses){
				if(!addr.isLoopback()) insert(new NetworkAddress(getConfiguration(), addr), 0);
			}
		}
		
		//Get addresses
		if(iface != null){
			List<InterfaceAddress> networkAddresses = new ArrayList<InterfaceAddress>();
			//NetworkManager networkManager = NetworkManager.getInstance(4784);
			NetworkManager.appendCurrentAddresses(iface, networkAddresses);
			for(InterfaceAddress addr : networkAddresses){
				insert(new NetworkAddress(getConfiguration(), addr), 0);
			}
		}	
	}
	
	private NetworkAddress(kq.flows.daemon.Configuration config, InterfaceAddress addr) throws Exception{
		super(config, addr.getAddress().getHostAddress());
		type = Type.typeAddress;
		this.addr = addr;
	}
	
	private NetworkAddress(kq.flows.daemon.Configuration config, NetworkInterface iface) throws Exception{
		super(config, iface.getDisplayName());
		type = Type.typeIFace;
		this.iface = iface;
		
		populateChildren(iface);
	}

	/**
	 * Create the panel.
	 */
	public NetworkAddress(kq.flows.daemon.Configuration config) throws Exception{
		super(config, "Network Addresses");
		type = Type.typeRoot;
		
		//populateChildren(null);
	}
	
	@Override
	public JPanel getOptionsPanel() {
		return new kq.flows.daemon.controlpanel.panels.NetworkAddress(this);
	}
	
	public void changeUsage(boolean bUse){
		getConfiguration().changeNetworkUsage(bUse);
	}
}
