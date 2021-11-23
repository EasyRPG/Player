package org.easyrpg.player.settings;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;
import androidx.documentfile.provider.DocumentFile;

import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;

import org.easyrpg.player.Helper;
import org.easyrpg.player.R;
import org.easyrpg.player.game_browser.GameBrowserActivity;
import org.easyrpg.player.game_browser.GameBrowserHelper;

public class SettingsGamesFolderActivity extends AppCompatActivity {
    private LinearLayout gamesFoldersListLayout;
    private Button setGamesFolderButton;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.activity_settings_games_folders);

        SettingsManager.init(getApplicationContext());

        Activity thisActivity = this;
        setGamesFolderButton = (Button) findViewById(R.id.set_games_folder);
        setGamesFolderButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                GameBrowserHelper.pickAGamesFolder(thisActivity);
            }
        });
    }

    /** Called when the user has chosen a game folder */
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent resultData) {
        super.onActivityResult(requestCode, resultCode, resultData);

        GameBrowserHelper.dealAfterFolderSelected(this, requestCode, resultCode, resultData);

        // <!> Important <!>
        // We directly start a new GameBrowser activity because the folder permission seems the be
        // active just for new activities
        // If the player choose a folder through SAF and then go back to Gamebrowser by clicking
        // back button, the GameBrowser don't have the permission to read the directory
        GameBrowserActivity.resetGamesList();
        Intent intent;
        intent = new Intent(this, GameBrowserActivity.class);
        startActivity(intent);
    }
}
