package org.easyrpg.player.settings;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.provider.DocumentsContract;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;

import androidx.appcompat.app.AppCompatActivity;

import org.easyrpg.player.R;
import org.easyrpg.player.game_browser.GameBrowserActivity;
import org.easyrpg.player.game_browser.GameBrowserHelper;

public class SettingsGamesFolderActivity extends AppCompatActivity {
    private GameBrowserHelper.SafError safError = GameBrowserHelper.SafError.OK;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.activity_settings_easyrpg_folders);

        safError = GameBrowserHelper.SafError.OK;

        SettingsManager.init(getApplicationContext());

        Activity thisActivity = this;
        Button setGamesFolderButton = findViewById(R.id.set_games_folder);
        setGamesFolderButton.setOnClickListener(v -> GameBrowserHelper.pickAGamesFolder(thisActivity));

        // Setting UI components
        CheckBox enableRTPScanning = findViewById(R.id.settings_enable_rtp_scanning);
        enableRTPScanning.setChecked(SettingsManager.isRTPScanningEnabled());
        enableRTPScanning.setOnClickListener(view -> SettingsManager.setRTPScanningEnabled(enableRTPScanning.isChecked()));

        // Setup UI components
        // The "Open Game Folder" Button
        Button openGameFolderButton = this.findViewById(R.id.open_game_folder);
        // We can open the file picker in a specific folder only with API >= 26
        if (android.os.Build.VERSION.SDK_INT >= 26) {
            openGameFolderButton.setOnClickListener(v -> {
                // Open the file explorer in the "soundfont" folder
                Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
                intent.setType("*/*");
                intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, SettingsManager.getGamesFolderURI(this));
                startActivity(intent);
            });
        } else {
            ViewGroup layout = (ViewGroup) openGameFolderButton.getParent();
            if(layout != null) {
                layout.removeView(openGameFolderButton);
            }
        }

        // The "Open RTP Folder" Button
        Button openRTPFolderButton = this.findViewById(R.id.open_rtp_folder);
        // We can open the file picker in a specific folder only with API >= 26
        if (android.os.Build.VERSION.SDK_INT >= 26) {
            openRTPFolderButton.setOnClickListener(v -> {
                // Open the file explorer in the "soundfont" folder
                Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
                intent.setType("*/*");
                intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, SettingsManager.getRTPFolderURI(this));
                startActivity(intent);
            });
        } else {
            ViewGroup layout = (ViewGroup) openRTPFolderButton.getParent();
            if(layout != null) {
                layout.removeView(openRTPFolderButton);
            }
        }

        // Video button
        findViewById(R.id.watch_video).setOnClickListener(v -> {
            Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(GameBrowserHelper.VIDEO_URL));
            startActivity(browserIntent);
        });
    }

    @Override
    public void onResume() {
        super.onResume();

        if (safError != GameBrowserHelper.SafError.OK && safError != GameBrowserHelper.SafError.ABORTED) {
            GameBrowserHelper.showErrorMessage(this, safError);
            safError = GameBrowserHelper.SafError.OK;
        }
    }

    /** Called when the user has chosen a game folder */
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent resultData) {
        super.onActivityResult(requestCode, resultCode, resultData);

        safError = GameBrowserHelper.dealAfterFolderSelected(this, requestCode, resultCode, resultData);
        if (safError != GameBrowserHelper.SafError.OK && safError != GameBrowserHelper.SafError.ABORTED) {
            return;
        }

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
