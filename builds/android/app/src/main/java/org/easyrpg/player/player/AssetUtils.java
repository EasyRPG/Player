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

package org.easyrpg.player.player;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.content.res.AssetManager;
import android.os.Environment;
import android.util.Log;

// based on https://stackoverflow.com/q/15574983/

public class AssetUtils {
	public static void copyFolder(AssetManager assetManager, String source, String target) {
		// "Name" is the name of your folder!
		String[] files = null;

		String state = Environment.getExternalStorageState();

		if (Environment.MEDIA_MOUNTED.equals(state)) {
			// We can read and write the media
			// Checking file on assets subfolder
			try {
				files = assetManager.list(source);
			} catch (IOException e) {
				Log.e("ERROR", "Failed to get asset file list.", e);
			}
			// Analyzing all file on assets subfolder
			for (String filename : files) {
				InputStream in = null;
				OutputStream out = null;
				// First: checking if there is already a target folder
				File folder = new File(target);
				boolean success = true;
				if (!folder.exists()) {
					success = folder.mkdir();
				}
				if (success) {
					// Moving all the files on external SD
					String sourceFile = source + "/" + filename;
					String targetFile = folder.getAbsolutePath() + "/" + filename;
					try {
						in = assetManager.open(sourceFile);
						out = new FileOutputStream(targetFile);
						/*Log.i("WEBVIEW",
								Environment.getExternalStorageDirectory()
										+ "/yourTargetFolder/" + name + "/"
										+ filename);*/
						copyFile(in, out);
						in.close();
						in = null;
						out.flush();
						out.close();
						out = null;
					} catch (IOException e) {
						try {
							assetManager.list(sourceFile);
						} catch (IOException f) {
							Log.e("ERROR",
									"Failed to copy asset file: " + filename, f);
							continue;
						}
						
						copyFolder(assetManager, sourceFile, targetFile);
					}
				} else {
					// Do something else on failure
				}
			}
		} else if (Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)) {
			// We can only read the media
		} else {
			// Something else is wrong. It may be one of many other states, but
			// all we need
			// is to know is we can neither read nor write
		}
	}

	// Method used by copyAssets() on purpose to copy a file.
	private static void copyFile(InputStream in, OutputStream out) throws IOException {
		byte[] buffer = new byte[1024];
		int read;
		while ((read = in.read(buffer)) != -1) {
			out.write(buffer, 0, read);
		}
	}
	
	public static boolean exists(AssetManager assetManager, String source) {
		try {
			String[] s = assetManager.list(source);
			if (s == null || s.length == 0) {
				return false;
			}
			
			return true;
		} catch (IOException e) {
			return false;
		}
	}
}
