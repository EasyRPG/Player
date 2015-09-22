package org.easyrpg.player;

import java.util.LinkedList;

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
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

/** Activity where users can change options */
public class SettingsActivity extends Activity {
	public static boolean	VIBRATION;
	public static long 		VIBRATION_DURATION = 20; //ms
	public static int		LAYOUT_TRANSPARENCY;
	
	//ButtonMapping options
	private SharedPreferences pref;
	private SharedPreferences.Editor editor ;
	private ButtonMappingModel mapping_model;
	private ListView layout_list_view;
	private InputLayoutListAdapter layout_list_adapter;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.settings_activity);
		
		pref = PreferenceManager.getDefaultSharedPreferences(this);
		editor = pref.edit();

		// Retrieve configuration to set the state of checkbox's
		CheckBox cb_vibration = (CheckBox)findViewById(R.id.settings_enable_vibration);
		cb_vibration.setChecked(pref.getBoolean(getString(R.string.pref_enable_vibration), true));
		
		SeekBar sb_input_transparency = (SeekBar)findViewById(R.id.settings_layout_transparency);
		configureSeekBarLayoutTransparency(sb_input_transparency);
		sb_input_transparency.setProgress(pref.getInt(getString(R.string.pref_layout_transparency), 100));
		
		// Retrieve the Button Mapping Model from the preferences' file
		mapping_model = ButtonMappingModel.getButtonMapping(this);

		// Configure the InputLayouts list
		layout_list_view = (ListView) findViewById(R.id.controls_settings_layout_list);
		layout_list_adapter = new InputLayoutListAdapter(mapping_model.getLayout_list());
		layout_list_view.setAdapter(layout_list_adapter);
	}
	
	public void checkboxEnableVibration(View v){
		CheckBox t = (CheckBox)v;
		if(t.isChecked()){
			editor.putBoolean(getString(R.string.pref_enable_vibration), true);
		}else{
			editor.putBoolean(getString(R.string.pref_enable_vibration), false);
		}
		editor.commit();
	}
	
	public void configureSeekBarLayoutTransparency(SeekBar b){
		b.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
			
			@Override
			public void onStopTrackingTouch(SeekBar seekBar) {
				editor.putInt(getString(R.string.pref_layout_transparency), seekBar.getProgress());
				editor.commit();
			}
			
			@Override
			public void onStartTrackingTouch(SeekBar seekBar) {}
			
			@Override
			public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {}
		});
	}
	
	/** Update the InputLayouts' list and save the modification done by the user */
	public void refreshAndSaveLayoutList(){
		layout_list_adapter.notifyDataSetChanged();
		ButtonMappingModel.writeButtonMappingFile(mapping_model);
	}
	
	/** Open a dialog box to add an InputLayout */
	public void addAnInputLayout(View view) {
		final EditText input = new EditText(this);
		//TODO : Restrict the edit text to alpha numeric characters

		// The dialog
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder
			.setTitle(R.string.add_an_input_layout)
			.setView(input)
			.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					String text = input.getText().toString();
					if (!text.isEmpty()) {
						InputLayout layout = new InputLayout(text);
						layout.setButton_list(ButtonMappingModel.InputLayout.getDefaultButtonList(getApplicationContext()));
						mapping_model.add(layout);
	
						refreshAndSaveLayoutList();
					}
				}
			})
			.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					dialog.cancel();
				}
			})
		;
		builder.show();
	}

	/** Open a dialog box to configure an InputLayout */
	public void configureInputLayout(final InputLayout game_layout) {
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		
		String[] choiceArray = {getString(R.string.set_as_default), 
								getString(R.string.edit_name), 
								getString(R.string.edit_layout),
								getString(R.string.delete)};
		
		builder
			.setTitle(game_layout.getName())
			.setItems(choiceArray, new DialogInterface.OnClickListener() {
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
			})
		;
		
		builder.show();
	}

	/** Open a dialog box to configure an InputLayout's name */
	public void editInputLayoutName(final InputLayout game_layout) {
		// The editText field
		final EditText input = new EditText(this);
		input.setText(game_layout.getName());
		
		//The dialog box
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder
			.setTitle(R.string.edit_name)
			.setView(input)
			.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					String text = input.getText().toString();
					if (!text.isEmpty()) {
						game_layout.setName(text);
					}
					refreshAndSaveLayoutList();
				}
			})
			.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					dialog.cancel();
				}
			})
		;
		builder.show();
	}

	/** Edit an InputLayout by opening the ButtonMapping activity */
	public void editInputLayout(final InputLayout game_layout) {
		Intent intent = new Intent(this, org.easyrpg.player.button_mapping.ButtonMappingActivity.class);
		intent.putExtra(ButtonMappingActivity.TAG_ID, game_layout.getId());
		startActivity(intent);
	}

	/** Delete an InputLayout */
	public void delete_layout(final ButtonMappingModel.InputLayout game_layout){
		//TODO : Ask confirmation
		mapping_model.delete(this, game_layout);
		refreshAndSaveLayoutList();
	}
	
	/** Load user preferences */
	public static void updateUserPreferences(Context context){
		SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(context);
		VIBRATION = sharedPref.getBoolean(context.getString(R.string.pref_enable_vibration), true);
		LAYOUT_TRANSPARENCY = sharedPref.getInt(context.getString(R.string.pref_layout_transparency), 100); 
	}
	
	/** The Adapter used to display the InputLayout list */ 
	private class InputLayoutListAdapter extends BaseAdapter {
		private LinkedList<InputLayout> layout_list;
		private LayoutInflater inflater;

		public InputLayoutListAdapter(LinkedList<InputLayout> layout_list) {
			this.layout_list = layout_list;
			this.inflater = getLayoutInflater();
		}
		
		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			ViewHolder holder = null;

			// If the view is new (not recycled)
			if (convertView == null) {
				convertView = inflater.inflate(R.layout.controls_settings_item_list, null);
				holder = new ViewHolder();

				// Place widgets in holder
				holder.name = (TextView) convertView.findViewById(R.id.controls_settings_preset_name);
				holder.option_button = (ImageButton) convertView
						.findViewById(R.id.controls_settings_preset_option_button);

				// Put the holder in the layout as a tag
				convertView.setTag(holder);
			} else {
				// Si on recycle la vue, on récupère son holder en tag
				holder = (ViewHolder) convertView.getTag();
			}

			// Get and configure the proper layout
			final InputLayout input_layout = (ButtonMappingModel.InputLayout) getItem(position);
			if (input_layout != null) {
				// The name (+ "is the default layout" indication)
				if(input_layout.isDefaultInputLayout(mapping_model)){
					holder.name.setText(input_layout.getName() + " (" + getString(R.string.default_layout) + ")");
				}
				else{
					holder.name.setText(input_layout.getName());
				}
				
				//Configuration Button
				holder.option_button.setOnClickListener(new OnClickListener() {  
					@Override
					public void onClick(View v) {
						configureInputLayout(input_layout);
					}
				});
				
				//Call the Edit Layout feature when clicking on the view
				convertView.setOnClickListener(new OnClickListener() {
					
					@Override
					public void onClick(View v) {
						editInputLayout(input_layout);
					}
				});
			}
			return convertView;
		}
		
		@Override
		public int getCount() {
			return layout_list.size();
		}

		@Override
		public Object getItem(int position) {
			return layout_list.get(position);
		}

		@Override
		public long getItemId(int position) {
			return layout_list.get(position).getId();
		}
	}

	/** ViewHolder is used as a pattern programming for optimizations purposes */
	static class ViewHolder {
		public TextView name;
		public ImageButton option_button;

	}
}
