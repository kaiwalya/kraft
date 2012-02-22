package kq;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URI;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.Iterator;
import kq.web.*;

public class Stock {
	
	Portfolio pf;
	IHTTPResourceCache cache;
	
	
	Stock(Portfolio pf){
		this.pf = pf;
		cache = new ZipHttpResourceCache("stock");
	}
	
	void analyze(){
		float totalncp = 0;
		float totalnsp = 0;
		float totalnspm = 0;
		Iterator<Portfolio.SymbolInfo> it = pf.getIterator();
		//System.out.println("SYM\tCost\tCurrent\tMax");
		//System.out.println("------------------------------------------");
		while(it.hasNext()){
			Portfolio.SymbolInfo si = it.next();
			//System.out.println("Analysing: " + si.symbolName + " bought on " + si.dateOfPurchase.getTime());
			GregorianCalendar today = new GregorianCalendar();
			
			//http://ichart.finance.yahoo.com/table.csv?s=IAU&a=11&b=23&c=2011&d=00&e=30&f=2012&g=d&ignore=.csv
			String suri = "http://ichart.finance.yahoo.com/table.csv?";
			suri += ("s=" + si.symbolName);
			suri += ("&a=" + si.dateOfPurchase.get(Calendar.MONTH));
			suri += ("&b=" + si.dateOfPurchase.get(Calendar.DATE));
			suri += ("&c=" + si.dateOfPurchase.get(Calendar.YEAR));
			suri += ("&d=" + today.get(Calendar.MONTH));
			suri += ("&e=" + today.get(Calendar.DATE));
			suri += ("&f=" + today.get(Calendar.YEAR));
			suri += "&g=d&ignore=.csv";
			
			//Current price
			//http://download.finance.yahoo.com/d/quotes.csv?s=DFS&f=sl1d1t1c1ohgv&e=.csv
			String suri_latest = "http://download.finance.yahoo.com/d/quotes.csv?s="+ si.symbolName + "&f=sl1d1t1c1ohgv&e=.csv";
			
			URI uri = null;
			URI uri_latest = null;
			
			try{
				uri = new URI(suri);
				uri_latest = new URI(suri_latest);
			}
			catch(Exception e){
				e.printStackTrace();
			}
			
			if(uri != null && uri_latest != null){
				float fCurr = -1;
				String sCurrentDate = null;
				{
					try{
						String [] currentData = (new BufferedReader(new InputStreamReader(cache.getResource(uri_latest, "")))).readLine().split(",");
						fCurr = Float.parseFloat(currentData[1]);
						String [] currentDate = currentData[2].replaceAll("\"", "").split("/");
						sCurrentDate = "" + currentDate[2] + "-" + currentDate[0] + "-" + currentDate[1];
					}
					catch(Exception E){
						E.printStackTrace();
					}
				}
				InputStream is = cache.getResource(uri, null);
				BufferedReader br = new BufferedReader(new InputStreamReader(is));
				String line = null;
				float fMax;
				fMax = -1;
				String sMaxDate = null;
				try {
					line = br.readLine(); //waste one line, thats the column names.
					line = br.readLine(); //Read first record to get the current price
					if(line != null){
						String [] data = line.split(",");
						//fCurr = Float.parseFloat(data[4]);
						//sCurrentDate = data[0];
						fMax = Float.parseFloat(data[2]);
						sMaxDate = data[0];
						while((line = br.readLine()) != null){
							data = line.split(",");
							float fNewMax = Float.parseFloat(data[2]);
							if(fNewMax > fMax){
								fMax = fNewMax;
								sMaxDate = data[0];
							}
							//System.out.println(line);
						}
					}
					
					//n = Number of shares
					float n = si.shareCount;
					
					//cost and sale prices per share without transaction cost
					float cp = si.costPerShare;
					float sp = fCurr;
					float spm = fMax;
					
					//prices with transaction cost
					float ncp = cp * n + si.costOfTransaction;
					float nsp = sp * n - si.costOfTransaction;
					float nspm = spm * n - si.costOfTransaction;
					
					totalncp += ncp;
					totalnsp += nsp;
					totalnspm += nspm;
					
					//prices per share with transction cost
					cp = ncp/n;
					sp = nsp/n;
					spm = nspm/n;
					
					//profit
					float np = nsp - ncp;
					float npm = nspm - ncp;
					
					//profit percent
					float pp = 100.0f * np / ncp;
					float ppm = 100.0f * npm / ncp;
					
					//loss since max
					float whatif_loss = nspm - nsp;
					float whatif_loss_percent = 100.0f * whatif_loss / (nspm - ncp);
					
					String [] maxdateelements = sMaxDate.split("-");
					GregorianCalendar maxdate = new GregorianCalendar(
							Integer.parseInt(maxdateelements[0]),
							Integer.parseInt(maxdateelements[1]) -1,
							Integer.parseInt(maxdateelements[2])
							);
					
					String [] currentdateelements = sCurrentDate.split("-");
					GregorianCalendar scurrentdate = new GregorianCalendar(
							Integer.parseInt(currentdateelements[0]),
							Integer.parseInt(currentdateelements[1]) -1,
							Integer.parseInt(currentdateelements[2])
							);
					
					float dayssincemax = (float)(((double)scurrentdate.getTimeInMillis() - (double)maxdate.getTimeInMillis())/(1000.0*60.0*60.0*24.0));
					
					System.out.print(si.symbolName);
					//System.out.format("\t%.2f,%.2f$", cp, ncp);
					//System.out.format("\t%.2f,%.2f$[%.2f,%.2f$,%.2f]", sp, nsp, sp-cp, nsp-ncp, 100*(nsp-ncp)/ncp);
					//System.out.format("\t%.2f,%.2f$[%.2f,%.2f$,%.2f]", spm, nspm, spm-cp, nspm-ncp, 100*(nspm-ncp)/ncp);
					System.out.format(" profits fell by [%.2f$, %.2f%%] in %d days", whatif_loss, whatif_loss_percent, (int)dayssincemax);
					
					if(sp >= cp)
						System.out.format(" with [%.2f$, %.2f%%] gain", np, pp);
					else
						System.out.format(" with [%.2f$, %.2f%%] loss", -np, -pp);
					
					
					System.out.println();
					//System.out.format("\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n", );
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
		
		System.out.format("\nTotal Gain: [%.2f, %.2f%%], what if gain [%.2f, %.2f%%]\n", totalnsp - totalncp, 100*(totalnsp-totalncp)/totalncp, totalnspm - totalncp, 100*(totalnspm-totalncp)/totalncp);
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		if(args.length < 1){
			System.err.println("First Param needs to be a config file");
			return;
		}
		
		File f = new File(args[0]);
		if(f.isFile() && f.canRead()){
			try {
				FileInputStream is = new FileInputStream(f);
				Portfolio pf = Portfolio.parse(is);
				if(pf != null){
					(new Stock(pf)).analyze();
				}
				else{
					System.err.println("Bad Portfolio, couldnt initialize stock analyzer");
				}
				
			} catch (FileNotFoundException e) {
				
				e.printStackTrace();
			}
		}
	}

}
