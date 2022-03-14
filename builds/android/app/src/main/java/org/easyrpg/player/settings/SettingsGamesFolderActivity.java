package org.easyrpg.player.settings;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.widget.CheckBox;

import androidx.appcompat.app.AppCompatActivity;

import org.easyrpg.player.R;
import org.easyrpg.player.game_browser.GameBrowserActivity;
import org.easyrpg.player.game_browser.GameBrowserHelper;

public class SettingsGamesFolderActivity extends AppCompatActivity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.activity_settings_easyrpg_folders);

        SettingsManager.init(getApplicationContext());

        Activity thisActivity = this;
        Button setGamesFolderButton = (Button) findViewById(R.id.set_games_folder);
        setGamesFolderButton.setOnClickListener(v -> GameBrowserHelper.pickAGamesFolder(thisActivity));

        // Setting UI components
        CheckBox enableRTPScanning = (CheckBox) findViewById(R.id.settings_enable_rtp_scanning);
        enableRTPScanning.setChecked(SettingsManager.isRTPScanningEnabled());
        enableRTPScanning.setOnClickListener(view -> SettingsManager.setRTPScanningEnabled(enableRTPScanning.isChecked()));

        // TODO : Make the Open RTP folder work
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
