package org.easyrpg.player.settings;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.net.Uri;
import android.os.Bundle;
import android.provider.DocumentsContract;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.SeekBar;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;
import androidx.documentfile.provider.DocumentFile;

import org.easyrpg.player.BaseActivity;
import org.easyrpg.player.Helper;
import org.easyrpg.player.R;
import org.libsdl.app.SDL;

import java.util.ArrayList;
import java.util.List;

public class SettingsFontActivity extends BaseActivity {
    private LinearLayout fonts1ListLayout;
    private LinearLayout fonts2ListLayout;
    private String[] extensions = new String[] {".fon", ".fnt", ".bdf", ".ttf", ".ttc", ".otf", ".woff2", ".woff"};

    List<FontItemList> font1List;
    List<FontItemList> font2List;
    private final int SIZE_MIN = 6;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings_font);

        fonts1ListLayout = findViewById(R.id.settings_font1_list);
        fonts2ListLayout = findViewById(R.id.settings_font2_list);

        SDL.setContext(getApplicationContext());

        // Setup UI components
        // The Font Button
        Button button = this.findViewById(R.id.button_open_font_folder);
        Helper.attachOpenFolderButton(this, button, SettingsManager.getFontsFolderURI(this));

        configureFont1Size();
        configureFont2Size();
    }

    @Override
    protected void onResume() {
        super.onResume();

        updateFontsListView();
    }

    private void updateFontsListView() {
        fonts1ListLayout.removeAllViews();
        fonts2ListLayout.removeAllViews();

        boolean font1Selected = false;
        boolean font2Selected = false;

        scanAvailableFonts();
        for (FontItemList i : font1List) {
            fonts1ListLayout.addView(i.getRadioButton());
            if (i.isSelected()) {
                font1Selected = true;
            }
        }
        for (FontItemList i : font2List) {
            fonts2ListLayout.addView(i.getRadioButton());
            if (i.isSelected()) {
                font2Selected = true;
            }
        }

        // If no font is selected, select the default one
        if (!font1Selected) {
            font1List.get(0).setSelected(true);
        }
        if (!font2Selected) {
            font2List.get(0).setSelected(true);
        }
    }

    private void updatePreview(boolean firstFont) {
        ImageView imageView;
        String font = "";
        int size;
        if (firstFont) {
            imageView = findViewById(R.id.settings_font1_preview);
            Uri fontUri = SettingsManager.getFont1FileURI();
            if (fontUri != null) {
                font = fontUri.toString();
            }
            size = SettingsManager.getFont1Size();
        } else {
            imageView = findViewById(R.id.settings_font2_preview);
            Uri fontUri = SettingsManager.getFont2FileURI();
            if (fontUri != null) {
                font = fontUri.toString();
            }
            size = SettingsManager.getFont2Size();
        }

        byte[] image = DrawText(font, size, firstFont);
        if (image != null) {
            Bitmap bitmap = Helper.createBitmapFromRGBA(image,304, 16 * 6);
            imageView.setImageBitmap(bitmap);
        }
    }

    private void scanAvailableFonts(){
        font1List = new ArrayList<>();
        font2List = new ArrayList<>();
        font1List.add(new FontItemList(this, this.getString(R.string.settings_font_default), null, true));
        font2List.add(new FontItemList(this, this.getString(R.string.settings_font_default), null, false));

        Uri fontsFolder = SettingsManager.getFontsFolderURI(this);
        if (fontsFolder != null) {
            for (String[] array : Helper.listChildrenDocuments(this, fontsFolder)) {
                String fileDocumentID = array[0];
                String fileDocumentType = array[1];
                String name = array[2];

                // Is it a font file ?
                boolean isDirectory = Helper.isDirectoryFromMimeType(fileDocumentType);
                String lname = name.toLowerCase();
                boolean fontOk = false;
                for (String ext: extensions) {
                    if (lname.endsWith(ext)) {
                        fontOk = true;
                        break;
                    }
                }
                if (!isDirectory && fontOk) {
                    DocumentFile fontFile = Helper.getFileFromDocumentID(this, fontsFolder, fileDocumentID);
                    if (fontFile != null) {
                        font1List.add(new FontItemList(this, name, fontFile.getUri(), true));
                        font2List.add(new FontItemList(this, name, fontFile.getUri(), false));
                    }
                }
            }
        }
    }

    public static boolean isSelectedFontFile(Context context, Uri fontUri, boolean firstFont) {
        Uri selectedFontUri = null;
        if (firstFont) {
            selectedFontUri = SettingsManager.getFont1FileURI();
        } else {
            selectedFontUri = SettingsManager.getFont2FileURI();
        }
        if (fontUri == null && selectedFontUri == null) {
            return true;
        }
        else if (fontUri != null) {
            return fontUri.equals(selectedFontUri);
        } else {
            return false;
        }
    }

    class FontItemList {
        private final String name;
        private final Uri uri;
        private final RadioButton radioButton;
        private final boolean firstFont;

        public FontItemList(Context context, String name, Uri uri, boolean firstFont) {
            this.name = name;
            this.uri = uri;
            this.firstFont = firstFont;

            // The Radio Button
            View layout = getLayoutInflater().inflate(R.layout.settings_soundfont_item_list, null);
            this.radioButton = layout.findViewById(R.id.settings_soundfont_radio_button);
            radioButton.setOnClickListener(v -> select());
            if (isSelectedFontFile(context, uri, firstFont)) {
                setSelected(true);
            }

            // The name
            radioButton.setText(name);
            radioButton.setOnClickListener(v -> select());
        }

        public void select() {
            if (firstFont) {
                SettingsManager.setFont1FileURI(uri);
                for (FontItemList s : font1List) {
                    s.getRadioButton().setChecked(false);
                }
                radioButton.setChecked(true);
                updatePreview(true);
            } else {
                SettingsManager.setFont2FileURI(uri);
                for (FontItemList s : font2List) {
                    s.getRadioButton().setChecked(false);
                }
                radioButton.setChecked(true);
                updatePreview(false);
            }
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

    private void configureFont1Size() {
        SeekBar fontSize1SeekBar = findViewById(R.id.settings_font1_size);
        fontSize1SeekBar.setProgress(SettingsManager.getFont1Size() - SIZE_MIN);

        TextView t = findViewById(R.id.settings_font1_size_text_view);

        fontSize1SeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                SettingsManager.setFont1Size(seekBar.getProgress() + SIZE_MIN);
                updatePreview(true);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                t.setText(String.valueOf(fontSize1SeekBar.getProgress() + SIZE_MIN));
            }
        });

        t.setText(String.valueOf(fontSize1SeekBar.getProgress() + SIZE_MIN));
        updatePreview(true);
    }

    private void configureFont2Size() {
        SeekBar fontSize2SeekBar = findViewById(R.id.settings_font2_size);
        fontSize2SeekBar.setProgress(SettingsManager.getFont2Size() - SIZE_MIN);

        TextView t = findViewById(R.id.settings_font2_size_text_view);

        fontSize2SeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                SettingsManager.setFont2Size(seekBar.getProgress() + SIZE_MIN);
                updatePreview(false);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                t.setText(String.valueOf(fontSize2SeekBar.getProgress() + SIZE_MIN));
            }
        });

        t.setText(String.valueOf(fontSize2SeekBar.getProgress() + SIZE_MIN));
        updatePreview(false);
    }

    private static native byte[] DrawText(String font, int size, boolean firstFont);
}

