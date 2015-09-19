package org.easyrpg.player.button_mapping;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.LinkedList;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.content.Context;
import android.os.Environment;
import android.util.Log;

public class ButtonMappingModel {
	public LinkedList<InputLayout> layout_list;

	public static final int NUM_VERSION = 1;
	public static final String TAG_VERSION = "version", TAG_PRESETS = "presets", DEFAULT_NAME = "default",
			TAG_ID = "id", TAG_NAME = "name", TAG_BUTTONS = "buttons", TAG_KEYCODE = "keycode", TAG_X = "x",
			TAG_Y = "y", TAG_SIZE = "size";
	public static final String FILE_NAME = "button_mapping.txt";

	public ButtonMappingModel() {
		layout_list = new LinkedList<InputLayout>();
	}

	public void add(InputLayout p) {
		layout_list.add(p);
	}

	public JSONObject serialize() {
		JSONObject o = new JSONObject();

		try {
			JSONArray presets = new JSONArray();
			for (InputLayout p : this.layout_list) {
				presets.put(p.serialize());
			}

			o.put(TAG_VERSION, NUM_VERSION);
			o.put(TAG_PRESETS, presets);
		} catch (JSONException e) {
			Log.e("Button Maping Model", "Impossible to serialize the button mapping model");
		}

		return o;
	}

	public static ButtonMappingModel getDefaultButtonMappingModel(Context context) {
		ButtonMappingModel m = new ButtonMappingModel();
		m.add(InputLayout.getDefaultPreset(context));
		return m;
	}

	public static ButtonMappingModel getButtonMapping(Context context) {
		String button_mapping_path = Environment.getExternalStorageDirectory().getPath() + "/easyrpg/" + FILE_NAME;
		return readButtonMappingFile(context, button_mapping_path);
	}

	public static ButtonMappingModel readButtonMappingFile(Context context, String path) {
		ButtonMappingModel m = new ButtonMappingModel();

		String file = new String(), tmp;
		try {
			// Read the file
			BufferedReader bf = new BufferedReader(new FileReader(new File(path)));
			while ((tmp = bf.readLine()) != null) {
				file += tmp;
			}
			bf.close();

			// Parse the JSON
			JSONObject jso = new JSONObject(file);

			// Presets' extraction
			JSONArray layout_array = jso.getJSONArray("presets");
			JSONObject p;
			for (int i = 0; i < layout_array.length(); i++) {
				p = (JSONObject) layout_array.get(i);
				m.add(InputLayout.deserialize(context, p));
			}

			return m;
		} catch (JSONException e) {
			Log.e("Button Mapping Model", "Error parsing de JSO file, loading the default one");
		} catch (IOException e) {
			Log.e("Button Mapping Model", "Error reading the button mapping file, loading the default one");
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

	public LinkedList<InputLayout> getLayout_list() {
		return layout_list;
	}

	public static class InputLayout {
		private String name;
		private int id;
		private LinkedList<VirtualButton> button_list = new LinkedList<VirtualButton>();

		public InputLayout(String name) {
			this.name = name;
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
			} catch (JSONException e) {
				Log.e("Button Mapping File", "Error while serializing an input layout : " + e.getLocalizedMessage());
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
				Log.e("Button Mapping File", "Error while deserializing an input layout : " + e.getLocalizedMessage());
			}

			return null;
		}

		/** Return the default button mapping preset : one cross, two buttons */
		public static InputLayout getDefaultPreset(Context context) {
			InputLayout b = new InputLayout(DEFAULT_NAME);

			b.add(new VirtualCross(context, 0.0, 0.5, 100));
			b.add(new VirtualButton(context, VirtualButton.ENTER, 0.80, 0.7, 100));
			b.add(new VirtualButton(context, VirtualButton.CANCEL, 0.90, 0.6, 100));

			return b;
		}

		public String getName() {
			return name;
		}

		public int getId() {
			return id;
		}

		public LinkedList<VirtualButton> getButton_list() {
			return button_list;
		}
	}
}
