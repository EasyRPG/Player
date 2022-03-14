package org.easyrpg.player.settings;

import android.content.Context;
import android.net.Uri;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RelativeLayout;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.documentfile.provider.DocumentFile;

import org.easyrpg.player.Helper;
import org.easyrpg.player.R;

import java.util.ArrayList;
import java.util.List;

public class SettingsAudioActivity extends AppCompatActivity {
    private LinearLayout soundfontsListLayout;
    List<SoundfontItemList> soundfontList;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings_audio);

        SettingsManager.init(getApplicationContext());

        // Update the EasyRPG folder path
        TextView soundFontExplanationView = (TextView) findViewById(R.id.settings_soundfont_explanation);
        String soundfontExplanation = soundFontExplanationView.getText().toString();
        Uri easyRPGFolderURI = SettingsManager.getEasyRPGFolderURI(this);
        if (easyRPGFolderURI != null) {
            String easyRPGFolderName = easyRPGFolderURI.getPath();
            soundfontExplanation = soundfontExplanation.replace("%", easyRPGFolderName);
            soundFontExplanationView.setText(soundfontExplanation);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

       updateSoundfontsListView();
    }

    private void updateSoundfontsListView() {
        if (soundfontsListLayout == null) {
            soundfontsListLayout = (LinearLayout) findViewById(R.id.settings_soundfonts_list);
        }
        soundfontsListLayout.removeAllViews();

        boolean thereIsASelectedSoundfont = false;
        soundfontList = scanAvailableSoundfonts();
        for (SoundfontItemList i : soundfontList) {
            soundfontsListLayout.addView(i.getLayout());
            if (i.isSelected()) {
                thereIsASelectedSoundfont = true;
            }
        }
        // If no soundfont is the selected soundfont, select the default one
        if (!thereIsASelectedSoundfont) {
            soundfontList.get(0).setSelected(true);
        }
    }

    private List<SoundfontItemList> scanAvailableSoundfonts(){
        List<SoundfontItemList> soundfontList = new ArrayList<>();
        soundfontList.add(getDefaultSoundfont(this));

        Uri soundfontsFolder = SettingsManager.getSoundfontsFolderURI(this);
        if (soundfontsFolder != null) {
            for (String[] array : Helper.listChildrenDocumentIDAndType(this, soundfontsFolder)) {
                String fileDocumentID = array[0];
                String fileDocumentType = array[1];

                // Is it a soundfont file ?
                boolean isDirectory = Helper.isDirectoryFromMimeType(fileDocumentType);
                String name = Helper.getFileNameFromDocumentID(fileDocumentID);
                if (!isDirectory && name.toLowerCase().endsWith(".sf2")) {
                    DocumentFile soundFontFile = Helper.getFileFromDocumentID(this, soundfontsFolder, fileDocumentID);
                    if (soundFontFile != null) {
                        soundfontList.add(new SoundfontItemList(this, name, soundFontFile.getUri()));
                    }
                }
            }
        }
        return soundfontList;
    }

    public SoundfontItemList getDefaultSoundfont(Context context) {
        return new SoundfontItemList(context, context.getString(R.string.settings_default_soundfont), null);
    }

    public static boolean isSelectedSoundfontFile(Context context, Uri soundfontUri) {
        Uri selectedSoundFontUri = SettingsManager.getSoundFountFileURI(context);
        if (soundfontUri == null && selectedSoundFontUri == null) {
            return true;
        }
        else if (soundfontUri != null) {
            return soundfontUri.equals(selectedSoundFontUri);
        } else {
            return false;
        }
    }

    class SoundfontItemList {
        private final String name;
        private final Uri uri;
        private final RelativeLayout layout;
        private final RadioButton radioButton;

        public SoundfontItemList(Context context, String name, Uri uri) {
            this.name = name;
            this.uri = uri;

            // Create the view
            LayoutInflater inflater = LayoutInflater.from(context);
            this.layout = (RelativeLayout) inflater.inflate(R.layout.settings_soundfont_item_list, null);

            // The Radio Button
            this.radioButton = (RadioButton) layout.findViewById(R.id.settings_soundfont_radio_button);
            radioButton.setOnClickListener(v -> {
                select();
            });
            if (isSelectedSoundfontFile(context, uri)) {
                setSelected(true);
            }

            // The name
            TextView nameTextView = (TextView) layout.findViewById(R.id.settings_soundfont_name);
            nameTextView.setText(name);
            nameTextView.setOnClickListener(v -> {
                select();
            });
        }

        public void select() {
            SettingsManager.setSoundFountFileURI(uri);

            // Uncheck other RadioButton
            for (SoundfontItemList s : soundfontList) {
                s.getRadioButton().setChecked(false);
            }
            radioButton.setChecked(true);
        }

        public String getName() {
            return name;
        }

        public Uri getUri() {
            return uri;
        }

        public RelativeLayout getLayout() {
            return layout;
        }

        public boolean isSelected() {
            return radioButton.isChecked();
        }

        public void setSelected(boolean selected) {
            radioButton.setChecked(selected);
        }

        public RadioButton getRadioButton() {
            return radioButton;
        }
    }
}


