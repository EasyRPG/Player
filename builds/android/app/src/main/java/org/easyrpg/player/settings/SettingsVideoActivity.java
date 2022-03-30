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
    CheckBox forceLandscapeModeCheckbox;
    List<RadioButton> imageSizeRadioButtonList = new ArrayList<>();
    RadioButton imageSizeUniformPixelRadioButton, imageSizeStretchPixelRadioButton;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.activity_settings_video);

        SettingsManager.init(getApplicationContext());

        // Setting UI components
        this.forceLandscapeModeCheckbox = findViewById(R.id.force_landscape_mode);
        this.forceLandscapeModeCheckbox.setChecked(SettingsManager.isForcedLandscape());
        this.forceLandscapeModeCheckbox.setOnClickListener(this);

        // Image size
        this.imageSizeUniformPixelRadioButton  = findViewById(R.id.settings_image_uniform_pixel_size_radio_button);
        imageSizeRadioButtonList.add(imageSizeUniformPixelRadioButton);
        imageSizeUniformPixelRadioButton.setOnClickListener(view -> {
            SettingsManager.setImageSize(SettingsManager.IMAGE_SIZE_UNIFORM_PIXEL_SIZE);
            updateImageSizeRadioButtonCheckStatus();
        });

        this.imageSizeStretchPixelRadioButton  = findViewById(R.id.settings_image_stretch_radio_button);
        imageSizeRadioButtonList.add(imageSizeStretchPixelRadioButton);
        imageSizeStretchPixelRadioButton.setOnClickListener(view -> {
            SettingsManager.setImageSize(SettingsManager.IMAGE_SIZE_STRETCH_IMAGE);
            updateImageSizeRadioButtonCheckStatus();
        });

        updateImageSizeRadioButtonCheckStatus();
    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.force_landscape_mode) {
            SettingsManager.setForcedLandscape(((CheckBox)v).isChecked());
        }
    }

    private void updateImageSizeRadioButtonCheckStatus() {
        imageSizeUniformPixelRadioButton.setChecked(SettingsManager.getImageSize() == SettingsManager.IMAGE_SIZE_UNIFORM_PIXEL_SIZE);
        imageSizeStretchPixelRadioButton.setChecked(SettingsManager.getImageSize() == SettingsManager.IMAGE_SIZE_STRETCH_IMAGE);
    }
}
