package org.easyrpg.player.settings;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;

import org.easyrpg.player.R;

public class SettingsMainActivity extends AppCompatActivity implements View.OnClickListener {
    private Button graphicsButton, audioButton, inputsButton, folderButton;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        this.setContentView(R.layout.activity_settings_main);

        // Setting UI components
        graphicsButton = (Button) findViewById(R.id.settings_main_video);
        graphicsButton.setOnClickListener(this);
        audioButton = (Button) findViewById(R.id.settings_main_audio);
        audioButton.setOnClickListener(this);
        inputsButton = (Button) findViewById(R.id.settings_main_input);
        inputsButton.setOnClickListener(this);
        folderButton = (Button) findViewById(R.id.settings_main_game_folder);
        folderButton.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        Intent intent = null;
        switch (v.getId()) {
            case R.id.settings_main_video:
                intent = new Intent(this, SettingsVideoActivity.class);
                break;
            case R.id.settings_main_audio:
                intent = new Intent(this, SettingsAudioActivity.class);
                break;
            case R.id.settings_main_game_folder:
                intent = new Intent(this, SettingsGamesFoldersActivity.class);
                break;
            case R.id.settings_main_input:
                intent = new Intent(this, SettingsInputActivity.class);
                break;
        }
        if (intent != null) {
            startActivity(intent);
        }
    }
}
