package org.easyrpg.player;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

/**
 * Game browser for EasyRPG Player
 */

public class GameBrowserActivity extends ListActivity {
	private String path;
	
	private boolean error = true;

	final String DATABASE_NAME = "RPG_RT.ldb";
	final String TREEMAP_NAME = "RPG_RT.lmt";

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.game_browser);

		path = Environment.getExternalStorageDirectory().getPath() + "/easyrpg/games";
		setTitle("Select RPG Maker 2000/2003 Game");

		List<String> values = new ArrayList<String>();
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
		Collections.sort(values);

		// Put the data into the list
		ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
				android.R.layout.simple_list_item_2, android.R.id.text1, values);
		setListAdapter(adapter);
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
			Toast.makeText(this, "Resolve errors and try again",
					Toast.LENGTH_LONG).show();
			return;
		}
		String filename = (String) getListAdapter().getItem(position);
		filename = path + "/" + filename;
		// Test again in case somebody messed with the file system
		if (isRpg2kGame(new File(filename))) {
			Intent intent = new Intent(this, EasyRpgPlayerActivity.class);
			// Path of game passed to PlayerActivity via intent "project_path"
			intent.putExtra("project_path", filename);
			startActivity(intent);
		} else {
			Toast.makeText(this, filename + " is not a valid game",
					Toast.LENGTH_LONG).show();
		}
	}
}
