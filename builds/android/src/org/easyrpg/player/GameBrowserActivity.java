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

		setContentView(R.layout.game_browser);

		path = Environment.getExternalStorageDirectory().getPath() + "/easyrpg/games";
		setTitle("Select RPG Maker 2000/2003 Game");

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
			
			if (encoding.equals("1252")) {
				rb = (RadioButton)findViewById(R.id.rd_west);
			} else if (encoding.equals("1251")) {
				rb = (RadioButton)findViewById(R.id.rd_east);
			} else if (encoding.equals("932")) {
				rb = (RadioButton)findViewById(R.id.rd_jp);
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
				
				if (((RadioButton)findViewById(R.id.rd_west)).isChecked()) {
					encoding = "1252";
				} else if (((RadioButton)findViewById(R.id.rd_east)).isChecked()) {
					encoding = "1251";
				}  else if (((RadioButton)findViewById(R.id.rd_jp)).isChecked()) {
					encoding = "932";
				}
				
				if (encoding != null) {
					iniReader.setEncoding(encoding);
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
