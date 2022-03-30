package org.easyrpg.player.settings;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import androidx.appcompat.app.AppCompatActivity;

import org.easyrpg.player.R;

public class SettingsMainActivity extends AppCompatActivity implements View.OnClickListener {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        this.setContentView(R.layout.activity_settings_main);

        // Setting UI components
        Button graphicsButton = findViewById(R.id.settings_main_video);
        graphicsButton.setOnClickListener(this);
        Button audioButton = findViewById(R.id.settings_main_audio);
        audioButton.setOnClickListener(this);
        Button inputsButton = findViewById(R.id.settings_main_input);
        inputsButton.setOnClickListener(this);
        Button folderButton = findViewById(R.id.settings_main_easyrpg_folders);
        folderButton.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        Intent intent = null;

        int id = v.getId();
        if (id == R.id.settings_main_video) {
            intent = new Intent(this, SettingsVideoActivity.class);
        } else if (id == R.id.settings_main_audio) {
            intent = new Intent(this, SettingsAudioActivity.class);
        } else if (id == R.id.settings_main_easyrpg_folders) {
            intent = new Intent(this, SettingsGamesFolderActivity.class);
        } else if (id == R.id.settings_main_input) {
            intent = new Intent(this, SettingsInputActivity.class);
        }

        if (intent != null) {
            startActivity(intent);
        }
    }
}
