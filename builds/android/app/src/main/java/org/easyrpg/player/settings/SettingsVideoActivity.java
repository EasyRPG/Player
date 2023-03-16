package org.easyrpg.player.settings;

import android.os.Bundle;
import android.view.View;
import android.widget.CheckBox;
import android.widget.RadioButton;

import androidx.appcompat.app.AppCompatActivity;

import org.easyrpg.player.R;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class SettingsVideoActivity extends AppCompatActivity implements View.OnClickListener {
    List<RadioButton> imageSizeRadioButtonList;
    List<RadioButton> gameResolutionRadioButtonList;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.activity_settings_video);

        SettingsManager.init(getApplicationContext());

        // Setting UI components
        CheckBox forceLandscapeModeCheckbox = findViewById(R.id.force_landscape_mode);
        forceLandscapeModeCheckbox.setChecked(SettingsManager.isForcedLandscape());
        forceLandscapeModeCheckbox.setOnClickListener(this);

        // Scaling mode
        imageSizeRadioButtonList = Arrays.asList(
            findViewById(R.id.settings_scale_nearest_radio_button),
            findViewById(R.id.settings_scale_integer_radio_button),
            findViewById(R.id.settings_scale_bilinear_radio_button));

        gameResolutionRadioButtonList = Arrays.asList(
            findViewById(R.id.settings_resolution_original),
            findViewById(R.id.settings_resolution_widescreen),
            findViewById(R.id.settings_resolution_ultrawide));

        for (int i = 0; i < imageSizeRadioButtonList.size(); ++i) {
            RadioButton radio = imageSizeRadioButtonList.get(i);
            final int finalI = i;
            radio.setOnClickListener(view -> {
                SettingsManager.setImageSize(finalI);
                updateImageSizeRadioButtonCheckStatus();
            });
        }
        updateImageSizeRadioButtonCheckStatus();

        for (int i = 0; i < gameResolutionRadioButtonList.size(); ++i) {
            RadioButton radio = gameResolutionRadioButtonList.get(i);
            final int finalI = i;
            radio.setOnClickListener(view -> {
                SettingsManager.setGameResolution(finalI);
                updateGameResolutionRadioButtonCheckStatus();
            });
        }
        updateGameResolutionRadioButtonCheckStatus();

        // Stretch
        CheckBox stretchCheckbox = findViewById(R.id.settings_stretch);
        stretchCheckbox.setChecked(SettingsManager.isStretch());
        stretchCheckbox.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.force_landscape_mode) {
            SettingsManager.setForcedLandscape(((CheckBox)v).isChecked());
        }

        if (v.getId() == R.id.settings_stretch) {
            SettingsManager.setStretch(((CheckBox)v).isChecked());
        }
    }

    private void updateImageSizeRadioButtonCheckStatus() {
        for (int i = 0; i < imageSizeRadioButtonList.size(); ++i) {
            imageSizeRadioButtonList.get(i).setChecked(SettingsManager.getImageSize() == i);
        }
    }

    private void updateGameResolutionRadioButtonCheckStatus() {
        for (int i = 0; i < gameResolutionRadioButtonList.size(); ++i) {
            gameResolutionRadioButtonList.get(i).setChecked(SettingsManager.getGameResolution() == i);
        }
    }
}
