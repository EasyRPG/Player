package org.easyrpg.player.button_mapping;

import java.util.LinkedList;

import org.easyrpg.player.R;

import android.app.Activity;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageButton;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;

public class ControlsActivity extends Activity {
	ButtonMappingModel mappingModel; 
	ListView layout_list_view;
	ListAdapter layout_list_adapter;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.controls_settings_activity);
		
		mappingModel = ButtonMappingModel.getButtonMapping(this);
		
		//Configure the game_layout list
		layout_list_view = (ListView) findViewById(R.id.controls_settings_layout_list);
		layout_list_adapter = new LayoutAdapter(mappingModel.getLayout_list());
		layout_list_view.setAdapter(layout_list_adapter);
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
				holder.option_button = (ImageButton) convertView.findViewById(R.id.controls_settings_preset_option_button);

				// Put the holder in the layout as a tag
				convertView.setTag(holder);
			} else {
				// Si on recycle la vue, on récupère son holder en tag
				holder = (ViewHolder) convertView.getTag();
			}

			//Get and configure the proper layout
			ButtonMappingModel.InputLayout game_layout = (ButtonMappingModel.InputLayout)getItem(position);
			if (game_layout != null) {
				holder.name.setText(game_layout.getName());
			}
			return convertView;
		}

	}

	static class ViewHolder {
		public TextView name;
		public ImageButton option_button;

	}
}
