package org.easyrpg.player;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.ArrayList;

import android.util.Log;

/**
 * Reads the encoding from the RPG_RT.ini file
 */
public class SimpleIniEncodingReader {
	private ArrayList<String> lines = new ArrayList<String>();
	private File iniFile;
	
	/**
	 * Reads the encoding from the RPG_RT.ini file
	 * 
	 * @param iniFile Ini file
	 * @throws IOException On any IO error
	 */
	public SimpleIniEncodingReader(File iniFile) throws IOException {
		this.iniFile = iniFile;
		BufferedReader br = null;
		try {
			br = new BufferedReader(new InputStreamReader(new FileInputStream(iniFile)));
			String line;
			line = br.readLine();
			while (line != null) {
				Log.v("Ini", line);
				lines.add(line);
				line = br.readLine();
			}
		} finally {
			if (br != null) {
				br.close();
			}
		}
	}
	
	/**
	 * Gets the encoding of the file.
	 * 
	 * @return encoding
	 */
	public String getEncoding() {
		boolean sectionFound = false;
		
		for (String line : lines) {
			if (!sectionFound && line.trim().equalsIgnoreCase("[EasyRPG]")) {
				sectionFound = true;
				continue;
			}
			if (sectionFound) {
				Log.v("Ini", line);
				if (line.trim().charAt(0) == '[') {
					sectionFound = false;
				} else {
					Log.v("Ini", line);
					String[] entry = line.toLowerCase().split("=", 2);
					if (entry[0].trim().equals("encoding")) {
						Log.v("Ini", "Enc" + entry[1]);
						return entry[1];
					}
				}
			}
		}
		Log.v("Ini", "Enc1252");
		return "1252";
	}
	
	/**
	 * Overwrites the current Encoding settings or adds new if missing
	 * 
	 * @param newEncoding new encoding
	 */
	public void setEncoding(String newEncoding) {
		boolean sectionFound = false;
		String newEnc = "Encoding=" + newEncoding;
		
		int index = 0;
		for (String line : lines) {
			if (!sectionFound && line.trim().equalsIgnoreCase("[EasyRPG]")) {
				sectionFound = true;
				continue;
			}
			if (sectionFound) {
				if (line.trim().charAt(0) == '[') {
					// End of section and not found -> insert a new entry
					lines.add(index+1, newEnc);
					return;
				} else {
					String[] entry = line.toLowerCase().split("=", 2);
					if (entry[0].trim().equals("encoding")) {
						lines.set(index+1, newEnc);
						return;
					}
				}
			}
			index++;
		}
		
		// No [EasyRPG] section found
		lines.add("[EasyRPG]");
		lines.add(newEnc);
	}
	
	/**
	 * Saves the changed encoding to the ini file
	 * 
	 * @throws IOException On any IO error
	 */
	public void save() throws IOException {
		BufferedWriter bw = null;
		try {
			bw = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(iniFile)));
			for (String line : lines) {
				bw.write(line + "\r\n");
			}
		}
		finally {
			if (bw != null) {
				bw.close();
			}
		}
	}
}
