package org.easyrpg.player.settings;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.CheckBox;
import android.widget.ImageButton;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.AppCompatSpinner;

import org.easyrpg.player.R;
import org.easyrpg.player.button_mapping.ButtonMappingActivity;
import org.easyrpg.player.button_mapping.InputLayout;

public class SettingsInputActivity extends AppCompatActivity implements View.OnClickListener {
    private CheckBox enableVibrateWhenSlidingCheckbox;
    private SeekBar layoutTransparencyLayout, layoutSizeSeekBar, fastForwardMultiplierSeekBar;
    private TextView layoutTransparencyTextView, layoutSizeTextView, fastForwardMultiplierTextView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.activity_settings_inputs);

        SettingsManager.init(getApplicationContext());

        // Setting UI components
        CheckBox enableVibrationCheckBox = findViewById(R.id.settings_enable_vibration);
        enableVibrationCheckBox.setChecked(SettingsManager.isVibrationEnabled());
        enableVibrationCheckBox.setOnClickListener(this);

        enableVibrateWhenSlidingCheckbox = findViewById(R.id.settings_vibrate_when_sliding);
        enableVibrateWhenSlidingCheckbox.setChecked(SettingsManager.isVibrateWhenSlidingDirectionEnabled());
        enableVibrateWhenSlidingCheckbox.setOnClickListener(this);

        configureFastForwardButton();
        configureLayoutTransparencySystem();
        configureLayoutSizeSystem();

        ImageButton horizontalLayoutSettingsButton = findViewById(R.id.settings_horizontal_input_layout_settings_button);
        horizontalLayoutSettingsButton.setOnClickListener(view -> editInputLayout(InputLayout.Orientation.ORIENTATION_HORIZONTAL));

        ImageButton verticalLayoutSettingsButton = findViewById(R.id.settings_vertical_input_layout_settings_button);
        verticalLayoutSettingsButton.setOnClickListener(view -> editInputLayout(InputLayout.Orientation.ORIENTATION_VERTICAL));
    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        if (id == R.id.settings_enable_vibration) {
            CheckBox c = (CheckBox) v;
            SettingsManager.setVibrationEnabled(c.isChecked());
            enableVibrateWhenSlidingCheckbox.setEnabled(c.isChecked());
        } else if (id == R.id.settings_vibrate_when_sliding){
            SettingsManager.setVibrateWhenSlidingDirectionEnabled(((CheckBox) v).isChecked());
        }
    }

    private void configureFastForwardButton() {
        AppCompatSpinner chooseFastForwardModeSpinner = findViewById(R.id.settings_fast_forward_mode);
        chooseFastForwardModeSpinner.setSelection(SettingsManager.getFastForwardMode());
        chooseFastForwardModeSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {

            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                SettingsManager.setFastForwardMode(i);
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {
            }
        });

        fastForwardMultiplierSeekBar = findViewById(R.id.settings_fast_forward_multiplier);
        fastForwardMultiplierSeekBar.setProgress(SettingsManager.getFastForwardMultiplier() - 2);
        fastForwardMultiplierSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                // The seekbar has values 0-8, we want 2-10
                SettingsManager.setFastForwardMultiplier(seekBar.getProgress() + 2);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                String text = getString(R.string.fast_forward_factor) + " " + (fastForwardMultiplierSeekBar.getProgress() + 2) + "x";
                fastForwardMultiplierTextView.setText(text);
            }
        });

        // The textview displays the current multiplier value
        fastForwardMultiplierTextView = findViewById(R.id.settings_fast_forward_multiplier_text_view);
        String text = getString(R.string.fast_forward_factor) + " " + (fastForwardMultiplierSeekBar.getProgress() + 2) + "x";
        fastForwardMultiplierTextView.setText(text);
    }

    private void configureLayoutTransparencySystem() {
        // The seekbar permit to modify this value
        layoutTransparencyLayout = findViewById(R.id.settings_layout_transparency);
        layoutTransparencyLayout.setProgress(SettingsManager.getLayoutTransparency());
        layoutTransparencyLayout.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                SettingsManager.setLayoutTransparency(seekBar.getProgress());
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                String text = (layoutTransparencyLayout.getProgress() * 100 / 255) + "%";
                layoutTransparencyTextView.setText(text);
            }
        });

        // The textview display the current transparency value
        layoutTransparencyTextView = findViewById(R.id.settings_layout_transparency_text_view);
        String text = (layoutTransparencyLayout.getProgress() * 100 / 255) + "%";
        layoutTransparencyTextView.setText(text);
    }

    public void configureLayoutSizeSystem() {
        // Checkbox : Ignore the predefined layout size
        CheckBox ignoreLayoutSizeCheckbox = findViewById(R.id.settings_ignore_layout_size);
        ignoreLayoutSizeCheckbox.setChecked(SettingsManager.isIgnoreLayoutSizePreferencesEnabled());
        ignoreLayoutSizeCheckbox.setOnClickListener(v -> {
            CheckBox b = (CheckBox) v;
            SettingsManager.setIgnoreLayoutSizePreferencesEnabled(b.isChecked());

            // Update UI
            layoutSizeSeekBar.setEnabled(b.isChecked());
            layoutSizeTextView.setEnabled(b.isChecked());
        });

        // Seekbar : modify the layout size
        layoutSizeSeekBar = findViewById(R.id.settings_layout_size);
        layoutSizeSeekBar.setProgress(SettingsManager.getLayoutSize());
        layoutSizeSeekBar.setEnabled(SettingsManager.isIgnoreLayoutSizePreferencesEnabled());
        layoutSizeSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                SettingsManager.setLayoutSize(seekBar.getProgress());
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                String text = layoutSizeSeekBar.getProgress() + "%";
                layoutSizeTextView.setText(text);
            }
        });

        // Textview : display the current transparency value
        layoutSizeTextView = findViewById(R.id.settings_input_size_text_view);
        String text = layoutSizeSeekBar.getProgress() + "%";
        layoutSizeTextView.setText(text);
        layoutSizeTextView.setEnabled(ignoreLayoutSizeCheckbox.isChecked());
    }

    /**
     * Edit an InputLayout by opening the ButtonMapping activity
     */
    private void editInputLayout(InputLayout.Orientation orientation) {
        Intent intent = new Intent(this, org.easyrpg.player.button_mapping.ButtonMappingActivity.class);
        if (orientation == InputLayout.Orientation.ORIENTATION_HORIZONTAL) {
            intent.putExtra(ButtonMappingActivity.TAG_ORIENTATION, ButtonMappingActivity.TAG_ORIENTATION_VALUE_HORIZONTAL);
        } else {
            intent.putExtra(ButtonMappingActivity.TAG_ORIENTATION, ButtonMappingActivity.TAG_ORIENTATION_VALUE_VERTICAL);
        }
        startActivity(intent);
    }
}
