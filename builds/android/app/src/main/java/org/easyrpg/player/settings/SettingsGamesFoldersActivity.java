package org.easyrpg.player.settings;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import org.easyrpg.player.DirectoryChooser;
import org.easyrpg.player.R;
import org.easyrpg.player.settings.SettingsManager;

public class SettingsGamesFoldersActivity extends AppCompatActivity implements View.OnClickListener {
    private LinearLayout gamesFoldersListLayout;
    private Button addGameFolderButton;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.activity_settings_games_folders);

        SettingsManager.init(getApplicationContext());

        // Setting UI components
        gamesFoldersListLayout = (LinearLayout) findViewById(R.id.games_folders_list);
        updateGameFoldersList();
        addGameFolderButton = (Button) findViewById(R.id.settings_add_game_folder_button);
        addGameFolderButton.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.settings_add_game_folder_button:
                addAGameFolder();
                break;
        }
    }

    private void updateGameFoldersList() {
        gamesFoldersListLayout.removeAllViews();

        for (String gameDirPath : SettingsManager.getGamesFolderList()) {
            LayoutInflater inflater = LayoutInflater.from(this);
            RelativeLayout layout = (RelativeLayout) inflater.inflate(R.layout.settings_item_list, null);

            // The name
            TextView nameTextView = (TextView) layout.findViewById(R.id.controls_settings_preset_name);
            nameTextView.setText(gameDirPath);
            nameTextView.setTextSize(14);

            // Option button (not present in the default folder)
            final String path = gameDirPath;
            ImageButton remove_button = (ImageButton) layout.findViewById(R.id.controls_settings_preset_option_button);
            remove_button.setImageResource(R.drawable.ic_clear_black_24dp);
            if (path.equals(SettingsManager.getEasyRPGFolder() + "/games")) {
                layout.removeView(remove_button);
            } else {
                remove_button.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        removeAGameFolder(path);
                    }
                });
            }

            gamesFoldersListLayout.addView(layout);
        }
    }

    private void removeAGameFolder(String path) {
        SettingsManager.removeAGameFolder(path);

        // Update UI
        updateGameFoldersList();
    }

    private void addAGameFolder() {
        new DirectoryChooser(this, SettingsManager.getEasyRPGFolder(), new Runnable() {
            @Override
            public void run() {
                String pathToAdd = DirectoryChooser.getSelectedPath();
                SettingsManager.addGameDirectory(pathToAdd);

                // Update UI
                updateGameFoldersList();
            }
        });
    }
}