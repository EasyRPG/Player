package org.easyrpg.player.settings;

import static org.easyrpg.player.settings.SettingsEnum.AUDIO_ENABLED;
import static org.easyrpg.player.settings.SettingsEnum.CUSTOM_SOUNDFONT_ENABLED;
import static org.easyrpg.player.settings.SettingsEnum.FAST_FORWARD_MODE;
import static org.easyrpg.player.settings.SettingsEnum.FAST_FORWARD_MULTIPLIER;
import static org.easyrpg.player.settings.SettingsEnum.FAVORITE_GAMES;
import static org.easyrpg.player.settings.SettingsEnum.FORCED_LANDSCAPE;
import static org.easyrpg.player.settings.SettingsEnum.GAMES_DIRECTORY;
import static org.easyrpg.player.settings.SettingsEnum.IGNORE_LAYOUT_SIZE_SETTINGS;
import static org.easyrpg.player.settings.SettingsEnum.LAYOUT_SIZE;
import static org.easyrpg.player.settings.SettingsEnum.LAYOUT_TRANSPARENCY;
import static org.easyrpg.player.settings.SettingsEnum.RTP_DIRECTORY;
import static org.easyrpg.player.settings.SettingsEnum.VIBRATE_WHEN_SLIDING_DIRECTION;
import static org.easyrpg.player.settings.SettingsEnum.VIBRATION_ENABLED;

import android.content.Context;
import android.content.SharedPreferences;
import android.net.Uri;
import android.preference.PreferenceManager;

import java.util.ArrayList;
import java.util.List;

public class SettingsManager {
    private final static long VIBRATION_DURATION = 20; // ms

    private static SharedPreferences pref;
    private static SharedPreferences.Editor editor;

    private static boolean vibrationEnabled, vibrateWhenSlidingDirectionEnabled;
    private static boolean audioEnabled, customSoundFountsEnabled;
    private static boolean ignoreLayoutSizePreferencesEnabled;
    private static boolean forcedLandscape;
    private static int layoutTransparency, layoutSize, fastForwardMode, fastForwardMultiplier;
    // Note: don't store DocumentFile as they can be nullify if there is a problem with the Context
    // TODO : Should we store String instead of URI? Maybe Uri can be nullify too
    private static Uri gamesFolderURI, rtpFolderURI, soundFountFileURI;
    private static List<String> favoriteGamesList = new ArrayList<>();

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

        vibrationEnabled = sharedPref.getBoolean(VIBRATION_ENABLED.toString(), true);
        audioEnabled = sharedPref.getBoolean(AUDIO_ENABLED.toString(), true);
        customSoundFountsEnabled = sharedPref.getBoolean(CUSTOM_SOUNDFONT_ENABLED.toString(), false);
        layoutTransparency = sharedPref.getInt(LAYOUT_TRANSPARENCY.toString(), 100);
        vibrateWhenSlidingDirectionEnabled = sharedPref.getBoolean(VIBRATE_WHEN_SLIDING_DIRECTION.toString(), false);
        ignoreLayoutSizePreferencesEnabled = sharedPref.getBoolean(IGNORE_LAYOUT_SIZE_SETTINGS.toString(), false);
        layoutSize = sharedPref.getInt(LAYOUT_SIZE.toString(), 100);
        forcedLandscape = sharedPref.getBoolean(FORCED_LANDSCAPE.toString(), false);
        fastForwardMode = sharedPref.getInt(FAST_FORWARD_MODE.toString(), 0);
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

    public static boolean isAudioEnabled() {
        return audioEnabled;
    }

    public static void setAudioEnabled(boolean b) {
        audioEnabled = b;
        editor.putBoolean(SettingsEnum.AUDIO_ENABLED.toString(), b);
        editor.commit();
    }

    public static boolean isCustomSoundFountsEnabled() {
        return customSoundFountsEnabled;
    }

    public static void setCustomSoundFountsEnabled(boolean b) {
        customSoundFountsEnabled = b;
        editor.putBoolean(CUSTOM_SOUNDFONT_ENABLED.toString(), b);
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

    public static Uri getGamesFolderURI(Context context) {
        if (gamesFolderURI == null) {
            SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(context);
            String gamesFolderString = sharedPref.getString(GAMES_DIRECTORY.toString(), "");
            if (gamesFolderString == null || gamesFolderString.isEmpty()) {
                gamesFolderURI = null;
            } else {
                gamesFolderURI = Uri.parse(gamesFolderString);
            }
        }
        return gamesFolderURI;
    }

    public static void setGamesFolderURI(Uri gamesFolderURI) {
        editor.putString(SettingsEnum.GAMES_DIRECTORY.toString(), gamesFolderURI.toString());
        editor.commit();
    }

    public static Uri getRTPFolderURI(Context context) {
        if (rtpFolderURI == null) {
            SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(context);
            // Fetch the rtp directory
            String rtpFolderString = sharedPref.getString(RTP_DIRECTORY.toString(), "");
            if (rtpFolderString == null || rtpFolderString.isEmpty()) {
                rtpFolderURI = null;
            } else {
                rtpFolderURI = Uri.parse(rtpFolderString);
            }
        }

        return rtpFolderURI;
    }

    public static void setRTPFolderURI(Uri rtpFolderUri) {
        editor.putString(SettingsEnum.RTP_DIRECTORY.toString(), rtpFolderUri.toString());
        editor.commit();
    }

    public static Uri getSoundFountFileURI() {
        return soundFountFileURI;
    }

    public static void setSoundFountFileURI(Uri soundFountFileURI) {
       SettingsManager.soundFountFileURI = soundFountFileURI;
    }
}

