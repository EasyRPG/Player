package org.easyrpg.player.settings;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.CheckBox;
import org.easyrpg.player.settings.SettingsManager;

import org.easyrpg.player.R;

public class SettingsVideoActivity extends AppCompatActivity implements View.OnClickListener {
    CheckBox forceLandscapeModeCheckbox;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.activity_settings_video);

        SettingsManager.init(getApplicationContext());

        // Setting UI components
        this.forceLandscapeModeCheckbox = (CheckBox) findViewById(R.id.force_landscape_mode);
        this.forceLandscapeModeCheckbox.setChecked(SettingsManager.isForcedLandscape());
        this.forceLandscapeModeCheckbox.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        Intent intent = null;
        switch (v.getId()) {
            case R.id.force_landscape_mode:
                SettingsManager.setForcedLandscape(((CheckBox)v).isChecked());
                break;
        }
    }
}
