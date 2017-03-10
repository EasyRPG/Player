package org.easyrpg.player.button_mapping;

import android.content.Context;
import android.util.Log;

import org.easyrpg.player.Helper;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedWriter;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

public class ButtonMappingManager {
    public static final int NUM_VERSION = 1;
    public static final String TAG_VERSION = "version", TAG_PRESETS = "presets", TAG_DEFAULT_LAYOUT = "default",
            DEFAULT_NAME = "RPG Maker 2000", TAG_ID = "id", TAG_NAME = "name", TAG_BUTTONS = "buttons",
            TAG_KEYCODE = "keycode", TAG_X = "x", TAG_Y = "y", TAG_SIZE = "size";
    public static final String FILE_NAME = "button_mapping.txt";

    private List<InputLayout> layoutList = new ArrayList<>();
    private int idDefaultLayout;
    private Context context;

    // Singleton pattern
    private static volatile ButtonMappingManager instance = null;

    private ButtonMappingManager() {
    }

    public final static ButtonMappingManager getInstance(Context context) {
        if (ButtonMappingManager.instance == null) {
            synchronized (ButtonMappingManager.class) {
                if (ButtonMappingManager.instance == null) {
                    ButtonMappingManager.instance = readButtonMappingFile(context);
                }
            }
        }
        ButtonMappingManager.instance.context = context;
        return ButtonMappingManager.instance;
    }

    /**
     * Add an input layout, add it as the default one if the list was empty
     */
    public void add(InputLayout p) {
        layoutList.add(p);

        // Set the default layout if there is no one
        if (layoutList.size() == 1) {
            setDefaultLayout(p.getId());
        }
    }

    /**
     * Delete safely a layout : handle problems of empty layout list and suppression of default layout
     */
    public void delete(Context context, InputLayout p) {
        // Remove p
        layoutList.remove(p);

        // If p was the last layout : add the default layout
        if (layoutList.size() <= 0) {
            add(InputLayout.getDefaultInputLayout(context));
        }

        // If p was the default layout : the first layout become the new default layout
        if (p.id == idDefaultLayout) {
            idDefaultLayout = layoutList.get(0).getId();
        }

        // Save the result
        save();
    }

    public InputLayout getLayoutById(int id) {
        // The layout exist : return it
        for (InputLayout i : layoutList) {
            if (i.getId() == id)
                return i;
        }

        // The layout doesn't exist : return the default one
        return getLayoutById(idDefaultLayout);
    }

    public void setDefaultLayout(int id) {
        // TODO : Verify if the id is in the input layout's list
        idDefaultLayout = id;
    }

    public int getDefaultLayoutId() {
        return idDefaultLayout;
    }

    public List<InputLayout> getLayoutList() {
        return layoutList;
    }

    /**
     * Return the list of the input layout's name
     */
    public String[] getLayoutsNames() {
        String[] layoutNameArray = new String[layoutList.size()];
        for (int i = 0; i < layoutNameArray.length; i++) {
            layoutNameArray[i] = layoutList.get(i).getName();
        }
        return layoutNameArray;
    }


    /**
     * Convert the current input layout model in a JSON Object
     */
    private JSONObject serialize() {
        JSONObject o = new JSONObject();

        try {
            JSONArray presets = new JSONArray();
            for (InputLayout p : layoutList) {
                presets.put(p.serialize());
            }

            o.put(TAG_VERSION, NUM_VERSION);
            o.put(TAG_DEFAULT_LAYOUT, idDefaultLayout);
            o.put(TAG_PRESETS, presets);
        } catch (JSONException e) {
            Log.e("Button Maping Model", "Impossible to serialize the button mapping model");
        }

        return o;
    }

    public void save() {
        writeButtonMappingFile(this.context, this);
    }

    /**
     * Return the default Button Mapping model
     */
    private static ButtonMappingManager getDefaultButtonMapping(Context context) {
        ButtonMappingManager m = new ButtonMappingManager();
        m.add(InputLayout.getDefaultInputLayout(context));
        m.idDefaultLayout = 0;
        return m;
    }

