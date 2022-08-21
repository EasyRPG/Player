package org.easyrpg.player.settings;

import static org.easyrpg.player.settings.SettingsEnum.EASYRPG_FOLDER_URI;
import static org.easyrpg.player.settings.SettingsEnum.ENABLE_RTP_SCANNING;
import static org.easyrpg.player.settings.SettingsEnum.FAST_FORWARD_MODE;
import static org.easyrpg.player.settings.SettingsEnum.FAST_FORWARD_MULTIPLIER;
import static org.easyrpg.player.settings.SettingsEnum.FAVORITE_GAMES;
import static org.easyrpg.player.settings.SettingsEnum.FORCED_LANDSCAPE;
import static org.easyrpg.player.settings.SettingsEnum.IGNORE_LAYOUT_SIZE_SETTINGS;
import static org.easyrpg.player.settings.SettingsEnum.IMAGE_SIZE;
import static org.easyrpg.player.settings.SettingsEnum.LAYOUT_SIZE;
import static org.easyrpg.player.settings.SettingsEnum.LAYOUT_TRANSPARENCY;
import static org.easyrpg.player.settings.SettingsEnum.VIBRATE_WHEN_SLIDING_DIRECTION;
import static org.easyrpg.player.settings.SettingsEnum.VIBRATION_ENABLED;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.net.Uri;
import android.preference.PreferenceManager;

import androidx.documentfile.provider.DocumentFile;

import org.easyrpg.player.Helper;
import org.easyrpg.player.button_mapping.InputLayout;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

public class SettingsManager {
    private final static long VIBRATION_DURATION = 20; // ms

    private static SharedPreferences pref;
    private static SharedPreferences.Editor editor;

    private static boolean vibrationEnabled, vibrateWhenSlidingDirectionEnabled;
    private static boolean ignoreLayoutSizePreferencesEnabled;
    private static boolean forcedLandscape;
    private static boolean rtpScanningEnabled;
    private static int imageSize, layoutTransparency, layoutSize, fastForwardMode, fastForwardMultiplier;
    private static InputLayout inputLayoutHorizontal, inputLayoutVertical;
    // Note: don't store DocumentFile as they can be nullify with a change of context
    private static Uri easyRPGFolderURI, soundFountFileURI;
    private static List<String> favoriteGamesList = new ArrayList<>();
    public static String RTP_FOLDER_NAME = "rtp", RTP_2000_FOLDER_NAME = "2000",
        RTP_2003_FOLDER_NAME = "2003", SOUND_FONTS_FOLDER_NAME = "soundfonts",
        GAMES_FOLDER_NAME = "games", SAVES_FOLDER_NAME = "saves";
    public static int IMAGE_SIZE_UNIFORM_PIXEL_SIZE = 0, IMAGE_SIZE_STRETCH_IMAGE = 1;
    public static int FAST_FORWARD_MODE_HOLD = 0, FAST_FORWARD_MODE_TAP = 1;

    private SettingsManager() {
    }

    public static void init(Context context) {
        SettingsManager.pref = PreferenceManager.getDefaultSharedPreferences(context);
        SettingsManager.editor = pref.edit();

        loadSettings(context);
    }

    // TODO : Totally remove loadSettings & init? (in case of the crash of an application, some field can be set to null?)
    private static void loadSettings(Context context) {
        SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(context);

        imageSize = sharedPref.getInt(IMAGE_SIZE.toString(), IMAGE_SIZE_UNIFORM_PIXEL_SIZE);
        rtpScanningEnabled = sharedPref.getBoolean(ENABLE_RTP_SCANNING.toString(), true);
        vibrationEnabled = sharedPref.getBoolean(VIBRATION_ENABLED.toString(), true);
        layoutTransparency = sharedPref.getInt(LAYOUT_TRANSPARENCY.toString(), 100);
        vibrateWhenSlidingDirectionEnabled = sharedPref.getBoolean(VIBRATE_WHEN_SLIDING_DIRECTION.toString(), true);
        ignoreLayoutSizePreferencesEnabled = sharedPref.getBoolean(IGNORE_LAYOUT_SIZE_SETTINGS.toString(), false);
        layoutSize = sharedPref.getInt(LAYOUT_SIZE.toString(), 100);
        forcedLandscape = sharedPref.getBoolean(FORCED_LANDSCAPE.toString(), false);
        fastForwardMode = sharedPref.getInt(FAST_FORWARD_MODE.toString(), FAST_FORWARD_MODE_TAP);
        fastForwardMultiplier = sharedPref.getInt(FAST_FORWARD_MULTIPLIER.toString(), 3);

        // Fetch the favorite game list :
        favoriteGamesList = new ArrayList<>();
        String favoriteGamesListString = sharedPref.getString(FAVORITE_GAMES.toString(), "");
        if (!favoriteGamesListString.isEmpty()) {
            for (String folder : favoriteGamesListString.split("\\*")) {
                if (!favoriteGamesList.contains(folder)) {
                    favoriteGamesList.add(folder);
                }
                // TODO : Remove folder that doesn't exist
            }
        }
    }

