package org.easyrpg.player.game_browser;

import java.util.LinkedList;

import org.easyrpg.player.R;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.TextView;

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
		
		// Region button
		ImageButton regionButton = (ImageButton)convertView.findViewById(R.id.game_browser_thumbnail_region_button);
		regionButton.setOnClickListener(new View.OnClickListener() {
			@Override
		    public void onClick(View v) {
		    	((GameBrowserActivity)context).regionButton(position);
		    }
		});
		
		// Define the click action
		convertView.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				((GameBrowserActivity)context).launchGame(position);
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
}
