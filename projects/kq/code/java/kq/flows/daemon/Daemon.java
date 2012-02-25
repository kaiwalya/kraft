package kq.flows.daemon;

class Daemon{
	
	class DaemonWorker implements Runnable{
		
		public void run(){
		
		}
	}
	
	
	
	Daemon(Configuration p) throws Exception{
		if(!p.loadLocalConfig()){
			String command = "java -cp " + p.getGlobalClassPath() + " " + ControlPanel.class.getCanonicalName();
			System.err.println("Looks like the first run, starting the Control Panel: \"" + command +"\"");
			Runtime.getRuntime().exec(command);
		}
	}
	
	public static void main(String [] arrParams) throws Exception{
		new Daemon(new Configuration(arrParams));
	}
}