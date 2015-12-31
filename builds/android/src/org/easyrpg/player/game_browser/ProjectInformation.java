package org.easyrpg.player.game_browser;

import java.io.File;
import java.io.FileWriter;

import org.easyrpg.player.Helper;
import org.easyrpg.player.SettingsActivity;
import org.easyrpg.player.button_mapping.ButtonMappingModel;
import org.json.JSONObject;

import android.util.Log;

public class ProjectInformation {
	public static final String TAG_ID_INPUT_LAYOUT = "layout_id", preferenceFileName = "easyrpg-pref.txt";
	public int id_input_layout = -1;
	
	private String title, path, save_path;

	public ProjectInformation(String path){
		this.path = path;
		File f = new File(path);
		
		if (f.canWrite()) {
			this.save_path = path;
		} else {
			// Not writable, redirect to a different path
			// Try preventing collisions by using the names of the two parent directories
			String savename = f.getParentFile().getName() + "/" + f.getName();  
			save_path = SettingsActivity.MAIN_DIRECTORY + "/Save/" + savename;
			new File(save_path).mkdirs();
		}
	}
	
	public ProjectInformation(String title, String path) {
		this(path);
		this.title = title;
	}

	public String getTitle() {
		return title;
	}

	public String getPath() {
		return path;
	}
	
	public String getSavePath() {
		return save_path;
	}

	public boolean read_project_preferences(ButtonMappingModel bmm) {
		JSONObject jso = Helper.readJSONFile(save_path + "/" + preferenceFileName);
		try {
			id_input_layout = jso.getInt(TAG_ID_INPUT_LAYOUT);
			return true;
		} catch (Exception e) {
			id_input_layout = bmm.getId_default_layout();
		}
		return false;
	}

	public void write_project_preferences() {
		try {
			FileWriter file = new FileWriter(save_path + "/" + preferenceFileName);
			JSONObject o = new JSONObject();

			o.put(TAG_ID_INPUT_LAYOUT, id_input_layout);

			file.write(o.toString(2));
			file.flush();
			file.close();
		} catch (Exception e) {
			Log.e("Writing preference project",
					"Error while writing preference project file : " + e.getLocalizedMessage());
		}

	}

	public int getId_input_layout() {
		return id_input_layout;
	}

	public void setId_input_layout(int id_input_layout) {
		this.id_input_layout = id_input_layout;
	}
}