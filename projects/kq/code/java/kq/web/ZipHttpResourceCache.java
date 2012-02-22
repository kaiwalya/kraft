package kq.web;

import java.beans.XMLDecoder;
import java.beans.XMLEncoder;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URI;
import java.util.HashMap;
import java.util.Map;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

import kq.web.Base64Coder;

public class ZipHttpResourceCache implements IHTTPResourceCache {
		
	File datastore;
	Map<String, String> map;
	boolean bDirty;
	IHTTPResourceCache fallback;
	
	public ZipHttpResourceCache(String parent) {
		_init(parent, null);
	}
	public ZipHttpResourceCache(String parent, IHTTPResourceCache fallback){
		_init(parent, fallback);
	}
	
	@SuppressWarnings("unchecked")
	public void _init(String parent, IHTTPResourceCache fallback){
		this.fallback = fallback;
		map = null;
		datastore = new File(parent, "downloadstore.zip");
		
		if(datastore.exists() && datastore.isFile() && datastore.canRead()){
			try{
				ZipInputStream is = new ZipInputStream(new FileInputStream(datastore));
				is.getNextEntry();
				XMLDecoder dec = new XMLDecoder(is);
				
				Object o = dec.readObject();
				map = (HashMap<String, String>)o;
				
				dec.close();
			}
			catch(Exception e){
				e.printStackTrace();
			}
		}
		else{
			File fparent = new File(parent);
			if(!fparent.exists()){
				fparent.mkdirs();
			}
		}

		if(map == null){
			map = new HashMap<String, String>();
		}
		
		bDirty = false;
		
	}

	
	private void set(String url, InputStream is) throws IOException{
		
		ByteArrayOutputStream os = new ByteArrayOutputStream();
		byte [] buff = new byte[1024];
		int iValid = 0;
		while((iValid = is.read(buff, 0, 1024)) != -1){
			os.write(buff, 0, iValid);
		}
		map.put(url, new String(Base64Coder.encode(os.toByteArray())));
		bDirty = true;
	}
	
	public InputStream find(String url){
		String data = map.get(url);
		if(data != null){
			return new ByteArrayInputStream(Base64Coder.decode((data.toCharArray())));
		}
		else{
			try{
				URI uri = new URI(url);
				InputStream is;
				if(fallback != null){
					is = fallback.getResource(uri, null);
				}
				else{
					is = uri.toURL().openStream();
				}
				set(url, is);
				persist();
				return find(url);
			}
			catch(Exception e){
				e.printStackTrace();
			}
		}
		return null;
	}
	
	private void persist(){
		if(!bDirty)return;
		try{
			ZipOutputStream os = new ZipOutputStream(new FileOutputStream(datastore));
			os.putNextEntry(new ZipEntry("store.xml"));
			XMLEncoder enc = new XMLEncoder(os);
			enc.writeObject(map);
			enc.close();
		}
		catch(Exception e){
			e.printStackTrace();
		}
	}


	@Override
	public InputStream getResource(URI uri, String sContentType) {
		return find(uri.toString());
		
	}
}
