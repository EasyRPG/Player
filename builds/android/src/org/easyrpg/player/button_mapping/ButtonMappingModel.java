package org.easyrpg.player.button_mapping;

import java.io.FileWriter;
import java.io.IOException;
import java.util.LinkedList;

import org.easyrpg.player.Helper;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.content.Context;
import android.os.Environment;
import android.util.Log;

public class ButtonMappingModel {
	private LinkedList<InputLayout> layout_list;
	private int id_default_layout;

	public static final int NUM_VERSION = 1;
	public static final String TAG_VERSION = "version", TAG_PRESETS = "presets", TAG_DEFAULT_LAYOUT = "default",
			DEFAULT_NAME = "RPG Maker 2000", TAG_ID = "id", TAG_NAME = "name", TAG_BUTTONS = "buttons",
			TAG_KEYCODE = "keycode", TAG_X = "x", TAG_Y = "y", TAG_SIZE = "size";
	public static final String FILE_NAME = "button_mapping.txt";

	public ButtonMappingModel() {
		layout_list = new LinkedList<InputLayout>();
	}

	/** Add an input layout, add it as the default one if the list was empty */
	public void add(InputLayout p) {
		layout_list.add(p);
		
		//Set the default layout if there is no one
		if(layout_list.size() == 1){
			setDefaultLayout(p.getId());
		}
	}
	
	/** Delete safely a layout, handle problems of empty layout list or suppresion of default layout */ 
	public void delete(Context context, InputLayout p){
		int id_p = p.id;
		
		//Remove p
		layout_list.remove(p);
		
		//If p was the last layout : add the default layout
		if(layout_list.size() <= 0){
			add(InputLayout.getDefaultInputLayout(context));
		}
		
		//If p was the default layout : the first layout is the new default layout
		if(id_p == id_default_layout){
			id_default_layout = layout_list.getFirst().getId();
		}
	}

	public JSONObject serialize() {
		JSONObject o = new JSONObject();

		try {
			JSONArray presets = new JSONArray();
			for (InputLayout p : this.layout_list) {
				presets.put(p.serialize());
			}

			o.put(TAG_VERSION, NUM_VERSION);
			o.put(TAG_DEFAULT_LAYOUT, id_default_layout);
			o.put(TAG_PRESETS, presets);
		} catch (JSONException e) {
			Log.e("Button Maping Model", "Impossible to serialize the button mapping model");
		}

		return o;
	}

	public static ButtonMappingModel getDefaultButtonMappingModel(Context context) {
		ButtonMappingModel m = new ButtonMappingModel();
		m.add(InputLayout.getDefaultInputLayout(context));
		return m;
	}

	public static ButtonMappingModel getButtonMapping(Context context) {
		String button_mapping_path = Environment.getExternalStorageDirectory().getPath() + "/easyrpg/" + FILE_NAME;
		return readButtonMappingFile(context, button_mapping_path);
	}

	public String[] getLayoutsNames(Context context){
		//Create the layout array
		String[] layout_name_array = new String[this.layout_list.size()];
		for(int i = 0; i < layout_name_array.length; i++){
			layout_name_array[i] = this.layout_list.get(i).getName();
		}
		return layout_name_array;
	}
	
	public static ButtonMappingModel readButtonMappingFile(Context context, String path) {
		ButtonMappingModel m = new ButtonMappingModel();

		try {
			// Parse the JSON
			JSONObject jso = Helper.readJSONFile(path);
			if(jso == null){
				Log.i("Button Mapping Model", "No " + path + " file, loading the default Button Mapping System");
				return getDefaultButtonMappingModel(context);
			}

			// Presets' extraction
			JSONArray layout_array = jso.getJSONArray("presets");
			JSONObject p;
			for (int i = 0; i < layout_array.length(); i++) {
				p = (JSONObject) layout_array.get(i);
				m.add(InputLayout.deserialize(context, p));
			}
			
			// Default layout
			//TODO : Verify that this default layout exists in the list
			m.setDefaultLayout(jso.getInt(TAG_DEFAULT_LAYOUT));

			return m;
		} catch (JSONException e) {
			Log.e("Button Mapping Model", "Error parsing de JSO file, loading the default one");
		}

		return getDefaultButtonMappingModel(context);
	}

