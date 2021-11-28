package org.easyrpg.player.settings;

import android.net.Uri;
import android.os.Bundle;
import android.widget.CheckBox;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import org.easyrpg.player.R;

public class SettingsAudioActivity extends AppCompatActivity {
    CheckBox enableAudioCheckbox, enableSoundfontCheckbox;

    // TODO: Implement OnResume() with a search of the soundfile (currently only during games scanning)

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings_audio);

        SettingsManager.init(getApplicationContext());

        // Setting UI components
        // Enable audio
        this.enableAudioCheckbox = (CheckBox) findViewById(R.id.settings_checkbox_enable_audio);
        this.enableAudioCheckbox.setChecked(SettingsManager.isAudioEnabled());
        this.enableAudioCheckbox.setOnClickListener(v -> SettingsManager.setAudioEnabled(((CheckBox)v).isChecked()));

        // Enable custom SoundFonts
        this.enableSoundfontCheckbox = (CheckBox) findViewById(R.id.settings_checkbox_enable_soundfonts);
        this.enableSoundfontCheckbox.setChecked(SettingsManager.isCustomSoundFountsEnabled());
        this.enableSoundfontCheckbox.setOnClickListener(v -> SettingsManager.setCustomSoundFountsEnabled(((CheckBox)v).isChecked()));

        // Update the games folder path
        TextView soundFontExplanationView = (TextView) findViewById(R.id.settings_soundfont_explanation);
        String soundfontExplanation = soundFontExplanationView.getText().toString();
        Uri gamesFolderURI = SettingsManager.getGamesFolderURI(this);
        if (gamesFolderURI != null) {
            String gamesFolderName = gamesFolderURI.getPath();
            soundfontExplanation = soundfontExplanation.replace("%", gamesFolderName);
            soundFontExplanationView.setText(soundfontExplanation);
        }

        // Update the SoundFont file path
        TextView soundFontFoundView = (TextView) findViewById(R.id.settings_soundfont_found);
        String soundFontFound = soundFontFoundView.getText().toString();
        Uri soundFountURI  = SettingsManager.getSoundFountFileURI();
        String soundFontFoundName;
        if (soundFountURI != null) {
            soundFontFoundName = soundFountURI.getPath();
        } else {
            soundFontFoundName = "N/A";
        }
        soundFontFound = soundFontFound.replace("%", soundFontFoundName);
        soundFontFoundView.setText(soundFontFound);
    }
}
