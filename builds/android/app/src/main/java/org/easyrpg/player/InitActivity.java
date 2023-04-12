package org.easyrpg.player;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.widget.Button;

import androidx.appcompat.app.AppCompatActivity;
import androidx.documentfile.provider.DocumentFile;

import org.easyrpg.player.game_browser.Game;
import org.easyrpg.player.game_browser.GameBrowserActivity;
import org.easyrpg.player.game_browser.GameBrowserHelper;
import org.easyrpg.player.player.AssetUtils;
import org.easyrpg.player.settings.SettingsManager;

import java.io.File;

/**
 * The activity called at launch.
 * Prepare data, launch the standalone mode or the proper GameBrowser (depending on api's version)
 * To start the standalone mode : put your project in assets/games
 * ("game" is the project directory, no sub folder)
 */
public class InitActivity extends AppCompatActivity {
    private boolean standaloneMode = false;
    private GameBrowserHelper.SafError safError = GameBrowserHelper.SafError.OK;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_init);

        safError = GameBrowserHelper.SafError.OK;

        // Retrieve User's preferences
        SettingsManager.init(getApplicationContext());

        Activity thisActivity = this;
        (findViewById(R.id.set_games_folder)).setOnClickListener(v -> GameBrowserHelper.pickAGamesFolder(thisActivity));

        // Video button
        findViewById(R.id.watch_video).setOnClickListener(v -> {
            Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(GameBrowserHelper.VIDEO_URL));
            startActivity(browserIntent);
        });

        // If the app is called in a game folder : start the game
        startGameStandalone();
    }

    @Override
    public void onResume() {
        super.onResume();

        if (!standaloneMode) {
            if (safError != GameBrowserHelper.SafError.OK && safError != GameBrowserHelper.SafError.ABORTED) {
                GameBrowserHelper.showErrorMessage(this, safError);
                safError = GameBrowserHelper.SafError.OK;
                return;
            }

            // If we have a readable EasyRPG folder, start the GameBrowser
            Uri easyRPGFolderURI = SettingsManager.getEasyRPGFolderURI(this);
            DocumentFile easyRPGFolder = Helper.getFileFromURI(this, easyRPGFolderURI);
            if (easyRPGFolder != null) {

                // Do we have read/right access to the EasyRPG folder?
                if (easyRPGFolder.canRead() && easyRPGFolder.canWrite()) {
                    launchGamesBrowser();
                }
            }
        }
    }

    /** Called when the user has chosen a game folder */
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent resultData) {
        super.onActivityResult(requestCode, resultCode, resultData);

        safError = GameBrowserHelper.dealAfterFolderSelected(this, requestCode, resultCode, resultData);
    }

    /**
     * Copies required runtime data from assets folder to data directory
     * We don't use it because we dropped Timidity, but it may be useful later
     */
    public void prepareData() {
        AssetManager assetManager = getAssets();
        String dataDir = getApplication().getApplicationInfo().dataDir;
        Log.i("EasyRPG", "The application folder is : " + dataDir);

        /*
        // We leave the timidity part here just to show how loading a library works
        // Copy timidity to data folder
        if (AssetUtils.exists(assetManager, "timidity")) {
            if (!(new File(dataDir + "/timidity").exists())) {
                AssetUtils.copyFolder(assetManager, "timidity", dataDir + "/timidity");
            }
        }
        */
    }

    /**
     * Standalone Mode-> if there is a game folder in assets: that folder is
     * copied to internal memory and executed.
     */
    private void startGameStandalone() {
        AssetManager assetManager = getAssets();
        String gameDir = "";

        // Standalone mode: Launch the game inside "game.zip" in the APK
        if (AssetUtils.fileExists(assetManager, "game.zip")) {
            Log.i("EasyRPG", "Standalone mode : a \"game.zip\" file is present inside the asset folder");
            standaloneMode = true;
            gameDir = "apk://game.zip";
        }

        if (standaloneMode) {
            // Launch the game
            String saveDir = getExternalFilesDir(null).getAbsolutePath() + "/Save";
            new File(saveDir).mkdirs();

            Game project = new Game(gameDir, saveDir);
            GameBrowserHelper.launchGame(this, project);
            finish();
        }
    }

    /**
     * Launch the game browsers
     */
    private void launchGamesBrowser() {
        // Recreate EasyRPG's folders and .nomedia file if necessary
        // TODO : This method might do some unnecessary actions, to verify
        Uri easyRPGFolderURI = SettingsManager.getEasyRPGFolderURI(this);
        Helper.createEasyRPGFolders(this, easyRPGFolderURI);

        // Create storage location for the config.ini
        new File(getExternalFilesDir(null).getAbsolutePath()).mkdirs();

        //Launch the proper game browser
        Intent intent;
        intent = new Intent(this, GameBrowserActivity.class);
        startActivity(intent);
        finish();
    }
}