	public static void writeButtonMappingFile(ButtonMappingModel m) {
		String button_mapping_path = Environment.getExternalStorageDirectory().getPath() + "/easyrpg/" + FILE_NAME;

		try {
			FileWriter file = new FileWriter(button_mapping_path);
			JSONObject obj = m.serialize();
			file.write(obj.toString(2));
			file.flush();
			file.close();
		} catch (IOException e) {
			Log.e("Button Mapping Model", "Error writting the button mapping file");
		} catch (JSONException e) {
			Log.e("Button Mapping Model", "Error parsing the button mapping file for writing");
		}

	}

	public InputLayout getLayoutById(Context context, int id){
		//The layout exist : return it
		for(InputLayout i : layout_list){
			if(i.getId() == id)
				return i;
		}
		
		//The layout doesn't exist : return the default one
		return getLayoutById(context, id_default_layout);
	}
	
	public void setDefaultLayout(int id){
		//TODO : Verify if the id is in the input layout's list
		this.id_default_layout = id;
	}

	public int getId_default_layout() {
		return id_default_layout;
	}
	
	public LinkedList<InputLayout> getLayout_list() {
		return layout_list;
	}

	public static class InputLayout {
		private String name;
		private int id;
		private LinkedList<VirtualButton> button_list = new LinkedList<VirtualButton>();

		public InputLayout(String name) {
			this.name = name;
			// TODO : Verify that id hasn't been already taken (yeah, 0,00000001%
			// probability)
			this.id = (int) (Math.random() * 100000000);
		}

		public InputLayout(String name, int id) {
			this(name);
			this.id = id;
		}

		public void add(VirtualButton v) {
			button_list.add(v);
		}

		public JSONObject serialize() {
			try {
				JSONObject preset = new JSONObject();

				preset.put(TAG_NAME, name);
				preset.put(TAG_ID, id);

				// Circle/Buttons
				JSONArray layout_array = new JSONArray();
				for (VirtualButton button : button_list) {
					JSONObject jso = new JSONObject();
					jso.put(TAG_KEYCODE, button.getKeyCode());
					jso.put(TAG_X, button.getPosX());
					jso.put(TAG_Y, button.getPosY());
					jso.put(TAG_SIZE, button.getSize());

					layout_array.put(jso);
				}
				preset.put(TAG_BUTTONS, layout_array);

				return preset;
			} catch (JSONException e) {
				Log.e("Button Mapping File", "Error while serializing an input layout : " + e.getMessage());
			}

			return null;
		}

		public static InputLayout deserialize(Context context, JSONObject jso) {
			try {
				InputLayout layout = new InputLayout(jso.getString(TAG_NAME), jso.getInt(TAG_ID));

				JSONArray button_list = jso.getJSONArray(TAG_BUTTONS);
				for (int i = 0; i < button_list.length(); i++) {
					JSONObject button = (JSONObject) button_list.get(i);
					int keyCode = button.getInt(TAG_KEYCODE);
					int size = button.getInt(TAG_SIZE);
					double posX = button.getDouble(TAG_X);
					double posY = button.getDouble(TAG_Y);
					if (keyCode == VirtualButton.DPAD) {
						layout.add(new VirtualCross(context, posX, posY, size));
					} else {
						layout.add(new VirtualButton(context, keyCode, posX, posY, size));
					}
				}

				return layout;
			} catch (JSONException e) {
				Log.e("Button Mapping File", "Error while deserializing an input layout : " + e.getMessage());
			}

			return null;
		}

		/** Return the default button mapping preset : one cross, two buttons */
		public static InputLayout getDefaultInputLayout(Context context) {
			InputLayout b = new InputLayout(DEFAULT_NAME, 0);
			b.setButton_list(getDefaultButtonList(context));
			return b;
		}

		public static LinkedList<VirtualButton> getDefaultButtonList(Context context) {
			LinkedList<VirtualButton> l = new LinkedList<VirtualButton>();
			l.add(new VirtualCross(context, 0.0, 0.5, 100));
			l.add(new VirtualButton(context, VirtualButton.ENTER, 0.80, 0.7, 100));
			l.add(new VirtualButton(context, VirtualButton.CANCEL, 0.90, 0.6, 100));

			return l;
		}

		public String getName() {
			return name;
		}

		public void setName(String name) {
			this.name = name;
		}

		public int getId() {
			return id;
		}

		public LinkedList<VirtualButton> getButton_list() {
			return button_list;
		}

		public void setButton_list(LinkedList<VirtualButton> button_list) {
			this.button_list = button_list;
		}
	}
}
