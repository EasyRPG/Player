package org.easyrpg.player.settings;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

import org.easyrpg.player.DirectoryChooser;
import org.easyrpg.player.R;
import org.easyrpg.player.button_mapping.ButtonMappingActivity;
import org.easyrpg.player.button_mapping.ButtonMappingManager;
import org.easyrpg.player.button_mapping.ButtonMappingManager.InputLayout;

/**
 * Activity where users can change options
 */
public class SettingsActivity extends AppCompatActivity implements OnClickListener {
    private ButtonMappingManager buttonMappingManager;
    private LinearLayout gamesFoldersListLayout, inputLayoutListLayout;

    // GUI component
    private CheckBox vibrateWhenSlidingCheckbox, ignoreLayoutSizeCheckbox, forceLandscapeCheckbox;
    private SeekBar layoutSizeSeekBar, layoutTransparencyLayout;
    private TextView layoutTransparencyTextView, layoutSizeTextView, directoryTextView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.settings_activity);

        this.buttonMappingManager = ButtonMappingManager.getButtonMapping(this);

        // Game folders
        gamesFoldersListLayout = (LinearLayout) findViewById(R.id.games_folders_list);
        updateGameFoldersList();

        // Fill UI components
        // Layout transparency
        configureSeekBarLayoutTransparency();
        configureSeekBarLayoutSize();

        // Vibration
        CheckBox cb_vibration = (CheckBox) findViewById(R.id.settings_enable_vibration);
        cb_vibration.setChecked(SettingsManager.isVibrationEnabled());

        vibrateWhenSlidingCheckbox = (CheckBox) findViewById(R.id.settings_vibrate_when_slidind);
        vibrateWhenSlidingCheckbox.setEnabled(cb_vibration.isChecked());
        vibrateWhenSlidingCheckbox.setChecked(SettingsManager.isVibrateWhenSlidingDirectionEnabled());

        // Sound
        CheckBox cb_sounds = (CheckBox) findViewById(R.id.settings_audio);
        cb_sounds.setChecked(SettingsManager.isAudioEnabled());

        // Force Landscape
        forceLandscapeCheckbox = (CheckBox) findViewById(R.id.force_landscape_mode);
        forceLandscapeCheckbox.setChecked(SettingsManager.isForcedLandscape());
        forceLandscapeCheckbox.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                SettingsManager.setForcedLandscape(((CheckBox)v).isChecked());
            }
        });

        // InputLayouts list
        inputLayoutListLayout = (LinearLayout) findViewById(R.id.controls_settings_layout_list);
        updateSettingsList();

        // Buttons
        findViewById(R.id.settings_add_game_folder_button).setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.settings_add_game_folder_button :
                addAGameFolder();
                break;
            default :
                break;
        }
    }

    private void addAGameFolder() {
        new DirectoryChooser(this, SettingsManager.getEasyRPGFolder(), new Runnable() {
            @Override
            public void run() {
                String pathToAdd = DirectoryChooser.getSelectedPath();
                SettingsManager.addGameDirectory(pathToAdd);

                // Update UI
                updateGameFoldersList();
            }
        });
    }

    private void removeAGameFolder(String path) {
        SettingsManager.removeAGameFolder(path);

        // Update UI
        updateGameFoldersList();
    }

    private void updateGameFoldersList() {
        gamesFoldersListLayout.removeAllViews();

        for (String gameDirPath : SettingsManager.getGamesFolderList()) {
            LayoutInflater inflater = LayoutInflater.from(this);
            RelativeLayout layout = (RelativeLayout) inflater.inflate(R.layout.settings_item_list, null);

            // The name
            TextView nameTextView = (TextView) layout.findViewById(R.id.controls_settings_preset_name);
            nameTextView.setText(gameDirPath);
            nameTextView.setTextSize(12);

            // Option button (not present in the default folder)
            final String path = gameDirPath;
            ImageButton remove_button = (ImageButton) layout.findViewById(R.id.controls_settings_preset_option_button);
            remove_button.setImageResource(R.drawable.ic_action_content_clear);
            if (path.equals(SettingsManager.getEasyRPGFolder() + "/games")) {
                layout.removeView(remove_button);
            } else {
                remove_button.setOnClickListener(new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        removeAGameFolder(path);
                    }
                });
            }

            gamesFoldersListLayout.addView(layout);
        }
    }

    private void configureSeekBarLayoutTransparency() {
        layoutTransparencyLayout = (SeekBar) findViewById(R.id.settings_layout_transparency);
        layoutTransparencyLayout.setProgress(SettingsManager.getLayoutTransparency());
        layoutTransparencyLayout.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                SettingsManager.setLayoutTransparency(seekBar.getProgress());
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                layoutTransparencyTextView.setText((layoutTransparencyLayout.getProgress() * 100 / 255) + "%");
            }
        });

        layoutTransparencyTextView = (TextView) findViewById(R.id.settings_layout_transparency_text_view);
        layoutTransparencyTextView.setText((layoutTransparencyLayout.getProgress() * 100 / 255) + "%");
    }

    public void checkboxEnableIgnoreLayoutSize(View v) {
        CheckBox t = (CheckBox) v;

        SettingsManager.setIgnoreLayoutSizePreferencesEnabled(t.isChecked());

        // Update UI
        layoutSizeSeekBar.setEnabled(t.isChecked());
        layoutSizeTextView.setEnabled(ignoreLayoutSizeCheckbox.isChecked());
    }

    public void configureSeekBarLayoutSize() {
        // Checkbox
        ignoreLayoutSizeCheckbox = (CheckBox) findViewById(R.id.settings_ignore_layout_size);
        ignoreLayoutSizeCheckbox.setChecked(SettingsManager.isIgnoreLayoutSizePreferencesEnabled());

        // Seekbar
        layoutSizeSeekBar = (SeekBar) findViewById(R.id.settings_layout_size);
        layoutSizeSeekBar.setProgress(SettingsManager.getLayoutSize());
        layoutSizeSeekBar.setEnabled(ignoreLayoutSizeCheckbox.isChecked());
        layoutSizeSeekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                SettingsManager.setLayoutSize(seekBar.getProgress());
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                layoutSizeTextView.setText(layoutSizeSeekBar.getProgress() + "%");
            }
        });

        layoutSizeTextView = (TextView) findViewById(R.id.settings_input_size_text_view);
        layoutSizeTextView.setText(layoutSizeSeekBar.getProgress() + "%");
        layoutSizeTextView.setEnabled(ignoreLayoutSizeCheckbox.isChecked());
    }

    public void checkboxEnableVibration(View v) {
        CheckBox t = (CheckBox) v;

        SettingsManager.setVibrationEnabled(t.isChecked());
        vibrateWhenSlidingCheckbox.setEnabled(t.isChecked());
    }

    public void checkboxVibrateWhenSlidingToAnotherDirection(View v) {
        CheckBox c = (CheckBox) v;
        SettingsManager.setVibrateWhenSlidingDirectionEnabled(c.isChecked());
    }

    public void checkboxEnableAudio(View v) {
        CheckBox s = (CheckBox) v;
        SettingsManager.setAudioEnabled(s.isChecked());
    }

    /**
     * Update the InputLayouts' list and save the modification done by the user
     */
    private void refreshAndSaveLayoutList() {
        updateSettingsList();
        ButtonMappingManager.writeButtonMappingFile(this, buttonMappingManager);
    }

    private void updateSettingsList() {
        inputLayoutListLayout.removeAllViews();
        for (InputLayout i : buttonMappingManager.getLayoutList()) {
            InputLayoutItemListView view = new InputLayoutItemListView(this, i);
            inputLayoutListLayout.addView(view.layout);
        }
    }

    /**
     * Open a dialog box to add an InputLayout
     */
    public void addAnInputLayout(View v) {
        final EditText input = new EditText(this);
        // TODO : Restrict the edit text to alpha numeric characters

        // The dialog
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.add_an_input_layout).setView(input)
                .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        String text = input.getText().toString();
                        if (!text.isEmpty()) {
                            InputLayout layout = new InputLayout(text);
                            layout.setButton_list(
                                    ButtonMappingManager.InputLayout.getDefaultButtonList(getApplicationContext()));
                            buttonMappingManager.add(layout);
                            editInputLayout(layout);
                            refreshAndSaveLayoutList();
                        }
                    }
                }).setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.cancel();
            }
        });
        builder.show();
    }

    /**
     * Open a dialog box to configure an InputLayout
     */
    private void configureInputLayout(final InputLayout game_layout) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);

        String[] choiceArray = {getString(R.string.set_as_default), getString(R.string.edit_name),
                getString(R.string.edit_layout), getString(R.string.delete)};

        builder.setTitle(game_layout.getName()).setItems(choiceArray, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                switch (which) {
                    case 0:
                        buttonMappingManager.setDefaultLayout(game_layout.getId());
                        refreshAndSaveLayoutList();
                        break;
                    case 1:
                        editInputLayoutName(game_layout);
                        break;
                    case 2:
                        editInputLayout(game_layout);
                        break;
                    case 3:
                        delete_layout(game_layout);
                        break;
                    default:
                        break;
                }
            }
        });

        builder.show();
    }

    /**
     * Open a dialog box to configure an InputLayout's name
     */
    private void editInputLayoutName(final InputLayout game_layout) {
        // The editText field
        final EditText input = new EditText(this);
        input.setText(game_layout.getName());

        // The dialog box
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.edit_name).setView(input)
                .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        String text = input.getText().toString();
                        if (!text.isEmpty()) {
                            game_layout.setName(text);
                        }
                        refreshAndSaveLayoutList();
                    }
                }).setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.cancel();
            }
        });
        builder.show();
    }

    /**
     * Edit an InputLayout by opening the ButtonMapping activity
     */
    private void editInputLayout(final InputLayout game_layout) {
        Intent intent = new Intent(this, org.easyrpg.player.button_mapping.ButtonMappingActivity.class);
        intent.putExtra(ButtonMappingActivity.TAG_ID, game_layout.getId());
        startActivity(intent);
    }

    /**
     * Delete an InputLayout
     */
    private void delete_layout(final ButtonMappingManager.InputLayout game_layout) {
        // TODO : Ask confirmation
        buttonMappingManager.delete(this, game_layout);
        refreshAndSaveLayoutList();
    }

    private class InputLayoutItemListView {
        private RelativeLayout layout;
        private ImageButton settings_button;

        public InputLayoutItemListView(Context context, final InputLayout input_layout) {

            LayoutInflater inflater = LayoutInflater.from(context);
            layout = (RelativeLayout) inflater.inflate(R.layout.settings_item_list, null);

            // The name
            TextView input_layout_name = (TextView) layout.findViewById(R.id.controls_settings_preset_name);
            input_layout_name.setText(input_layout.getName());
            if (input_layout.isDefaultInputLayout(buttonMappingManager)) {
                input_layout_name.setText(input_layout_name.getText() + " (" + getString(R.string.default_layout) + ")");
            }

            // Option button
            settings_button = (ImageButton) layout.findViewById(R.id.controls_settings_preset_option_button);
            settings_button.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    configureInputLayout(input_layout);
                }
            });

            // Edit the layout by clicking on the view
            layout.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    editInputLayout(input_layout);
                }
            });
        }
    }
}
