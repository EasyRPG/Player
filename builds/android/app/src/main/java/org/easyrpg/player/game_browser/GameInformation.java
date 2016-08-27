package org.easyrpg.player.game_browser;

import android.util.Log;

import org.easyrpg.player.Helper;
import org.easyrpg.player.button_mapping.ButtonMappingManager;
import org.easyrpg.player.settings.SettingsManager;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileWriter;

public class GameInformation {
	public static final String TAG_ID_INPUT_LAYOUT = "layout_id";
	public static final String TAG_ENCODING = "encoding";
	public static final String preferenceFileName = "easyrpg-pref.txt";
	private int id_input_layout = -1;
	private String encoding = "";

	private String title, gameFolderPath, savePath;

	public GameInformation(String gameFolderPath) {
		this.gameFolderPath = gameFolderPath;
		File f = new File(gameFolderPath);
		
		if (GameBrowserHelper.canWrite(f)) {
			this.savePath = gameFolderPath;
		} else {
			// Not writable, redirect to a different path
			// Try preventing collisions by using the names of the two parent directories
			String savename = f.getParentFile().getName() + "/" + f.getName();  
			savePath = SettingsManager.getEasyRPGFolder() + "/Save/" + savename;
			new File(savePath).mkdirs();
		}
	}
	
	public GameInformation(String title, String gameFolderPath) {
		this(gameFolderPath);
		this.title = title;
	}

	public String getTitle() {
		return title;
	}

	public String getGameFolderPath() {
		return gameFolderPath;
	}
	
	public String getSavePath() {
		return savePath;
	}

	/** Set the inputLayout preferences depending on the preferences file */
	public boolean getProjectInputLayout(ButtonMappingManager bmm) {
        // Try to obtain the preferences file.
		JSONObject jso = Helper.readJSONFile(savePath + "/" + preferenceFileName);
		if (jso == null) {
			return false;
		}
		
		try {
			this.id_input_layout = jso.getInt(TAG_ID_INPUT_LAYOUT);
		} catch (JSONException e) {
			this.id_input_layout = bmm.getDefaultLayoutId();
			return false;
		}
		return true;
	}
	
	public boolean read_project_preferences_encoding() {
		JSONObject jso = Helper.readJSONFile(savePath + "/" + preferenceFileName);
		if (jso == null) {
			return false;
		}
		
		try {
			encoding = jso.getString(TAG_ENCODING);
		} catch (JSONException e) {
			return false;
		}
		
		return true;
	}

	public void write_project_preferences() {
		try {
			FileWriter file = new FileWriter(savePath + "/" + preferenceFileName);
			JSONObject o = new JSONObject();

			if (id_input_layout != -1) {
				o.put(TAG_ID_INPUT_LAYOUT, id_input_layout);
			}
			
			if (encoding.length() > 0) {
				o.put(TAG_ENCODING, encoding);
			}
			
			file.write(o.toString(2));
			file.flush();
			file.close();
		} catch (Exception e) {
			Log.e("Write pref project",
					"Error while writing preference project file : " + e.getLocalizedMessage());
		}

	}

	public int getId_input_layout() {
		return id_input_layout;
	}

	public void setId_input_layout(int id_input_layout) {
		this.id_input_layout = id_input_layout;
	}
	
	public String getEncoding() {
		return encoding;
	}

	public void setEncoding(String encoding) {
		this.encoding = encoding;
	}

    @Override
    public String toString() {
        return getTitle();
    }
}