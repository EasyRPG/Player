package org.easyrpg.player;

import java.io.File;
import java.io.IOException;

import org.easyrpg.player.game_browser.GameBrowserActivity;
import org.easyrpg.player.game_browser.LegacyGameBrowserActivity;
import org.easyrpg.player.player.AssetUtils;
import org.easyrpg.player.player.EasyRpgPlayerActivity;

import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;

public class MainActivity extends Activity {
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		SettingsActivity.updateUserPreferences(this);
		
		prepareData();
		
		//If the app is called in a game folder, start the game
		startGameStandalone();
		
		//Else : launch the gamebrowser activity
		launchProperBrowser();
	}

	/**
	 * Copies required runtime data from assets folder to data directory
	 */
	public void prepareData() {
		AssetManager assetManager = getAssets();
		String dataDir = getApplication().getApplicationInfo().dataDir;
		
		// Copy timidity to data folder
		if (AssetUtils.exists(assetManager, "timidity")) {
			if (!(new File(dataDir + "/timidity").exists())) {
				AssetUtils.copyFolder(assetManager, "timidity", dataDir + "/timidity");
			}
		}
		
		// Create the easyrpg's directories if they don't exist
		Helper.createEasyRPGDirectories(SettingsActivity.DIRECTORY);
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
	 * Launch the proper game browser depending on the API.
	 */
	private void launchProperBrowser() {
		Intent intent;
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB_MR1) {
			intent = new Intent(this, GameBrowserActivity.class);
		} else {
			intent = new Intent(this, LegacyGameBrowserActivity.class);
		}
		startActivity(intent);
	}
}
