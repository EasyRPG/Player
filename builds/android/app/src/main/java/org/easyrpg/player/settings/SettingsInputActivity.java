package org.easyrpg.player.settings;

import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.AppCompatSpinner;

import org.easyrpg.player.R;
import org.easyrpg.player.button_mapping.ButtonMappingActivity;
import org.easyrpg.player.button_mapping.ButtonMappingManager;

public class SettingsInputActivity extends AppCompatActivity implements View.OnClickListener {
    private CheckBox enableVibrateWhenSlidingCheckbox;
    private ButtonMappingManager buttonMappingManager;
    private LinearLayout inputLayoutList;
    private SeekBar layoutTransparencyLayout, layoutSizeSeekBar, fastForwardMultiplierSeekBar;
    private TextView layoutTransparencyTextView, layoutSizeTextView, fastForwardMultiplierTextView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.activity_settings_inputs);

        SettingsManager.init(getApplicationContext());

        this.buttonMappingManager = ButtonMappingManager.getInstance(this);

        // Setting UI components
        CheckBox enableVibrationCheckBox = (CheckBox) findViewById(R.id.settings_enable_vibration);
        enableVibrationCheckBox.setChecked(SettingsManager.isVibrationEnabled());
        enableVibrationCheckBox.setOnClickListener(this);

        enableVibrateWhenSlidingCheckbox = (CheckBox) findViewById(R.id.settings_vibrate_when_sliding);
        enableVibrateWhenSlidingCheckbox.setChecked(SettingsManager.isVibrateWhenSlidingDirectionEnabled());
        enableVibrateWhenSlidingCheckbox.setOnClickListener(this);

        configureFastForwardButton();
        configureLayoutTransparencySystem();
        configureLayoutSizeSystem();

        // Input Layouts
        updateInputLayoutList();
        // Button for adding input layouts
        Button addInputLayoutButton = (Button) findViewById(R.id.settings_add_input_layout_button);
        addInputLayoutButton.setOnClickListener(this);
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
        } else if (id == R.id.settings_add_input_layout_button) {
            addAnInputLayout();
        }
    }

    private void configureFastForwardButton() {
        AppCompatSpinner chooseFastForwardModeSpinner = (AppCompatSpinner) findViewById(R.id.settings_fast_forward_mode);
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

        fastForwardMultiplierSeekBar = (SeekBar) findViewById(R.id.settings_fast_forward_multiplier);
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
        fastForwardMultiplierTextView = (TextView) findViewById(R.id.settings_fast_forward_multiplier_text_view);
        String text = getString(R.string.fast_forward_factor) + " " + (fastForwardMultiplierSeekBar.getProgress() + 2) + "x";
        fastForwardMultiplierTextView.setText(text);
    }

    private void configureLayoutTransparencySystem() {
        // The seekbar permit to modify this value
        layoutTransparencyLayout = (SeekBar) findViewById(R.id.settings_layout_transparency);
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
        layoutTransparencyTextView = (TextView) findViewById(R.id.settings_layout_transparency_text_view);
        String text = (layoutTransparencyLayout.getProgress() * 100 / 255) + "%";
        layoutTransparencyTextView.setText(text);
    }

    public void configureLayoutSizeSystem() {
        // Checkbox : Ignore the predefined layout size
        CheckBox ignoreLayoutSizeCheckbox = (CheckBox) findViewById(R.id.settings_ignore_layout_size);
        ignoreLayoutSizeCheckbox.setChecked(SettingsManager.isIgnoreLayoutSizePreferencesEnabled());
        ignoreLayoutSizeCheckbox.setOnClickListener(v -> {
            CheckBox b = (CheckBox) v;
            SettingsManager.setIgnoreLayoutSizePreferencesEnabled(b.isChecked());

            // Update UI
            layoutSizeSeekBar.setEnabled(b.isChecked());
            layoutSizeTextView.setEnabled(b.isChecked());
        });

        // Seekbar : modify the layout size
        layoutSizeSeekBar = (SeekBar) findViewById(R.id.settings_layout_size);
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
        layoutSizeTextView = (TextView) findViewById(R.id.settings_input_size_text_view);
        String text = layoutSizeSeekBar.getProgress() + "%";
        layoutSizeTextView.setText(text);
        layoutSizeTextView.setEnabled(ignoreLayoutSizeCheckbox.isChecked());
    }

    private void updateInputLayoutList() {
        if (inputLayoutList == null) {
            inputLayoutList = (LinearLayout) findViewById(R.id.controls_settings_layout_list);
        }
        inputLayoutList.removeAllViews();

        for (ButtonMappingManager.InputLayout i : buttonMappingManager.getLayoutList()) {
            InputLayoutItemListView view = new InputLayoutItemListView(this, i);
            inputLayoutList.addView(view.layout);
        }
    }

    private void refreshInputLayoutList() {
        updateInputLayoutList();
    }

    /**
     * Open a dialog box to add an InputLayout
     */
    public void addAnInputLayout() {
        final EditText input = new EditText(this);
        // TODO : Restrict the edit text to alpha numeric characters

        // The dialog
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.add_an_input_layout).setView(input)
                .setPositiveButton(R.string.ok, (dialog, which) -> {
                    String text = input.getText().toString();
                    if (!text.isEmpty()) {
                        // Create a new input layout, with the default layout
                        ButtonMappingManager.InputLayout newInputLayout = new ButtonMappingManager.InputLayout(text);
                        newInputLayout.setButtonList(
                                ButtonMappingManager.InputLayout.getDefaultButtonList(getApplicationContext()));

                        // Add it to the input layout list, and open the activity to modify it
                        buttonMappingManager.add(newInputLayout);
                        buttonMappingManager.save();
                        //editInputLayout(newInputLayout);

                        refreshInputLayoutList();
                    }
                }).setNegativeButton("Cancel", (dialog, which) -> dialog.cancel());
        builder.show();
    }

    /**
     * Open a dialog box to configure an InputLayout
     */
    private void configureInputLayout(final ButtonMappingManager.InputLayout gameLayout) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);

        String[] choiceArray = {getString(R.string.edit_name),
                getString(R.string.edit_layout), getString(R.string.delete)};

        builder.setTitle(gameLayout.getName()).setItems(choiceArray, (dialog, which) -> {
            switch (which) {
                case 0:
                    editInputLayoutName(gameLayout);
                    break;
                case 1:
                    editInputLayout(gameLayout);
                    break;
                case 2:
                    deleteInputLayout(gameLayout);
                    break;
                default:
                    break;
            }
        });

        builder.show();
    }

    /**
     * Open a dialog box to configure an InputLayout's name
     */
    private void editInputLayoutName(final ButtonMappingManager.InputLayout inputLayout) {
        // The editText field
        final EditText input = new EditText(this);
        input.setText(inputLayout.getName());

        // The dialog box
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.edit_name).setView(input)
                .setPositiveButton(R.string.ok, (dialog, which) -> {
                    String text = input.getText().toString();
                    if (!text.isEmpty()) {
                        inputLayout.setName(text);
                    }
                    buttonMappingManager.save();
                    refreshInputLayoutList();
                }).setNegativeButton(R.string.cancel, (dialog, which) -> dialog.cancel());
        builder.show();
    }

    /**
     * Edit an InputLayout by opening the ButtonMapping activity
     */
    private void editInputLayout(final ButtonMappingManager.InputLayout game_layout) {
        Intent intent = new Intent(this, org.easyrpg.player.button_mapping.ButtonMappingActivity.class);
        intent.putExtra(ButtonMappingActivity.TAG_ID, game_layout.getId());
        startActivity(intent);
    }

    /**
     * Delete an InputLayout
     */
    private void deleteInputLayout(final ButtonMappingManager.InputLayout game_layout) {
        // TODO : Ask confirmation
        buttonMappingManager.delete(this, game_layout);
        refreshInputLayoutList();
    }

    private class InputLayoutItemListView {
        private final RelativeLayout layout;

        public InputLayoutItemListView(Context context, final ButtonMappingManager.InputLayout input_layout) {

            LayoutInflater inflater = LayoutInflater.from(context);
            layout = (RelativeLayout) inflater.inflate(R.layout.settings_item_list, null);

            // The Radio Button
            RadioButton radioButton = (RadioButton) layout.findViewById(R.id.controls_settings_preset_radio_button);
            radioButton.setOnClickListener(v -> {
                buttonMappingManager.setSelectedInputLayout(input_layout.getId());
                buttonMappingManager.save();
                updateInputLayoutList();
            });
            if (input_layout.isChosenInputLayout(buttonMappingManager)) {
                radioButton.setChecked(true);
            }

            // The name
            TextView input_layout_name = (TextView) layout.findViewById(R.id.controls_settings_preset_name);
            input_layout_name.setText(input_layout.getName());

            // Option button
            ImageButton settings_button = (ImageButton) layout.findViewById(R.id.controls_settings_preset_option_button);
            settings_button.setOnClickListener(v -> configureInputLayout(input_layout));

            // Edit the layout by clicking on the view
            layout.setOnClickListener(v -> editInputLayout(input_layout));
        }
    }
}
