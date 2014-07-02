/*
 * This file is part of EasyRPG Player
 *
 * Copyright (c) 2013 EasyRPG Project. All rights reserved.
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

package org.easyrpg.player;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import android.app.Dialog;
import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.Toast;

/**
 * Game browser for EasyRPG Player
 */
public class GameBrowserActivity extends ListActivity {
	private String path;
	
	private boolean error = true;

	final String DATABASE_NAME = "RPG_RT.ldb";
	final String TREEMAP_NAME = "RPG_RT.lmt";
	final String INI_FILE = "RPG_RT.ini";

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		prepareData();
		startGameStandalone();

		setContentView(R.layout.game_browser);

		path = Environment.getExternalStorageDirectory().getPath() + "/easyrpg/games";
		//setTitle("Select RPG Maker 2000/2003 Game");

		List<String> values = new ArrayList<String>();
		
		String state = Environment.getExternalStorageState();
		if (Environment.MEDIA_MOUNTED.equals(state)) {
		
			File dir = new File(path);
			if (!dir.exists()) {
				if (!dir.mkdirs()) {
					values.add("Creating " + path + " directory failed");
				}
			}
	
			if (!dir.canRead() || !dir.isDirectory()) {
				values.add(path + " not readable");
			} else {
	
				File[] list = dir.listFiles();
				if (list != null) {
					for (File file : list) {
						if (!file.getName().startsWith(".") && isRpg2kGame(file)) {
							values.add(file.getName());
						}
					}
				}
	
				if (values.size() == 0) {
					values.add("No games found in " + path);
				} else {
					error = false;
				}
			}
		} else {
			values.add("No external storage (e.g. SD card) found");
		}

		Collections.sort(values);

