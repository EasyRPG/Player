package org.easyrpg.player.game_browser;

import java.util.LinkedList;

import org.easyrpg.player.R;
import org.easyrpg.player.button_mapping.ButtonMappingModel;
import org.easyrpg.player.player.EasyRpgPlayerActivity;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

public class GameListAdapter extends BaseAdapter {
	Context context;
	LinkedList<ProjectInformation> project_list;
	LayoutInflater inflater;
	
	public GameListAdapter(Context context, LinkedList<ProjectInformation> project_list){
		this.context = context;
		this.project_list = project_list;
		
		inflater = (LayoutInflater) this.context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
	}
	
	@Override
	public View getView(final int position, View convertView, ViewGroup parent) {
		ProjectInformation game = project_list.get(position);
		
		if (convertView == null) {
            convertView = inflater.inflate(R.layout.game_browser_item_list, null);
        }
        
		// Title
		TextView titleView = (TextView)convertView.findViewById(R.id.game_browser_thumbnail_title);
		titleView.setText(game.getTitle());
		
		// Option button
		ImageButton optionButton = (ImageButton)convertView.findViewById(R.id.game_browser_thumbnail_option_button);
		optionButton.setOnClickListener(new View.OnClickListener() {
			@Override
		    public void onClick(View v) {
				String[] choices_list = { context.getResources().getString(R.string.select_game_region), "Change the layout"};
				
				AlertDialog.Builder builder = new AlertDialog.Builder(context);
				builder
					.setTitle("Preferences")
					.setItems(choices_list, new OnClickListener() {
						@Override
						public void onClick(DialogInterface dialog, int which) {
							switch (which) {
							case 0:
								GameBrowserHelper.regionButton(context, project_list.get(position));
								break;
							case 1:
								chooseLayout(context, project_list.get(position));
								break;
							default:
								break;
							}
						}
					});
				builder.show();
		    }
		});
		
		// Define the click action
		convertView.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				GameBrowserHelper.launchGame(context, project_list.get(position));
			}
		});
        return convertView;
	}
	
	@Override
	public int getCount() {
		return project_list.size();
	}

	@Override
	public Object getItem(int position) {
		return project_list.get(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}
	
	public void chooseLayout(Context context, final ProjectInformation pi){
		final ButtonMappingModel bmm = ButtonMappingModel.getButtonMapping(context);
		String[] layout_name_array = bmm.getLayoutsNames(context);
		
		AlertDialog.Builder builder = new AlertDialog.Builder(context);
		builder
			.setTitle("Choose a layout")
			.setItems(layout_name_array, new OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					pi.setId_input_layout(bmm.getLayout_list().get(which).getId());
					pi.write_project_preferences();
				}
			});
		builder.show();
	}
}
