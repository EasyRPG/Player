/*
 * This file is part of EasyRPG Player
 *
 * Copyright (c) 2016 EasyRPG Project. All rights reserved.
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

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.ArrayAdapter;
import android.widget.ListAdapter;
import android.widget.ListView;

import org.easyrpg.player.R;
import org.easyrpg.player.settings.SettingsMainActivity;

/**
 * Game browser for API < 12
 */
@TargetApi(10)
public class GameBrowserActivityAPI10 extends Activity {
	private String path;
	private ListAdapter adapter;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		//Scan the folder
		displayGameList(this);
		
		// First launch : display the "how to use" dialog box	
		GameBrowserHelper.displayHowToMessageOnFirstStartup(this);
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
			displayGameList(this);
			return true;
		case R.id.game_browser_settings:
			Intent intent = new Intent(this, SettingsMainActivity.class);
			startActivity(intent);
			return true;
		case R.id.game_browser_how_to_use_easy_rpg:
			GameBrowserHelper.displayHowToUseEasyRpgDialog(this);
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}
	}

    public void displayGameList(Activity activity){
        // Scan games
        GameScanner gameScanner = GameScanner.getInstance(activity);

        // Populate the list view
        if (gameScanner.hasError()) {
            ArrayAdapter<String> a = new ArrayAdapter<String>(this,
                    android.R.layout.simple_list_item_2, android.R.id.text1,
                    gameScanner.getErrorList());
            adapter = a;
        } else {
            GameListAdapter a = new GameListAdapter(this, gameScanner.getGameList());
            adapter = a;
        }

        //Set the view;
        setContentView(R.layout.game_browser_activity);
        ListView list_view = (ListView)findViewById(R.id.game_browser_list_view);
        list_view.setAdapter(adapter);
    }
}
