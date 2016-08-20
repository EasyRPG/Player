package org.easyrpg.player;

import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;

import org.easyrpg.player.game_browser.GameBrowserActivity;
import org.easyrpg.player.game_browser.GameBrowserHelper;
import org.easyrpg.player.game_browser.GameInformation;
import org.easyrpg.player.player.AssetUtils;
import org.easyrpg.player.settings.SettingsManager;

import java.io.File;

/**
 * The activity called at launch.
 * Prepare data, launch the standalone mode or the proper gamebrowser (depending on api's version)
 * To start the standalone mode : put your project in assets/games
 * ("game" is the project directory, no sub folder)
 */
public class MainActivity extends Activity {
    private boolean standaloneMode = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        prepareData();

        // if the app is called in a game folder : start the game
        startGameStandalone();

        // else : launch the gamebrowser activity
        if (!standaloneMode) {
            launchProperBrowser();
        }
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
    }

    /**
     * Standalone Mode-> if there is a game folder in assets: that folder is
     * copied to internal memory and executed.
     */
    private void startGameStandalone() {
        AssetManager assetManager = getAssets();
        String dataDir = getApplication().getApplicationInfo().dataDir;

        // Standalone mode: Copy game in game folder to data folder and launch
        // it
        if (AssetUtils.exists(assetManager, "game")) {
            Log.i("EasyRPG", "Standalone mode : a \"game\" folder is present in asset folder");
            standaloneMode = true;

            // Copy game in internal memory
            if (!(new File(dataDir + "/game").exists())) {
                AssetUtils.copyFolder(assetManager, "game", dataDir + "/game");
            }

            // Launch the game
            GameInformation project = new GameInformation(dataDir + "/game");
            GameBrowserHelper.launchGame(this, project);
            finish();
        }
    }

    /**
     * Launch the proper game browser depending on the API.
     */
    private void launchProperBrowser() {
        // Retrieve user's preferences (for application's folder)
        SettingsManager.init(getApplicationContext());

        // Create the easyrpg's directories if they don't exist
        Helper.createEasyRPGDirectories(SettingsManager.getEasyRPGFolder());

        //Launch the proper game browser
        Intent intent;
        intent = new Intent(this, GameBrowserActivity.class);
        startActivity(intent);
    }
}
