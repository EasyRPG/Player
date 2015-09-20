package org.easyrpg.player;

import java.util.LinkedList;

import org.easyrpg.player.R;
import org.easyrpg.player.button_mapping.ButtonMappingActivity;
import org.easyrpg.player.button_mapping.ButtonMappingModel;
import org.easyrpg.player.button_mapping.ButtonMappingModel.InputLayout;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class PreferencesActivity extends Activity {
	ButtonMappingModel mappingModel;
	ListView layout_list_view;
	ListAdapter layout_list_adapter;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.controls_settings_activity);

		mappingModel = ButtonMappingModel.getButtonMapping(this);

		// Configure the game_layout list
		layout_list_view = (ListView) findViewById(R.id.controls_settings_layout_list);
		layout_list_adapter = new LayoutAdapter(mappingModel.getLayout_list());
		layout_list_view.setAdapter(layout_list_adapter);
	}
	
	public void refreshAndSaveLayoutList(){
		layout_list_view.invalidate();
		ButtonMappingModel.writeButtonMappingFile(mappingModel);
	}
	
	public void addAButton(View view) {
		final EditText input = new EditText(this);

		// The dialog
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder
			.setTitle("Add an input layout")
			.setView(input)
			.setPositiveButton("OK", new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					String text = input.getText().toString();
					if (!text.isEmpty()) {
						InputLayout layout = new InputLayout(text);
						layout.setButton_list(ButtonMappingModel.InputLayout.getDefaultButtonList(getApplicationContext()));
						mappingModel.add(layout);
	
						refreshAndSaveLayoutList();
					}
				}
			})
			.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					dialog.cancel();
				}
		});
		builder.show();
	}

	public void configureButton(final ButtonMappingModel.InputLayout game_layout) {
		String[] choiceArray = { "Set as default", "Edit Name", "Edit the Layout", "Delete"};
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setTitle(game_layout.getName()).setItems(choiceArray, new DialogInterface.OnClickListener() {
			public void onClick(DialogInterface dialog, int which) {
				switch (which) {
				case 0:
					mappingModel.setDefaultLayout(game_layout.getId());
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

	public void editInputLayoutName(final ButtonMappingModel.InputLayout game_layout) {
		// The editText field
		final EditText input = new EditText(this);
		input.setText(game_layout.getName());
		
		//The dialog box
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder
			.setTitle("Edit the name")
			.setView(input)
			.setPositiveButton("OK", new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					String text = input.getText().toString();
					if (!text.isEmpty()) {
						game_layout.setName(text);
					}
					refreshAndSaveLayoutList();
				}
			})
			.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					dialog.cancel();
				}
		});

		builder.show();
	}

	public void editInputLayout(final ButtonMappingModel.InputLayout game_layout) {
		Intent intent = new Intent(this, org.easyrpg.player.button_mapping.ButtonMappingActivity.class);
		intent.putExtra(ButtonMappingActivity.TAG_ID, game_layout.getId());
		startActivity(intent);
	}

	public void delete_layout(final ButtonMappingModel.InputLayout game_layout){
		//TODO : Ask confirmation
		mappingModel.delete(game_layout);
		refreshAndSaveLayoutList();
	}
	
	private class LayoutAdapter extends BaseAdapter {
		LinkedList<ButtonMappingModel.InputLayout> layout_list;
		LayoutInflater inflater;

		public LayoutAdapter(LinkedList<ButtonMappingModel.InputLayout> layout_list) {
			this.layout_list = layout_list;
			inflater = getLayoutInflater();
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

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			ViewHolder holder = null;

			// If the view is not recycled
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
			final ButtonMappingModel.InputLayout game_layout = (ButtonMappingModel.InputLayout) getItem(position);
			if (game_layout != null) {
				holder.name.setText(game_layout.getName());
				
				//Configuration Button
				holder.option_button.setOnClickListener(new OnClickListener() {  
					@Override
					public void onClick(View v) {
						configureButton(game_layout);
					}
				});
			}
			return convertView;
		}

	}

	static class ViewHolder {
		public TextView name;
		public ImageButton option_button;

	}
}
