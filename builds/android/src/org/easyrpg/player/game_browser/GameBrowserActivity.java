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

package org.easyrpg.player.game_browser;

import java.lang.reflect.Field;
import java.util.LinkedList;

import org.easyrpg.player.R;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.ViewConfiguration;
import android.widget.ArrayAdapter;
import android.widget.ListAdapter;
import android.widget.ListView;

/**
 * Game browser for EasyRPG Player
 */
public class GameBrowserActivity extends Activity {
	private String path;
	
	ListView list_view;
	private ListAdapter adapter;
	LinkedList<ProjectInformation> project_list = new LinkedList<ProjectInformation>();
	LinkedList<String> error_list = new LinkedList<String>();
	
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		// Setting the game list
		setContentView(R.layout.game_browser_activity);
		list_view = (ListView)findViewById(R.id.game_browser_list_view);
		displayGameList();
		makeActionOverflowMenuShown();
		
		// First launch : display the "how to use" dialog box	
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
		inflater.inflate(R.menu.game_browser, menu);
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle item selection
		switch (item.getItemId()) {
		case R.id.game_browser_refresh:
			displayGameList();
			return true;
		case R.id.game_browser_controls_settings:
			Intent intent = new Intent(this, org.easyrpg.player.PreferencesActivity.class);
		    startActivity(intent);
			return true;
		case R.id.game_browser_how_to_use_easy_rpg:
			displayHowToUseEasyRpgDialog();
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
	}

	public void displayGameList(){
		//Scan the folder
		path = Environment.getExternalStorageDirectory().getPath() + "/easyrpg/games";
		GameBrowserHelper.scanGame(this, path, project_list, error_list);
		
		// Put the result into the proper adapter
		if (error_list.size() > 0) {
			//If the game list is empty, we use a simplified adapter
			adapter = new ArrayAdapter<String>(this,
					android.R.layout.simple_list_item_2, android.R.id.text1, error_list);
		} else {
			//If the game list is not empty, we use the proper adapter
			adapter = new GameListAdapter(this, project_list);
		}
		
		//Set the view;
		list_view.setAdapter(adapter);
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
	
	/** This function prevents some Samsung's device to not show the action overflow button
	 *  in the action bar
	 */
	private void makeActionOverflowMenuShown() {
	    //devices with hardware menu button (e.g. Samsung Note) don't show action overflow menu
	    try {
	        ViewConfiguration config = ViewConfiguration.get(this);
	        Field menuKeyField = ViewConfiguration.class.getDeclaredField("sHasPermanentMenuKey");
	        if (menuKeyField != null) {
	            menuKeyField.setAccessible(true);
	            menuKeyField.setBoolean(config, false);
	        }
	    } catch (Exception e) {
	        Log.d("TAG", e.getLocalizedMessage());
	    }
	}
}
