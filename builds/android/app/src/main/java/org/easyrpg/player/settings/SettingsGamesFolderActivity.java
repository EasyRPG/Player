package org.easyrpg.player.settings;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.provider.DocumentsContract;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import org.easyrpg.player.BaseActivity;
import org.easyrpg.player.Helper;
import org.easyrpg.player.R;
import org.easyrpg.player.game_browser.GameBrowserActivity;
import org.easyrpg.player.game_browser.GameBrowserHelper;
import org.libsdl.app.SDL;

public class SettingsGamesFolderActivity extends BaseActivity {
    private GameBrowserHelper.SafError safError = GameBrowserHelper.SafError.OK;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.activity_settings_easyrpg_folders);
        SDL.setContext(getApplicationContext());

        safError = GameBrowserHelper.SafError.OK;

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
        Helper.attachOpenFolderButton(this, openGameFolderButton, SettingsManager.getGamesFolderURI(this));

        // The "Open RTP Folder" Button
        Button openRTPFolderButton = this.findViewById(R.id.open_rtp_folder);
        Helper.attachOpenFolderButton(this, openRTPFolderButton, SettingsManager.getRTPFolderURI(this));

        // Video button
        findViewById(R.id.watch_video).setOnClickListener(v -> {
            Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(GameBrowserHelper.VIDEO_URL));
            startActivity(browserIntent);
        });

        DetectRtp();
    }

    @Override
    public void onResume() {
        super.onResume();

        if (safError != GameBrowserHelper.SafError.OK && safError != GameBrowserHelper.SafError.ABORTED) {
            GameBrowserHelper.showErrorMessage(this, safError);
            safError = GameBrowserHelper.SafError.OK;
        } else {
            DetectRtp();
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

    private void DetectRtp() {
        Uri rtpUri = SettingsManager.getRTPFolderURI(this);
        if (rtpUri == null) {
            return;
        }

        RtpHitInfo hitInfo = new RtpHitInfo();
        DetectRtp(rtpUri + "/2000", hitInfo, 2000);

        TextView res = findViewById(R.id.settings_rtp_2000_result);
        String rtpText;
        if (hitInfo.version == 2000) {
            rtpText = getApplicationContext().getResources().getString(R.string.rtp_found);
            rtpText = rtpText.replace("$YEAR", "2000")
                .replace("$NAME", hitInfo.name)
                .replace("$FOUND", Integer.toString(hitInfo.hits))
                .replace("$MAX", Integer.toString(hitInfo.max));
        } else {
            rtpText = getApplicationContext().getResources().getString(R.string.rtp_not_found);
            rtpText = rtpText.replace("$YEAR", "2000");
        }
        res.setText(rtpText);

        DetectRtp(rtpUri + "/2003", hitInfo, 2003);

        res = findViewById(R.id.settings_rtp_2003_result);
        if (hitInfo.version == 2003) {
            rtpText = getApplicationContext().getResources().getString(R.string.rtp_found);
            rtpText = rtpText.replace("$YEAR", "2003")
                .replace("$NAME", hitInfo.name)
                .replace("$FOUND", Integer.toString(hitInfo.hits))
                .replace("$MAX", Integer.toString(hitInfo.max));
        } else {
            rtpText = getApplicationContext().getResources().getString(R.string.rtp_not_found);
            rtpText = rtpText.replace("$YEAR", "2003");
        }
        res.setText(rtpText);
    }

    public static class RtpHitInfo {
        String name;
        int version = 0;
        int hits = 0;
        int max = 0;
    };

    private native void DetectRtp(String path, RtpHitInfo hitInfo, int version);
}
