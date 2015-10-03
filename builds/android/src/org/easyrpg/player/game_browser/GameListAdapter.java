package org.easyrpg.player.game_browser;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.LinkedList;

import org.easyrpg.player.R;
import org.easyrpg.player.button_mapping.ButtonMappingModel;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageButton;
import android.widget.RadioButton;
import android.widget.TextView;
import android.widget.Toast;

public class GameListAdapter extends BaseAdapter {
	Context context;
	LinkedList<ProjectInformation> project_list;
	LayoutInflater inflater;
	IniEncodingReader iniReader;
	
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
				String[] choices_list = { context.getResources().getString(R.string.select_game_region), context.getString(R.string.change_the_layout)};
				
				AlertDialog.Builder builder = new AlertDialog.Builder(context);
				builder
					.setTitle(R.string.settings)
					.setItems(choices_list, new OnClickListener() {
						@Override
						public void onClick(DialogInterface dialog, int which) {
							switch (which) {
							case 0:
								chooseRegion(context, project_list.get(position));
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
	
	public void chooseLayout(final Context context, final ProjectInformation pi){
		final ButtonMappingModel bmm = ButtonMappingModel.getButtonMapping(context);
		String[] layout_name_array = bmm.getLayoutsNames(context);
		
		//Detect default layout
		pi.read_project_preferences(bmm);
		int id = -1;
		for(int i = 0; i < bmm.getLayout_list().size(); i++){
			if(bmm.getLayout_list().get(i).getId() == pi.getId_input_layout()){
				id = i;
				break;
			}
		}
		
		final ArrayList<Integer> selected = new ArrayList<Integer>();
		AlertDialog.Builder builder = new AlertDialog.Builder(context);
		builder
			.setTitle(R.string.choose_layout)
			.setSingleChoiceItems(layout_name_array, id, new OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int which) {
					selected.clear();
					selected.add(Integer.valueOf(which));
				}
			})
			.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int id) {
					if(!selected.isEmpty()){
						pi.setId_input_layout(bmm.getLayout_list().get(selected.get(0)).getId());
						pi.write_project_preferences();
					}
				}
			})
			.setNegativeButton(R.string.cancel, null);
		builder.show();
	}
	
	public void chooseRegion(final Context context, final ProjectInformation pi){
		//The list of region choices
		String[] region_array = {
			context.getString(R.string.autodetect),
			context.getString(R.string.west_europe),
			context.getString(R.string.east_europe),
			context.getString(R.string.japan),
			context.getString(R.string.cyrillic),
			context.getString(R.string.korean),
			context.getString(R.string.chinese_simple),
			context.getString(R.string.chinese_traditional),
			context.getString(R.string.greek),
			context.getString(R.string.turkish),
			context.getString(R.string.baltic)
		};
		
		//Retrieve the project's .ini file
		File iniFile = GameBrowserHelper.getIniOfGame(pi.getPath(), true);
		String error_msg = context.getString(R.string.accessing_configuration_failed).replace("$PATH", pi.getTitle());
		if (iniFile == null) {
			Toast.makeText(context, error_msg, Toast.LENGTH_LONG).show();
			return;
		}
		
		// Retrieve the current region (to check the correct radio button)
		iniReader = null;
		try {
			iniReader = new IniEncodingReader(iniFile);
		} catch (IOException e) {
			Toast.makeText(context, error_msg, Toast.LENGTH_LONG).show();
		}
		String encoding = iniReader.getEncoding();
		int id = -1;
					
		if (encoding == null) {
			id = 0;
		} else if (encoding.equals("1252")) {
			id = 1;
		} else if (encoding.equals("1250")) {
			id = 2;
		} else if (encoding.equals("932")) {
			id = 3;
		} else if (encoding.equals("1251")) {
			id = 4;
		} else if (encoding.equals("949")) {
			id = 5;
		} else if (encoding.equals("936")) {
			id = 6;
		} else if (encoding.equals("950")) {
			id = 7;
		} else if (encoding.equals("1253")) {
			id = 8;
		} else if (encoding.equals("1254")) {
			id = 9;
		} else if (encoding.equals("1257")) {
			id = 10;
		}
		
		if (id == -1) {
			Toast.makeText(context, context.getString(R.string.unknown_region), Toast.LENGTH_LONG).show();
		}
		
		//Building the dialog
		AlertDialog.Builder builder = new AlertDialog.Builder(context);
		builder
			.setTitle(R.string.select_game_region)
			.setSingleChoiceItems(region_array, id, null)
			.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
				@Override
				public void onClick(DialogInterface dialog, int id) {
					int selectedPosition = ((AlertDialog)dialog).getListView().getCheckedItemPosition();
					String encoding = null;
					
					if (selectedPosition == 0) {
						encoding = "auto";
					} else if (selectedPosition == 1) {
						encoding = "1252";
					} else if (selectedPosition == 2) {
						encoding = "1250";
					} else if (selectedPosition == 3) {
						encoding = "932";
					} else if (selectedPosition == 4) {
						encoding = "1251";
					} else if (selectedPosition == 5) {
						encoding = "949";
					} else if (selectedPosition == 6) {
						encoding = "936";
					} else if (selectedPosition == 7) {
						encoding = "950";
					} else if (selectedPosition == 8) {
						encoding = "1253";
					} else if (selectedPosition == 9) {
						encoding = "1254";
					} else if (selectedPosition == 10) {
						encoding = "1257";
					}
					
					if (encoding != null) {
						if (encoding.equals("auto")) {
							iniReader.deleteEncoding();
						} else {
							iniReader.setEncoding(encoding);
						}
						try {
							iniReader.save();
						} catch (IOException e) {
							Toast.makeText(context, context.getString(R.string.region_modification_failed), Toast.LENGTH_LONG).show();
						}
					}	
				}
			})
			.setNegativeButton(R.string.cancel, null);
		builder.show();		
	}
}