		// Put the data into the list
		ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
				android.R.layout.simple_list_item_2, android.R.id.text1, values);
		setListAdapter(adapter);
		
		// Setup long click listener
		ListView lv = getListView();
		lv.setOnItemLongClickListener(new OnLongClickListener(this));
	}
	
	/**
	 * Copies required runtime data from assets folder to data directory
	 */
	private void prepareData() {
		AssetManager assetManager = getAssets();
		String dataDir = getApplication().getApplicationInfo().dataDir;
		
		// Copy timidity to data folder
		if (!(new File(dataDir + "/timidity").exists())) {
			AssetUtils.copyFolder(assetManager, "timidity", dataDir + "/timidity");
		}
	}
	
	/**
	 * Standalone Mode:
	 * If there is a game folder in assets that folder is copied to the data
	 * folder and executed.
	 */
	private void startGameStandalone() {
		AssetManager assetManager = getAssets();
		String dataDir = getApplication().getApplicationInfo().dataDir;
		
		// Standalone mode: Copy game in game folder to data folder and launch it
		if (AssetUtils.exists(assetManager, "game")) {
			// Copy game and start directly
			if (!(new File(dataDir + "/game").exists())) {
				AssetUtils.copyFolder(assetManager, "game", dataDir + "/game");
			}
			
			Intent intent = new Intent(this, EasyRpgPlayerActivity.class);
			// Path of game passed to PlayerActivity via intent "project_path"
			intent.putExtra("project_path", dataDir + "/game");
			startActivity(intent);
			finish();
		}
	}
	
	/**
	 * Returns name of game folder at index.
	 * 
	 * @param index list index
	 * @return game folder name
	 */
	public String getGameAt(int index) {
		return (String) getListAdapter().getItem(index);
	}
	
	/**
	 * Returns path of game folder at index.
	 * 
	 * @param index list index
	 * @return game path
	 */
	public String getGamepathAt(int index) {
		return path + "/" + getGameAt(index);
	}
	
	/**
	 * Returns Ini File of game at index.
	 * 
	 * @param index list index
	 * @return ini
	 */
	public File getIniOfGameAt(int index) {
		File dir = new File(getGamepathAt(index));
		
		if (!dir.isDirectory() || !dir.canRead()) {
			return null;
		}

		for (File entry : dir.listFiles()) {
			if (entry.isFile() && entry.canRead()) {
				if (entry.getName().equalsIgnoreCase(INI_FILE)) {
					return entry;
				}
			}
		}

		return null;
	}

	/**
	 * Tests if a folder is a RPG2k Game.
	 * (contains DATABASE_NAME and TREEMAP_NAME)
	 * @param dir Directory to test
	 * @return true if RPG2k game
	 */
	private boolean isRpg2kGame(File dir) {
		if (!dir.isDirectory() || !dir.canRead()) {
			return false;
		}

		boolean databaseFound = false;
		boolean treemapFound = false;
		boolean iniFound = false;

		for (File entry : dir.listFiles()) {
			if (entry.isFile() && entry.canRead()) {
				if (!databaseFound && entry.getName().equalsIgnoreCase(DATABASE_NAME)) {
					databaseFound = true;
				} else if (!treemapFound && entry.getName().equalsIgnoreCase(TREEMAP_NAME)) {
					treemapFound = true;
				} else if (!iniFound && entry.getName().equalsIgnoreCase(INI_FILE)) {
					iniFound = true;
				}
				if (databaseFound && treemapFound && iniFound) {
					return true;
				}
			}
		}

		return false;
	}

	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		if (error) {
			Toast.makeText(this, "Resolve errors and try again",
					Toast.LENGTH_LONG).show();
			return;
		}
		String filename = getGamepathAt(position);
		// Test again in case somebody messed with the file system
		if (isRpg2kGame(new File(filename))) {
			Intent intent = new Intent(this, EasyRpgPlayerActivity.class);
			// Path of game passed to PlayerActivity via intent "project_path"
			intent.putExtra("project_path", filename);
			startActivity(intent);
		} else {
			Toast.makeText(this, getGameAt(position) + " is not a valid game",
					Toast.LENGTH_LONG).show();
		}
	}
	
	private class OnLongClickListener implements OnItemLongClickListener {
		private GameBrowserActivity parent;
		
		public OnLongClickListener(GameBrowserActivity activity) {
			this.parent = activity;
		}

		@Override
		public boolean onItemLongClick(AdapterView<?> arg0, View arg1,
				int row, long arg3) {
			File iniFile = parent.getIniOfGameAt(row);
			try {
				Dialog dialog = new RegionDialog(parent, iniFile);
				dialog.show();
			} catch (IOException e) {
				Toast.makeText(parent, "Accessing configuration of " + getGameAt(row) + " failed.",
						Toast.LENGTH_LONG).show();
			}
			return false;
		}
	}
	
	/**
	 * Used to select the Region (Codepage) of a game.
	 */
	private class RegionDialog extends Dialog {
		private File iniFile;
		private SimpleIniEncodingReader iniReader;
		private RadioGroup rg;

		public RegionDialog(Context context, File iniFile) throws IOException {
			super(context);
			this.iniFile = iniFile;
		    setContentView(R.layout.region_menu);
			setTitle("Select Game Region");
			setCancelable(true);
			
			rg = (RadioGroup)findViewById(R.id.rg);
			
			Button cancel = (Button)findViewById(R.id.rd_region_cancel);
			cancel.setOnClickListener(new RegionOnCloseClickListener(this));

			OpenIni();
			
			// Connect after OpenIni to prevent signal by toggle function
			rg.setOnCheckedChangeListener(new RadioGroupOnClickListener(this, context));
		}

		private void OpenIni() throws IOException {
			iniReader = new SimpleIniEncodingReader(iniFile);
			String encoding = iniReader.getEncoding();
			RadioButton rb = null;
						
			if (encoding == null) {
				rb = (RadioButton)findViewById(R.id.rd_autodetect);
			} else if (encoding.equals("1252")) {
				rb = (RadioButton)findViewById(R.id.rd_west);
			} else if (encoding.equals("1250")) {
				rb = (RadioButton)findViewById(R.id.rd_east);
			} else if (encoding.equals("932")) {
				rb = (RadioButton)findViewById(R.id.rd_jp);
			} else if (encoding.equals("1251")) {
				rb = (RadioButton)findViewById(R.id.rd_cyrillic);
			} else if (encoding.equals("949")) {
				rb = (RadioButton)findViewById(R.id.rd_korean);
			} else if (encoding.equals("936")) {
				rb = (RadioButton)findViewById(R.id.rd_chinese_simple);
			} else if (encoding.equals("950")) {
				rb = (RadioButton)findViewById(R.id.rd_chinese_traditional);
			} else if (encoding.equals("1253")) {
				rb = (RadioButton)findViewById(R.id.rd_greek);
			} else if (encoding.equals("1254")) {
				rb = (RadioButton)findViewById(R.id.rd_turkish);
			} else if (encoding.equals("1257")) {
				rb = (RadioButton)findViewById(R.id.rd_baltic);
			}
			
			if (rb != null) {
				rb.toggle();
			} else {
				Toast.makeText(getContext(), "Unknown region",
						Toast.LENGTH_LONG).show();
			}
		}
		
		private class RadioGroupOnClickListener implements OnCheckedChangeListener {
			Dialog parent;
			Context context;
			
			public RadioGroupOnClickListener(Dialog parent, Context context) {
				this.parent = parent;
				this.context = context;
			}
			
			@Override
			public void onCheckedChanged(RadioGroup group, int checkedId) {
				RadioButton v = (RadioButton)parent.findViewById(checkedId);
				parent.dismiss();
				
				String encoding = null;
				
				if (((RadioButton)findViewById(R.id.rd_autodetect)).isChecked()) {
					encoding = "auto";
				} else if (((RadioButton)findViewById(R.id.rd_west)).isChecked()) {
					encoding = "1252";
				} else if (((RadioButton)findViewById(R.id.rd_east)).isChecked()) {
					encoding = "1250";
				} else if (((RadioButton)findViewById(R.id.rd_jp)).isChecked()) {
					encoding = "932";
				} else if (((RadioButton)findViewById(R.id.rd_cyrillic)).isChecked()) {
					encoding = "1251";
				} else if (((RadioButton)findViewById(R.id.rd_korean)).isChecked()) {
					encoding = "949";
				} else if (((RadioButton)findViewById(R.id.rd_chinese_simple)).isChecked()) {
					encoding = "936";
				} else if (((RadioButton)findViewById(R.id.rd_chinese_traditional)).isChecked()) {
					encoding = "950";
				} else if (((RadioButton)findViewById(R.id.rd_greek)).isChecked()) {
					encoding = "1253";
				} else if (((RadioButton)findViewById(R.id.rd_turkish)).isChecked()) {
					encoding = "1254";
				} else if (((RadioButton)findViewById(R.id.rd_baltic)).isChecked()) {
					encoding = "1257";
				}
				
				if (encoding != null) {
					if (encoding.equals("auto")) {
						iniReader.deleteEncoding();
					} else {
						iniReader.setEncoding(encoding);
					}
					try {
						iniReader.save();
						Toast.makeText(context, "Region changed to " + v.getText().toString(),
								Toast.LENGTH_LONG).show();
					} catch (IOException e) {
						Toast.makeText(context, "Changing region failed",
								Toast.LENGTH_LONG).show();
					}
				}
			}
		}
		
		private class RegionOnCloseClickListener implements View.OnClickListener {
			Dialog parent;
			
			public RegionOnCloseClickListener(Dialog parent) {
				this.parent = parent;
			}
			
			@Override
			public void onClick(View v) {
				parent.dismiss();
			}
		}
	}
}
