package kq.flows.daemon;

import java.io.File;
import java.io.FileInputStream;
import java.util.Properties;


class Configuration{
	
	
	private Properties propertiesGlobal;
	private static final String keyGlobalPWD = "user.dir";
	private static final String keyGlobalCP = "java.class.path";
	private static final String keyGlobalUserDir = "user.home"; 
	
	private String globalPWD;
	private String globalCP;
	private String globalUserDir;
	
	
	private File configFile;
	private Properties propertiesLocal;
	private static final String keyLocalLastUpdate = "Last Update";
	

	public Configuration(String [] arrParams) throws Exception{
		propertiesGlobal = System.getProperties();
//		for(Entry<Object, Object> pair:props.entrySet()){
//			System.out.println("" + pair.getKey() + "\t" + pair.getValue());
//		}
		globalPWD = propertiesGlobal.getProperty(keyGlobalPWD);
		globalCP = propertiesGlobal.getProperty(keyGlobalCP);
		globalUserDir = propertiesGlobal.getProperty(keyGlobalUserDir);
		propertiesLocal = null;
	}
	
	
	boolean loadLocalConfig() throws Exception{
		boolean ret = false;
		String homeDirectoryString = globalUserDir;
		//System.out.println("Home Directory: " + homeDirectoryString);
		File homeDirectory = new File(homeDirectoryString);
		if(homeDirectory.exists() && homeDirectory.isDirectory() && homeDirectory.canRead()){
			File configDirectory = new File(homeDirectory, ".kq");
			//If config directory doesnt exist create one
			if(configDirectory.exists() || (homeDirectory.canWrite() && configDirectory.mkdir())){
				String namespaceString = ControlPanel.class.getName();
				namespaceString = namespaceString.substring(0, namespaceString.lastIndexOf("."));
				File daemonConfig = new File(configDirectory, namespaceString);			
				if((daemonConfig.exists() || daemonConfig.createNewFile()) && daemonConfig.canRead() && daemonConfig.canWrite()){
					//System.out.println("Trying Config File: " + daemonConfig.getAbsolutePath());

					configFile = daemonConfig;
					FileInputStream daemonConfigFileInputStream = new FileInputStream(configFile);
					Properties local = new Properties();
					local.load(daemonConfigFileInputStream);
					daemonConfigFileInputStream.close();
					
					if(local.containsKey(keyLocalLastUpdate)){
						propertiesLocal = local;
						return true;
					}
				}
			}
		}

		return ret;
	}
	
	
	String getGlobalWorkingDirectory(){
		return globalPWD;
	}
	
	String getGlobalClassPath(){
		return globalCP;
	}
	
	String getUserDirectory(){
		return globalUserDir;
	}	
	
}