    public static List<String> getFavoriteGamesList() {
        return favoriteGamesList;
    }

    public static void addFavoriteGame(String gameTitle) {
        gameTitle = gameTitle.trim();

        // The game folder must not be already in the list
        if (favoriteGamesList.contains(gameTitle)) {
            return;
        }

        // Update user's preferences
        favoriteGamesList.add(gameTitle);

        setFavoriteGamesList(favoriteGamesList);
    }

    public static void removeAFavoriteGame(String path) {
        favoriteGamesList.remove(path);
        setFavoriteGamesList(favoriteGamesList);
    }

    private static void setFavoriteGamesList(List<String> folderList) {
        favoriteGamesList = folderList;

        StringBuilder sb = new StringBuilder();
        for (String folder : favoriteGamesList) {
            sb.append(folder).append('*');
        }
        editor.putString(FAVORITE_GAMES.toString(), sb.toString());
        editor.commit();
    }

    public static int getImageSize() {
        return imageSize;
    }

    public static void setImageSize(int imageSize) {
        SettingsManager.imageSize = imageSize;
        editor.putInt(IMAGE_SIZE.toString(), imageSize);
        editor.commit();
    }

    public static long getVibrationDuration() {
        return VIBRATION_DURATION;
    }

    public static boolean isVibrationEnabled() {
        return vibrationEnabled;
    }

    public static void setVibrationEnabled(boolean b) {
        vibrationEnabled = b;
        editor.putBoolean(SettingsEnum.VIBRATION_ENABLED.toString(), b);
        editor.commit();
    }

    public static boolean isRTPScanningEnabled() {
        return rtpScanningEnabled;
    }

    public static void setRTPScanningEnabled(boolean rtpScanningEnabled) {
        SettingsManager.rtpScanningEnabled = rtpScanningEnabled;
        editor.putBoolean(ENABLE_RTP_SCANNING.toString(), rtpScanningEnabled);
        editor.commit();
    }

    public static int getFastForwardMode() {
        return fastForwardMode;
    }

    public static void setFastForwardMode(int i) {
        fastForwardMode = i;
        editor.putInt(SettingsEnum.FAST_FORWARD_MODE.toString(), i);
        editor.commit();
    }

    public static int getFastForwardMultiplier() {
        return fastForwardMultiplier;
    }

    public static void setFastForwardMultiplier(int i) {
        fastForwardMultiplier = i;
        editor.putInt(SettingsEnum.FAST_FORWARD_MULTIPLIER.toString(), i);
        editor.commit();
    }

    public static boolean isVibrateWhenSlidingDirectionEnabled() {
        return vibrateWhenSlidingDirectionEnabled;
    }

    public static void setVibrateWhenSlidingDirectionEnabled(boolean b) {
        vibrateWhenSlidingDirectionEnabled = b;
        editor.putBoolean(SettingsEnum.VIBRATE_WHEN_SLIDING_DIRECTION.toString(), b);
        editor.commit();
    }

    public static boolean isIgnoreLayoutSizePreferencesEnabled() {
        return ignoreLayoutSizePreferencesEnabled;
    }

    public static void setIgnoreLayoutSizePreferencesEnabled(boolean b) {
        ignoreLayoutSizePreferencesEnabled = b;
        editor.putBoolean(SettingsEnum.IGNORE_LAYOUT_SIZE_SETTINGS.toString(), b);
        editor.commit();

    }

    public static int getLayoutTransparency() {
        return layoutTransparency;
    }

    public static void setLayoutTransparency(int t) {
        layoutTransparency = t;
        editor.putInt(SettingsEnum.LAYOUT_TRANSPARENCY.toString(), t);
        editor.commit();
    }

    public static int getLayoutSize() {
        return layoutSize;
    }

    public static void setLayoutSize(int i) {
        layoutSize = i;
        editor.putInt(SettingsEnum.LAYOUT_SIZE.toString(), i);
        editor.commit();
    }

    public static boolean isForcedLandscape() {
        return forcedLandscape;
    }

    public static void setForcedLandscape(boolean b) {
        forcedLandscape = b;
        editor.putBoolean(SettingsEnum.FORCED_LANDSCAPE.toString(), b);
        editor.commit();
    }

