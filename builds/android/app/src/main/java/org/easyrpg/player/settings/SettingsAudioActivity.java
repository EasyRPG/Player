package org.easyrpg.player.settings;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.provider.DocumentsContract;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.AppCompatSpinner;
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

        soundfontsListLayout = findViewById(R.id.settings_sound_fonts_list);

        SettingsManager.init(getApplicationContext());

        // Setup UI components
        // The Soundfont Button
        Button button = this.findViewById(R.id.button_open_soundfont_folder);
        // We can open the file picker in a specific folder only with API >= 26
        if (android.os.Build.VERSION.SDK_INT >= 26) {
            button.setOnClickListener(v -> {
                // Open the file explorer in the "soundfont" folder
                Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
                intent.setType("*/*");
                intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, SettingsManager.getSoundFontsFolderURI(this));
                startActivity(intent);
            });
        } else {
            ViewGroup layout = (ViewGroup) button.getParent();
            if(layout != null) {
                layout.removeView(button);
            }
        }

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
            for (String[] array : Helper.listChildrenDocumentIDAndType(this, soundFontsFolder)) {
                String fileDocumentID = array[0];
                String fileDocumentType = array[1];

                // Is it a soundfont file ?
                boolean isDirectory = Helper.isDirectoryFromMimeType(fileDocumentType);
                String name = Helper.getFileNameFromDocumentID(fileDocumentID);
                if (!isDirectory && name.toLowerCase().endsWith(".sf2")) {
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

