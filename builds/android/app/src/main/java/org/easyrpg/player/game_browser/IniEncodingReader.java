/*
 * This file is part of EasyRPG Player
 *
 * Copyright (c) 2017 EasyRPG Project. All rights reserved.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 */

package org.easyrpg.player.game_browser;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.ArrayList;

/**
 * Reads the encoding from the RPG_RT.ini file
 */
public class IniEncodingReader {
	private ArrayList<String> lines = new ArrayList<String>();
	private File iniFile;
	
	/**
	 * Reads the encoding from the RPG_RT.ini file
	 * 
	 * @param iniFile Ini file
	 * @throws IOException On any IO error
	 */
	public IniEncodingReader(File iniFile) throws IOException {
		this.iniFile = iniFile;
		BufferedReader br = null;
		try {
			br = new BufferedReader(new InputStreamReader(new FileInputStream(iniFile)));
			String line;
			while ((line = br.readLine()) != null) {
				//Log.v("Ini", line);
				lines.add(line);
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
				//Log.v("Ini", line);
				if (!line.isEmpty() && line.trim().charAt(0) == '[') {
					sectionFound = false;
				} else {
					//Log.v("Ini", line);
					String[] entry = line.toLowerCase().split("=", 2);
					if (entry[0].trim().equals("encoding")) {
						//Log.v("Ini", "Enc" + entry[1]);
						return entry[1];
					}
				}
			}
		}
		//Log.v("Ini", "Enc1252");
		return null;
	}
}
