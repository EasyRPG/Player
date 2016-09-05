package org.easyrpg.player.settings;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.CheckBox;
import org.easyrpg.player.settings.SettingsManager;

import org.easyrpg.player.R;

public class SettingsAudioActivity extends AppCompatActivity implements View.OnClickListener {
    CheckBox enableAudioCheckbox;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings_audio);

        SettingsManager.init(getApplicationContext());

        // Setting UI components
        this.enableAudioCheckbox = (CheckBox) findViewById(R.id.settings_audio);
        this.enableAudioCheckbox.setChecked(SettingsManager.isAudioEnabled());
        this.enableAudioCheckbox.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        Intent intent = null;
        switch (v.getId()) {
            case R.id.settings_audio:
                SettingsManager.setAudioEnabled(((CheckBox)v).isChecked());
                break;
        }
    }
}
