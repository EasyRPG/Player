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

import org.easyrpg.player.virtual_buttons.ButtonMappingActivity;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
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
	//Files' names
	final String DATABASE_NAME = "RPG_RT.ldb";
	final String TREEMAP_NAME = "RPG_RT.lmt";
	final String INI_FILE = "RPG_RT.ini";
	
	private String path;
	private boolean error = true;

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
					String msg = getString(R.string.creating_dir_failed).replace("$PATH", path);
					values.add(msg);
				}
			}
	
			if (!dir.canRead() || !dir.isDirectory()) {
				String msg = getString(R.string.path_not_readable).replace("$PATH", path);
				values.add(msg);
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
					String no_game_found = getString(R.string.no_games_found).replace("$PATH", path);
					values.add(no_game_found);
				} else {
					error = false;
				}
			}
		} else {
			values.add(getString(R.string.no_external_storage));
		}

		Collections.sort(values);

		// Put the data into the list
		ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
				android.R.layout.simple_list_item_2, android.R.id.text1, values);
		setListAdapter(adapter);
		
		// Setup long click listener
		ListView lv = getListView();
		lv.setOnItemLongClickListener(new OnLongClickListener(this));
		
		// First launch
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
		boolean first_launch = preferences.getBoolean("FIRST_LAUNCH", true);
		if(first_launch){
			// Displaying the "how to use" dialog box	
			displayHowToUseEasyRpgDialog();
			
			// Set FIRST_LAUNCH to false
			SharedPreferences.Editor editor = preferences.edit();
			editor.putBoolean("FIRST_LAUNCH", false);
			editor.commit();
		}	
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.game_browser_menu, menu);
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle item selection
		switch (item.getItemId()) {
		case R.id.game_browser_menu_change_mapping:
			Intent intent = new Intent(this, ButtonMappingActivity.class);
		    startActivity(intent);
			return true;
		case R.id.game_browser_how_to_use_easy_rpg:
			displayHowToUseEasyRpgDialog();
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
	}

	/**
	 * Prepare and display the dialog box explaining how to use EasyRPG
	 */
	public void displayHowToUseEasyRpgDialog(){
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		
		// Dialog construction
		builder.setMessage(R.string.how_to_use_easy_rpg_explanation)
			.setTitle(R.string.how_to_use_easy_rpg)
			.setNeutralButton(R.string.ok, null);

		builder.create();
		builder.show();
	}
	
	/**
	 * Copies required runtime data from assets folder to data directory
	 */
	private void prepareData() {
		AssetManager assetManager = getAssets();
		String dataDir = getApplication().getApplicationInfo().dataDir;
		
		// Copy timidity to data folder
		if (AssetUtils.exists(assetManager, "timidity")) {
			if (!(new File(dataDir + "/timidity").exists())) {
				AssetUtils.copyFolder(assetManager, "timidity", dataDir + "/timidity");
			}
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
			finish();
			startActivity(intent);
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
	 * Optionally creates it.
	 * 
	 * @param index list index
	 * @param create create ini if not found
	 * @return ini
	 */
	public File getIniOfGameAt(int index, boolean create) {
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
		
		if (create) {
			File newIni = new File(dir.getAbsolutePath() + "/RPG_RT.ini");
			
			try {
				newIni.createNewFile();
			} catch (IOException e) {
				return null;
			}
			
			return newIni;
		}

		return null;
	}
	
	/**
	 * Returns Ini File of game at index.
	 * 
	 * @param index list index
	 * @return ini
	 */
	public File getIniOfGameAt(int index) {
		return getIniOfGameAt(index, false);
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

		for (File entry : dir.listFiles()) {
			if (entry.isFile() && entry.canRead()) {
				if (!databaseFound && entry.getName().equalsIgnoreCase(DATABASE_NAME)) {
					databaseFound = true;
				} else if (!treemapFound && entry.getName().equalsIgnoreCase(TREEMAP_NAME)) {
					treemapFound = true;
				}

				if (databaseFound && treemapFound) {
					return true;
				}
			}
		}

		return false;
	}

	@Override
	protected void onListItemClick(ListView l, View v, int position, long id) {
		if (error) {
			Toast.makeText(this, getString(R.string.resolve_errors),
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
			String msg = getString(R.string.not_valid_game).replace("$PATH", getGameAt(position));
			Toast.makeText(this, msg, Toast.LENGTH_LONG).show();
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
			File iniFile = parent.getIniOfGameAt(row, true);

			String error_msg = getString(R.string.accessing_configuration_failed).replace("$PATH", getGameAt(row));
			
			if (iniFile == null) {
				Toast.makeText(parent, error_msg, Toast.LENGTH_LONG).show();
				return false;
			}
			
			try {
				Dialog dialog = new RegionDialog(parent, iniFile);
				dialog.show();
			} catch (IOException e) {
				Toast.makeText(parent, error_msg, Toast.LENGTH_LONG).show();
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
			setTitle(getString(R.string.select_game_region));
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
				Toast.makeText(getContext(), getString(R.string.unknown_region),
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
						String msg = getString(R.string.region_modification_success).replace("$NAME", v.getText().toString());
						Toast.makeText(context, msg, Toast.LENGTH_LONG).show();
					} catch (IOException e) {
						Toast.makeText(context, getString(R.string.region_modification_failed),
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
