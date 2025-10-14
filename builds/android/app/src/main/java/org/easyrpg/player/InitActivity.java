package org.easyrpg.player;

import android.content.Intent;
import android.os.Bundle;
import java.io.File;

import org.easyrpg.player.game_browser.Game;
import org.easyrpg.player.game_browser.GameBrowserHelper;
import org.easyrpg.player.game_browser.ProjectType;

public class InitActivity extends BaseActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Path to the game directory
        // The game data is expected to be in /data/data/<package>/files/SS_Densetsu
        File gameDir = new File(getFilesDir(), "SS_Densetsu");
        File rpgRtIni = new File(gameDir, "RPG_RT.ini");

        if (gameDir.exists() && gameDir.isDirectory() && rpgRtIni.exists()) {
            // Game data exists, launch the game
            String gamePath = gameDir.getAbsolutePath();
            String saveDir = getExternalFilesDir(null).getAbsolutePath() + "/Save";
            new File(saveDir).mkdirs();

            Game project = new Game(gamePath, saveDir, null, ProjectType.SUPPORTED.ordinal());
            GameBrowserHelper.launchGame(this, project);
            finish();
        } else {
            // Game data not found, launch DownloadActivity
            Intent intent = new Intent(this, DownloadActivity.class);
            startActivity(intent);
            finish();
        }
    }
}