    public static Uri getEasyRPGFolderURI(Context context) {
        if (easyRPGFolderURI == null) {
            SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(context);
            String gamesFolderString = sharedPref.getString(EASYRPG_FOLDER_URI.toString(), "");
            if (gamesFolderString == null || gamesFolderString.isEmpty()) {
                easyRPGFolderURI = null;
            } else {
                easyRPGFolderURI = Uri.parse(gamesFolderString);
            }
        }
        return easyRPGFolderURI;
    }

    public static void setEasyRPGFolderURI(Uri easyRPGFolderURI) {
        SettingsManager.easyRPGFolderURI = easyRPGFolderURI;
        editor.putString(SettingsEnum.EASYRPG_FOLDER_URI.toString(), easyRPGFolderURI.toString());
        editor.commit();
    }

    public static Uri getGamesFolderURI(Context context) {
        DocumentFile easyRPGFolder = Helper.getFileFromURI(context, easyRPGFolderURI);
        if (easyRPGFolder != null) {
            return Helper.findFileUri(context, easyRPGFolder.getUri(), GAMES_FOLDER_NAME);
        } else {
            return null;
        }
    }

    public static Uri getRTPFolderURI(Context context) {
        DocumentFile easyRPGFolder = Helper.getFileFromURI(context, easyRPGFolderURI);
        if (easyRPGFolder != null) {
            return Helper.findFileUri(context, easyRPGFolder.getUri(), RTP_FOLDER_NAME);
        } else {
            return null;
        }
    }

    public static Uri getSoundFontsFolderURI(Context context) {
        DocumentFile easyRPGFolder = Helper.getFileFromURI(context, easyRPGFolderURI);
        if (easyRPGFolder != null) {
            return Helper.findFileUri(context, easyRPGFolder.getUri(), SOUND_FONTS_FOLDER_NAME);
        } else {
            return null;
        }
    }

    public static Uri getSoundFountFileURI(Context context) {
        if (soundFountFileURI == null) {
            SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(context);
            String soundfontURI = sharedPref.getString(SettingsEnum.SOUNDFONT_URI.toString(), "");
            if (soundfontURI == null || soundfontURI.isEmpty()) {
                soundFountFileURI = null;
            } else {
                soundFountFileURI = Uri.parse(soundfontURI);
            }
        }
        return soundFountFileURI;
    }

    public static void setSoundFountFileURI(Uri soundFountFileURI) {
        String st = "";
        SettingsManager.soundFountFileURI = soundFountFileURI;
        if (soundFountFileURI != null) {
            st = soundFountFileURI.toString();
        }
        editor.putString(SettingsEnum.SOUNDFONT_URI.toString(), st);
        editor.commit();
    }

    public static InputLayout getInputLayoutHorizontal(Activity activity) {
        SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(activity);
        String inputLayoutString = sharedPref.getString(SettingsEnum.INPUT_LAYOUT_HORIZONTAL.toString(), null);
        if (inputLayoutString == null || inputLayoutString.isEmpty()) {
            SettingsManager.inputLayoutHorizontal = InputLayout.getDefaultInputLayoutHorizontal(activity);
        } else {
            SettingsManager.inputLayoutHorizontal = InputLayout.parse(activity, InputLayout.Orientation.ORIENTATION_HORIZONTAL, inputLayoutString);
        }
        return SettingsManager.inputLayoutHorizontal;
    }

    public static InputLayout getInputLayoutVertical(Activity activity) {
        SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(activity);
        String inputLayoutString = sharedPref.getString(SettingsEnum.INPUT_LAYOUT_VERTICAL.toString(), null);
        if (inputLayoutString == null || inputLayoutString.isEmpty()) {
            SettingsManager.inputLayoutVertical = InputLayout.getDefaultInputLayoutVertical(activity);
        } else {
            SettingsManager.inputLayoutVertical = InputLayout.parse(activity, InputLayout.Orientation.ORIENTATION_VERTICAL, inputLayoutString);
        }
        return SettingsManager.inputLayoutVertical;
    }

    public static void setInputLayoutHorizontal(Activity activity, InputLayout inputLayoutHorizontal) {
        SettingsManager.inputLayoutHorizontal = inputLayoutHorizontal;
        editor.putString(SettingsEnum.INPUT_LAYOUT_HORIZONTAL.toString(), inputLayoutHorizontal.toStringForSave(activity));
        editor.commit();
    }

    public static void setInputLayoutVertical(Activity activity, InputLayout inputLayoutVertical) {
        SettingsManager.inputLayoutVertical = inputLayoutVertical;
        editor.putString(SettingsEnum.INPUT_LAYOUT_VERTICAL.toString(), inputLayoutVertical.toStringForSave(activity));
        editor.commit();
    }
}

