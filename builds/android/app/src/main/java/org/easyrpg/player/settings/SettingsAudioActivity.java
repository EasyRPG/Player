package org.easyrpg.player.settings;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.provider.DocumentsContract;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.documentfile.provider.DocumentFile;

import org.easyrpg.player.BaseActivity;
import org.easyrpg.player.Helper;
import org.easyrpg.player.R;

import java.util.ArrayList;
import java.util.List;

public class SettingsAudioActivity extends BaseActivity {
    private LinearLayout soundfontsListLayout;
    List<SoundfontItemList> soundfontList;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings_audio);

        soundfontsListLayout = findViewById(R.id.settings_sound_fonts_list);

        // Setup UI components
        // The Soundfont Button
        Button button = this.findViewById(R.id.button_open_soundfont_folder);
        Helper.attachOpenFolderButton(this, button, SettingsManager.getSoundFontsFolderURI(this));

        configureMusicVolume();
        configureSoundVolume();
    }

    @Override
    protected void onResume() {
        super.onResume();

       updateSoundfontsListView();
    }

    private void updateSoundfontsListView() {
        soundfontsListLayout.removeAllViews();

        boolean thereIsASelectedSoundfont = false;
        soundfontList = scanAvailableSoundfonts();
        for (SoundfontItemList i : soundfontList) {
            soundfontsListLayout.addView(i.getRadioButton());
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

        Uri soundFontsFolder = SettingsManager.getSoundFontsFolderURI(this);
        if (soundFontsFolder != null) {
            for (String[] array : Helper.listChildrenDocuments(this, soundFontsFolder)) {
                String fileDocumentID = array[0];
                String fileDocumentType = array[1];
                String name = array[2];

                // Is it a soundfont file ?
                boolean isDirectory = Helper.isDirectoryFromMimeType(fileDocumentType);
                if (!isDirectory && (name.toLowerCase().endsWith(".sf2") || name.toLowerCase().endsWith(".soundfont"))) {
                    DocumentFile soundFontFile = Helper.getFileFromDocumentID(this, soundFontsFolder, fileDocumentID);
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
        Uri selectedSoundFontUri = SettingsManager.getSoundFontFileURI();
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
        private final RadioButton radioButton;

        public SoundfontItemList(Context context, String name, Uri uri) {
            this.name = name;
            this.uri = uri;

            // The Radio Button
            View layout = getLayoutInflater().inflate(R.layout.settings_soundfont_item_list, null);
            this.radioButton = layout.findViewById(R.id.settings_soundfont_radio_button);
            radioButton.setOnClickListener(v -> select());
            if (isSelectedSoundfontFile(context, uri)) {
                setSelected(true);
            }

            // The name
            radioButton.setText(name);
            radioButton.setOnClickListener(v -> select());
        }

        public void select() {
            SettingsManager.setSoundFontFileURI(uri);

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

    private void configureMusicVolume() {
        SeekBar musicVolumeSeekBar = findViewById(R.id.settings_music_volume);
        musicVolumeSeekBar.setProgress(SettingsManager.getMusicVolume());

        TextView t = findViewById(R.id.settings_music_volume_text_view);

        musicVolumeSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                SettingsManager.setMusicVolume(seekBar.getProgress() );
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                t.setText(String.valueOf(musicVolumeSeekBar.getProgress()));
            }
        });

        t.setText(String.valueOf(musicVolumeSeekBar.getProgress()));
    }

    private void configureSoundVolume() {
        SeekBar soundVolumeSeekBar = findViewById(R.id.settings_sound_volume);
        soundVolumeSeekBar.setProgress(SettingsManager.getSoundVolume());

        TextView t = findViewById(R.id.settings_sound_volume_text_view);

        soundVolumeSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                SettingsManager.setSoundVolume(seekBar.getProgress() );
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                t.setText(String.valueOf(soundVolumeSeekBar.getProgress()));
            }
        });

        t.setText(String.valueOf(soundVolumeSeekBar.getProgress()));
    }
}

