package kq;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Calendar;

import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

public class Portfolio {
	
	public class SymbolInfo{
		String symbolName;
		float shareCount;
		float costPerShare;
		float costOfTransaction;
		Calendar dateOfPurchase;
	}
	
	Map<String, SymbolInfo> symbols;
	
	Portfolio(){
	}
	
	public Iterator<SymbolInfo> getIterator(){
		return symbols.values().iterator();
	}
	
	public static Portfolio parse(InputStream is){
		InputStreamReader isr = new InputStreamReader(is);
		BufferedReader br = new BufferedReader(isr);
		String sRecord;
		Map<String, SymbolInfo> info = new HashMap<String, SymbolInfo>();
		int iLine = 1;
		Portfolio pf = new Portfolio();
		try{
			while((sRecord = br.readLine()) != null){
				if(sRecord.isEmpty()) continue;
				String [] cols = sRecord.split(",");
				if(cols.length == 7){
					SymbolInfo si = pf.new SymbolInfo();
					si.symbolName = cols[0];
					si.shareCount = Float.parseFloat(cols[1]);
					si.costPerShare = Float.parseFloat(cols[2]);
					si.costOfTransaction = Float.parseFloat(cols[3]);
					si.dateOfPurchase = new GregorianCalendar(Integer.parseInt(cols[6]), Integer.parseInt(cols[4]) - 1, Integer.parseInt(cols[5]));
					
					info.put(cols[0], si);
				}
				else{
					System.err.println("Line " + iLine + " malformed while parsing portfolio");
				}
				
				iLine++;
			}
		}
		catch(IOException io){
			io.printStackTrace();
		}
		if(info.isEmpty()){
			return null;
		}
		
		pf.symbols = info;
		return pf;
	}
}
