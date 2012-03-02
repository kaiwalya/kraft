package kq.flows.daemon;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.text.DateFormat;
import java.util.Date;
import java.util.Properties;


public class Configuration{
	
	
	private Properties propertiesGlobal;
	private static final String keyGlobalPWD = "user.dir";
	private static final String keyGlobalCP = "java.class.path";
	private static final String keyGlobalUserDir = "user.home"; 
	
	private String globalPWD;
	private String globalCP;
	private String globalUserDir;
	
	
	private File defaultConfigFile;
	private File configFile;
	private Properties propertiesLocal;
	
	private static final String keyLocalLastUpdate = "Last Update";
	private static final String keyLocalNetworkUsage = "NetworkUsage";
	
	private boolean hasPendingChange;
	

	public Configuration(String [] arrParams) throws Exception{
		propertiesGlobal = System.getProperties();
//		for(Entry<Object, Object> pair:props.entrySet()){
//			System.out.println("" + pair.getKey() + "\t" + pair.getValue());
//		}
		globalPWD = propertiesGlobal.getProperty(keyGlobalPWD);
		globalCP = propertiesGlobal.getProperty(keyGlobalCP);
		globalUserDir = propertiesGlobal.getProperty(keyGlobalUserDir);
		propertiesLocal = new Properties();
		configFile = null;
		hasPendingChange = false;
		defaultConfigFile = null;
		defaultConfigFile = getDefaultConfigFile(false);
		
	}
	
	public File getDefaultConfigFile(boolean bCreate) throws Exception{
		if(defaultConfigFile != null && !bCreate){
			return defaultConfigFile;
		}
		
		if(bCreate){
			String homeDirectoryString = globalUserDir;
			File homeDirectory = new File(homeDirectoryString);
			if(homeDirectory.exists() && homeDirectory.isDirectory() && homeDirectory.canRead()){
				File configDirectory = new File(homeDirectory, ".kq");
				if(configDirectory.exists() || (homeDirectory.canWrite() && configDirectory.mkdir())){
					String namespaceString = Daemon.class.getName();
					namespaceString = namespaceString.substring(0, namespaceString.lastIndexOf("."));
					File daemonConfig = new File(configDirectory, namespaceString);			
					if((daemonConfig.exists() || daemonConfig.createNewFile()) && daemonConfig.canRead() && daemonConfig.canWrite()){
						return daemonConfig;
					}
				}
			}
		}
		/*
		else{
			String homeDirectoryString = globalUserDir;
			File homeDirectory = new File(homeDirectoryString);
			if(homeDirectory.exists() && homeDirectory.isDirectory() && homeDirectory.canRead()){
				File configDirectory = new File(homeDirectory, ".kq");
				if(configDirectory.exists()){
					String namespaceString = Daemon.class.getName();
					namespaceString = namespaceString.substring(0, namespaceString.lastIndexOf("."));
					File daemonConfig = new File(configDirectory, namespaceString);			
					if(daemonConfig.exists() && daemonConfig.canRead() && daemonConfig.canWrite()){
						return daemonConfig;
					}
				}
			}
		}
		*/
		
		return
				new File(
						new File(globalUserDir, ".kq"),
						Daemon.class.getName().substring(0,  Daemon.class.getName().lastIndexOf("."))
						)
		;
		
	}
	
	
	public boolean saveConfigToCurrentFile() throws Exception{
		if(hadUnsavedChanges()){
			File fileToSave = configFile;
			if(configFile == null){
				fileToSave = getDefaultConfigFile(true);
			}
			else{
				fileToSave = configFile;
			}
			
			if(fileToSave != null){
				FileOutputStream daemonConfigFileOutputStream = new FileOutputStream(fileToSave);
				propertiesLocal.setProperty(keyLocalLastUpdate, DateFormat.getDateTimeInstance().format(new Date()));
				propertiesLocal.store(daemonConfigFileOutputStream, "No Comment!");
				daemonConfigFileOutputStream.close();
				configFile = fileToSave;
				
			}
		}
		
		return false;
	}
	

	public boolean loadConfigFromSpecificFile(File daemonConfig) throws Exception{
		if(!daemonConfig.exists()) return false;
		FileInputStream daemonConfigFileInputStream = new FileInputStream(daemonConfig);
		Properties local = new Properties();
		local.load(daemonConfigFileInputStream);
		daemonConfigFileInputStream.close();
		propertiesLocal = local;
		configFile = daemonConfig;
		return true;
	}
	
	public boolean loadConfigFromDefaultFile() throws Exception{
		return loadConfigFromSpecificFile(getDefaultConfigFile(false));
	}
	
	public boolean hadUnsavedChanges(){
		return hasPendingChange;
	}
	
	public String getGlobalWorkingDirectory(){
		return globalPWD;
	}
	
	public String getGlobalClassPath(){
		return globalCP;
	}
	
	public String getGlobalUserDirectory(){
		return globalUserDir;
	}	
	
	
	public void changeNetworkUsage(boolean bNetworkUsage){
		String s1 = bNetworkUsage?"true":"false";
		if(!(propertiesLocal.containsKey(keyLocalNetworkUsage) && propertiesLocal.getProperty(keyLocalNetworkUsage).equals(s1))){
			propertiesLocal.setProperty(keyLocalNetworkUsage, s1);
			hasPendingChange = true;
		}
	}
	
	public boolean getLocalNetworkUsage(){
		return propertiesLocal.containsKey(keyLocalNetworkUsage)?propertiesLocal.getProperty(keyLocalNetworkUsage).equals("true"): false;
	}
	
	public String getCurrentConfigFilePath(){
		try{
			if(configFile != null) return configFile.getCanonicalPath();
			if(defaultConfigFile != null) return defaultConfigFile.getCanonicalPath();
		}
		catch(Exception e){
			e.printStackTrace();
		}
		return null;
	}
	
}