    private static ButtonMappingManager readButtonMappingFile(Context context) {
        try {
            // Parse the JSON
            String text = Helper.readInternalFileContent(context, FILE_NAME);
            JSONObject jso = Helper.readJSON(text);

            if (jso == null) {
                Log.i("Button Mapping Model", "No " + FILE_NAME + " found, loading the default Button Mapping System");
                return getDefaultButtonMapping(context);
            }

            // Extract the input layouts list et construct the button mapping
            ButtonMappingManager m = new ButtonMappingManager();
            m.context = context;
            JSONArray layout_array = jso.getJSONArray("presets");
            JSONObject p;
            for (int i = 0; i < layout_array.length(); i++) {
                p = (JSONObject) layout_array.get(i);
                m.add(InputLayout.deserialize(context, p));
            }

            // Default layout
            // TODO : Verify that the default layout exists
            m.setDefaultLayout(jso.getInt(TAG_DEFAULT_LAYOUT));

            return m;
        } catch (JSONException e) {
            Log.e("Button Mapping Model", "Error parsing the Button Mapping file, loading the default one");
            return getDefaultButtonMapping(context);
        }
    }

    private static void writeButtonMappingFile(Context context, ButtonMappingManager m) {
        try {
            // FileWriter file = new FileWriter(button_mapping_path);
            FileOutputStream fos = context.openFileOutput(FILE_NAME, Context.MODE_PRIVATE);
            BufferedWriter file = new BufferedWriter(new OutputStreamWriter(fos));
            JSONObject obj = m.serialize();
            file.write(obj.toString(2));
            file.flush();
            file.close();
            Log.i("Button Mapping Model", "File writed with success");
        } catch (IOException e) {
            Log.e("Button Mapping Model", "Error writting the button mapping file");
        } catch (JSONException e) {
            Log.e("Button Mapping Model", "Error parsing the button mapping file for writing");
        }

    }

    public static class InputLayout {
        private String name;
        private int id;
        private List<VirtualButton> buttonList = new ArrayList<>();

        public InputLayout(String name) {
            this.name = name;
            // TODO : Verify that id hasn't been already taken (like 0,00000001% probability)
            this.id = (int) (Math.random() * 100000000);
        }

        public InputLayout(String name, int id) {
            this(name);
            this.id = id;
        }

        public void add(VirtualButton v) {
            buttonList.add(v);
        }

        public JSONObject serialize() {
            try {
                JSONObject preset = new JSONObject();

                preset.put(TAG_NAME, name);
                preset.put(TAG_ID, id);

                // Circle/Buttons
                JSONArray layout_array = new JSONArray();
                for (VirtualButton button : buttonList) {
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
                    } else if (keyCode == MenuButton.MENU_BUTTON_KEY) {
                        layout.add(new MenuButton(context, posX, posY, size));
                    } else {
                        layout.add(VirtualButton.Create(context, keyCode, posX, posY, size));
                    }
                }

                return layout;
            } catch (JSONException e) {
                Log.e("Button Mapping File", "Error while deserializing an input layout : " + e.getMessage());
            }

            return null;
        }

        /**
         * Return the default button mapping preset : one cross, two buttons
         */
        public static InputLayout getDefaultInputLayout(Context context) {
            InputLayout b = new InputLayout(DEFAULT_NAME, 0);
            b.setButtonList(getDefaultButtonList(context));
            return b;
        }

        public static LinkedList<VirtualButton> getDefaultButtonList(Context context) {
            LinkedList<VirtualButton> l = new LinkedList<VirtualButton>();
            l.add(new VirtualCross(context, 0.0, 0.5, 100));
            l.add(VirtualButton.Create(context, VirtualButton.ENTER, 0.80, 0.7, 100));
            l.add(VirtualButton.Create(context, VirtualButton.CANCEL, 0.90, 0.6, 100));
            l.add(new MenuButton(context, 0, 0, 90));

            return l;
        }

        public boolean isDefaultInputLayout(ButtonMappingManager bmm) {
            return id == bmm.getDefaultLayoutId();
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

        public List<VirtualButton> getButtonList() {
            return buttonList;
        }

        public void setButtonList(List<VirtualButton> buttonList) {
            this.buttonList = buttonList;
        }
    }
}
