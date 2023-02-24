package org.easyrpg.player.settings;

import android.os.Bundle;
import android.view.View;
import android.widget.CheckBox;
import android.widget.RadioButton;

import androidx.appcompat.app.AppCompatActivity;

import org.easyrpg.player.R;

import java.util.ArrayList;
import java.util.List;

public class SettingsVideoActivity extends AppCompatActivity implements View.OnClickListener {
    List<RadioButton> imageSizeRadioButtonList = new ArrayList<>();
    RadioButton imageSizeNearestRadioButton, imageSizeIntegerRadioButton, imageSizeBilinearRadioButton;

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
        this.imageSizeNearestRadioButton = findViewById(R.id.settings_scale_nearest_radio_button);
        imageSizeRadioButtonList.add(imageSizeNearestRadioButton);
        imageSizeNearestRadioButton.setOnClickListener(view -> {
            SettingsManager.setImageSize(0);
            updateImageSizeRadioButtonCheckStatus();
        });

        this.imageSizeIntegerRadioButton = findViewById(R.id.settings_scale_integer_radio_button);
        imageSizeRadioButtonList.add(imageSizeIntegerRadioButton);
        imageSizeIntegerRadioButton.setOnClickListener(view -> {
            SettingsManager.setImageSize(1);
            updateImageSizeRadioButtonCheckStatus();
        });

        this.imageSizeBilinearRadioButton = findViewById(R.id.settings_scale_bilinear_radio_button);
        imageSizeRadioButtonList.add(imageSizeBilinearRadioButton);
        imageSizeBilinearRadioButton.setOnClickListener(view -> {
            SettingsManager.setImageSize(2);
            updateImageSizeRadioButtonCheckStatus();
        });

        updateImageSizeRadioButtonCheckStatus();

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
        imageSizeNearestRadioButton.setChecked(SettingsManager.getImageSize() == 0);
        imageSizeIntegerRadioButton.setChecked(SettingsManager.getImageSize() == 1);
        imageSizeBilinearRadioButton.setChecked(SettingsManager.getImageSize() == 2);
    }
}
