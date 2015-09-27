package org.easyrpg.player;

import org.easyrpg.player.button_mapping.ButtonMappingActivity;
import org.easyrpg.player.button_mapping.ButtonMappingModel;
import org.easyrpg.player.button_mapping.ButtonMappingModel.InputLayout;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
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

/** Activity where users can change options */
public class SettingsActivity extends Activity {
	public static boolean VIBRATION;
	public static long VIBRATION_DURATION = 20; // ms
	public static boolean VIBRATE_WHEN_SLIDING_DIRECTION;
	public static int LAYOUT_TRANSPARENCY;
	public static boolean IGNORE_LAYOUT_SIZE_SETTINGS;
	public static int LAYOUT_SIZE;

	// ButtonMapping options
	private SharedPreferences pref;
	private SharedPreferences.Editor editor;
	private ButtonMappingModel mapping_model;
	private LinearLayout layout_settings;

	// GUI component
	CheckBox cb_vibration_direction, cb_ignore_layout_size;
	SeekBar sb_layout_size_buttons, sb_input_transparency;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.settings_activity);

		pref = PreferenceManager.getDefaultSharedPreferences(this);
		editor = pref.edit();

		// Retrieve configuration to set the state of component
		configureSeekBarLayoutTransparency();
		configureSeekBarLayoutSize();

		// Vibration
		CheckBox cb_vibration = (CheckBox) findViewById(R.id.settings_enable_vibration);
		cb_vibration.setChecked(pref.getBoolean(getString(R.string.pref_enable_vibration), true));

		cb_vibration_direction = (CheckBox) findViewById(R.id.settings_vibrate_when_slidind);
		cb_vibration_direction
				.setChecked(pref.getBoolean(getString(R.string.pref_vibrate_when_sliding_direction), false));
		cb_vibration_direction.setEnabled(cb_vibration.isChecked());

		// ButtonMapping system
		// Retrieve the Button Mapping Model from the preferences' file
		mapping_model = ButtonMappingModel.getButtonMapping(this);

		// Configure the InputLayouts list
		layout_settings = (LinearLayout) findViewById(R.id.controls_settings_layout_list);
		updateSettingsList();
	}

	/** Load user preferences */
	public static void updateUserPreferences(Context context) {
		SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(context);
		VIBRATION = sharedPref.getBoolean(context.getString(R.string.pref_enable_vibration), true);
		LAYOUT_TRANSPARENCY = sharedPref.getInt(context.getString(R.string.pref_layout_transparency), 100);
		VIBRATE_WHEN_SLIDING_DIRECTION = sharedPref
				.getBoolean(context.getString(R.string.pref_vibrate_when_sliding_direction), false);
		IGNORE_LAYOUT_SIZE_SETTINGS = sharedPref.getBoolean(context.getString(R.string.pref_ignore_size_settings),
				false);
		LAYOUT_SIZE = sharedPref.getInt(context.getString(R.string.pref_size_every_buttons), 100);
	}

	public void configureSeekBarLayoutTransparency() {
		sb_input_transparency = (SeekBar) findViewById(R.id.settings_layout_transparency);
		sb_input_transparency.setProgress(pref.getInt(getString(R.string.pref_layout_transparency), 100));
		sb_input_transparency.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

			@Override
			public void onStopTrackingTouch(SeekBar seekBar) {
				editor.putInt(getString(R.string.pref_layout_transparency), seekBar.getProgress());
				editor.commit();
			}

			@Override
			public void onStartTrackingTouch(SeekBar seekBar) {
			}

			@Override
			public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
			}
		});
	}

	public void checkboxEnableIgnoreLayoutSize(View v) {
		CheckBox t = (CheckBox) v;

		editor.putBoolean(getString(R.string.pref_ignore_size_settings), t.isChecked());
		sb_layout_size_buttons.setEnabled(t.isChecked());

		editor.commit();
	}

	public void configureSeekBarLayoutSize() {
		// Checkbox
		cb_ignore_layout_size = (CheckBox) findViewById(R.id.settings_ignore_layout_size);
		cb_ignore_layout_size.setChecked(pref.getBoolean(getString(R.string.pref_ignore_size_settings), false));

		// Seekbar
		sb_layout_size_buttons = (SeekBar) findViewById(R.id.settings_layout_size);
		sb_layout_size_buttons.setProgress(pref.getInt(getString(R.string.pref_size_every_buttons), 100));
		sb_layout_size_buttons.setEnabled(cb_ignore_layout_size.isChecked());
		sb_layout_size_buttons.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

			@Override
			public void onStopTrackingTouch(SeekBar seekBar) {
				editor.putInt(getString(R.string.pref_size_every_buttons), seekBar.getProgress());
				editor.commit();
			}

			@Override
			public void onStartTrackingTouch(SeekBar seekBar) {
			}

			@Override
			public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
			}
		});
	}

	public void checkboxEnableVibration(View v) {
		CheckBox t = (CheckBox) v;

		editor.putBoolean(getString(R.string.pref_enable_vibration), t.isChecked());
		cb_vibration_direction.setEnabled(t.isChecked());

		editor.commit();
	}

	public void checkboxVibrateWhenSlidingToAnotherDirection(View v) {
		CheckBox c = (CheckBox) v;

		if (c.isChecked())
			editor.putBoolean(getString(R.string.pref_vibrate_when_sliding_direction), true);
		else
			editor.putBoolean(getString(R.string.pref_vibrate_when_sliding_direction), false);

		editor.commit();
	}

	/**
	 * Update the InputLayouts' list and save the modification done by the user
	 */
	public void refreshAndSaveLayoutList() {
		updateSettingsList();
		ButtonMappingModel.writeButtonMappingFile(mapping_model);
	}

	public void updateSettingsList() {
		layout_settings.removeAllViews();
		for (InputLayout i : mapping_model.getLayout_list()) {
			InputLayoutItemListView view = new InputLayoutItemListView(this, i);
			layout_settings.addView(view.layout);
		}
	}

	/** Open a dialog box to add an InputLayout */
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
									ButtonMappingModel.InputLayout.getDefaultButtonList(getApplicationContext()));
							mapping_model.add(layout);

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

	/** Open a dialog box to configure an InputLayout */
	public void configureInputLayout(final InputLayout game_layout) {
		AlertDialog.Builder builder = new AlertDialog.Builder(this);

		String[] choiceArray = { getString(R.string.set_as_default), getString(R.string.edit_name),
				getString(R.string.edit_layout), getString(R.string.delete) };

		builder.setTitle(game_layout.getName()).setItems(choiceArray, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				switch (which) {
				case 0:
					mapping_model.setDefaultLayout(game_layout.getId());
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

	/** Open a dialog box to configure an InputLayout's name */
	public void editInputLayoutName(final InputLayout game_layout) {
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

	/** Edit an InputLayout by opening the ButtonMapping activity */
	public void editInputLayout(final InputLayout game_layout) {
		Intent intent = new Intent(this, org.easyrpg.player.button_mapping.ButtonMappingActivity.class);
		intent.putExtra(ButtonMappingActivity.TAG_ID, game_layout.getId());
		startActivity(intent);
	}

	/** Delete an InputLayout */
	public void delete_layout(final ButtonMappingModel.InputLayout game_layout) {
		// TODO : Ask confirmation
		mapping_model.delete(this, game_layout);
		refreshAndSaveLayoutList();
	}

	private class InputLayoutItemListView {
		private RelativeLayout layout;
		private ImageButton settings_button;

		public InputLayoutItemListView(Context context, final InputLayout input_layout) {

			LayoutInflater inflater = LayoutInflater.from(context);
			layout = (RelativeLayout) inflater.inflate(R.layout.controls_settings_item_list, null);

			// The name
			TextView input_layout_name = (TextView) layout.findViewById(R.id.controls_settings_preset_name);
			input_layout_name.setText(input_layout.getName());
			if(input_layout.isDefaultInputLayout(mapping_model)){
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
