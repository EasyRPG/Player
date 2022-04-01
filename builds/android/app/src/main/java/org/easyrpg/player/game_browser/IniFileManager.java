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

import android.content.Context;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import org.easyrpg.player.R;

import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.ArrayList;

/**
 * Reads the encoding from the RPG_RT.ini file
 */
public class IniFileManager {
    private final ArrayList<String> lines = new ArrayList<>();
	private final Uri iniFile;
	private final static String EASYRPG_TAG = "[EasyRPG]";

	/**
	 * Reads the encoding from the RPG_RT.ini file
	 */
	public IniFileManager(Context context, Uri iniFileURI) {
	    this.iniFile = iniFileURI;

	    BufferedReader br;
		try {
            InputStream inputStream = context.getContentResolver().openInputStream(iniFileURI);
		    br = new BufferedReader(new InputStreamReader(inputStream));
			String line;
			while ((line = br.readLine()) != null) {
				lines.add(line);
			}
            br.close();
		} catch (Exception e){
            Log.e("EasyRPG", "Failed query: " + e);
        }
	}

	/**
	 * Gets the encoding of the file.
	 *
	 * @return encoding
	 */
	public Encoding getEncoding() {
        boolean easyRPGTagHasBeenFound = false;

        // Analysing the .ini content
		for (String line : lines) {
            String trimmedLine = line.trim();
            // We only take into account what is after "[EasyRPG]"
		    if (!easyRPGTagHasBeenFound && trimmedLine.equalsIgnoreCase(EASYRPG_TAG)) {
				easyRPGTagHasBeenFound = true;
				continue;
			}

		    // We found the tag, let's analysing following lines
			if (easyRPGTagHasBeenFound) {
				if (trimmedLine.startsWith("[")) {
					easyRPGTagHasBeenFound = false;
				} else if (trimmedLine.startsWith("encoding=")) {
					String[] entry = line.toLowerCase().split("=", 2);
					if (entry.length > 1) {
						return Encoding.regionCodeToEnum(entry[1]);
					}
				}
			}
		}
		return Encoding.AUTO;
	}

	public void setEncoding(Context context, Encoding encoding) {
        boolean easyRPGTagHasBeenFound = false;
        boolean encodingWritten = false;
        String lineToAdd="encoding="+encoding.regionCode;

        String line;

	    for (int i = 0; i < lines.size(); i++){
	        line = lines.get(i).trim();

	        // We only take into account what is after "[EasyRPG]"
            if (!easyRPGTagHasBeenFound && line.equalsIgnoreCase("[EasyRPG]")) {
                easyRPGTagHasBeenFound = true;
                continue;
            }

            // We found the tag, let's analysing following lines
            if (easyRPGTagHasBeenFound) {
                // We found the EasyRPG's tag but not the encoding parameter, let's add it
                if (line.startsWith("[")) {
                    easyRPGTagHasBeenFound = false;
                    this.lines.add(i, lineToAdd);
                    encodingWritten = true;
                    break;
                }

                // We found the tag and didn't finished the file, let's continue
                else if (line.startsWith("encoding=")) {
                    // Encoding line found! Let's modify the line
                    this.lines.set(i, lineToAdd);
                    encodingWritten = true;
                }
            }
        }

	    // If we didn't find the EasyRPG's tag, let's add it
        if (!easyRPGTagHasBeenFound) {
            this.lines.add(EASYRPG_TAG);
        }

        // If we didn't find the encoding tag, let's add it
        if (!encodingWritten) {
            this.lines.add(lineToAdd);
        }

        // Save the file
        writeFile(context);
    }

    private void writeFile(Context context) {
        // TODO : Handle the case where the file didn't exist (create a ini file)
        StringBuilder sb = new StringBuilder();
        for (String s : lines) {
            sb.append(s);
            sb.append("\n");
        }

	    try {
            ParcelFileDescriptor pfd = context.getContentResolver().
                openFileDescriptor(iniFile, "w");
            FileOutputStream fileOutputStream =
                new FileOutputStream(pfd.getFileDescriptor());
	        OutputStreamWriter outputStreamWriter = new OutputStreamWriter(fileOutputStream);
            outputStreamWriter.write(sb.toString());
            outputStreamWriter.close();
        }
        catch (IOException e) {
            Log.e("Exception", "File write failed: " + e);
        }
    }

    public enum Encoding {
        AUTO(R.string.autodetect, "auto")
        , WEST_EUROPE(R.string.west_europe, "1252")
        , CENTRAL_EASTERN_EUROPE(R.string.east_europe, "1250")
        , JAPANESE(R.string.japan, "932")
        , CYRILLIC(R.string.cyrillic, "1251")
        , KOREAN(R.string.korean, "949")
        , CHINESE_SIMPLE(R.string.chinese_simple, "936")
        , CHINESE_TRADITIONAL(R.string.chinese_traditional, "950")
        , GREEK(R.string.greek, "1253")
        , TURKISH(R.string.turkish, "1254")
        , BALTIC(R.string.baltic, "1257");

        private final int descriptionStringID;
        private final String regionCode;

        Encoding(int descriptionStringID, String regionCode) {
            this.descriptionStringID = descriptionStringID;
            this.regionCode = regionCode;
        }

        public int getIndex(){
            return this.ordinal();
        }

        public Encoding getEncodingByIndex(int i){
            return Encoding.values()[i];
        }

        public static Encoding regionCodeToEnum(String string) {
            String s = string.trim();

            for (Encoding encoding : Encoding.values()) {
                if (encoding.regionCode.equals(s)) {
                    return encoding;
                }
            }
            return AUTO;
        }

        public static String[] getEncodingDescriptions(Context context){
            Encoding[] encodings = Encoding.values();
            String[] array = new String[encodings.length];
            for (int i = 0; i < encodings.length; i++) {
                array[i] = context.getString(encodings[i].descriptionStringID);
            }
            return array;
        }

        public String getRegionCode() {
            return regionCode;
        }
    }
}
