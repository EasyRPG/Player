package org.easyrpg.player.settings;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.widget.Toast;

import org.easyrpg.player.R;
import org.easyrpg.player.game_browser.GameBrowserHelper;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import static org.easyrpg.player.settings.SettingsEnum.AUDIO_ENABLED;
import static org.easyrpg.player.settings.SettingsEnum.FAST_FORWARD_MODE;
import static org.easyrpg.player.settings.SettingsEnum.FAST_FORWARD_MULTIPLIER;
import static org.easyrpg.player.settings.SettingsEnum.FORCED_LANDSCAPE;
import static org.easyrpg.player.settings.SettingsEnum.GAMES_DIRECTORY;
import static org.easyrpg.player.settings.SettingsEnum.IGNORE_LAYOUT_SIZE_SETTINGS;
import static org.easyrpg.player.settings.SettingsEnum.LAYOUT_SIZE;
import static org.easyrpg.player.settings.SettingsEnum.LAYOUT_TRANSPARENCY;
import static org.easyrpg.player.settings.SettingsEnum.MAIN_DIRECTORY;
import static org.easyrpg.player.settings.SettingsEnum.VIBRATE_WHEN_SLIDING_DIRECTION;
import static org.easyrpg.player.settings.SettingsEnum.VIBRATION_ENABLED;

public class SettingsManager {
    private final static long VIBRATION_DURATION = 20; // ms

    private static SharedPreferences pref;
    private static SharedPreferences.Editor editor;
    private static Context context;

    private static boolean vibrationEnabled;
    private static boolean vibrateWhenSlidingDirectionEnabled;
    private static boolean audioEnabled;
    private static boolean ignoreLayoutSizePreferencesEnabled;
    private static boolean forcedLandscape;
    private static int layoutTransparency, layoutSize, fastForwardMode, fastForwardMultiplier;
    private static String easyRPGFolder;
    private static List<String> gamesFolderList = new ArrayList<>();

    private SettingsManager() {
    }

    public static void init(Context context) {
        SettingsManager.context = context;
        SettingsManager.pref = PreferenceManager.getDefaultSharedPreferences(context);
        SettingsManager.editor = pref.edit();

        loadSettings();
    }

    private static void loadSettings() {
        SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(context);

        vibrationEnabled = sharedPref.getBoolean(VIBRATION_ENABLED.toString(), true);
        audioEnabled = sharedPref.getBoolean(AUDIO_ENABLED.toString(), true);
        layoutTransparency = sharedPref.getInt(LAYOUT_TRANSPARENCY.toString(), 100);
        vibrateWhenSlidingDirectionEnabled = sharedPref.getBoolean(VIBRATE_WHEN_SLIDING_DIRECTION.toString(), false);
        ignoreLayoutSizePreferencesEnabled = sharedPref.getBoolean(IGNORE_LAYOUT_SIZE_SETTINGS.toString(), false);
        layoutSize = sharedPref.getInt(LAYOUT_SIZE.toString(), 100);
        easyRPGFolder = sharedPref.getString(MAIN_DIRECTORY.toString(),
                Environment.getExternalStorageDirectory().getPath() + "/easyrpg");
        forcedLandscape = sharedPref.getBoolean(FORCED_LANDSCAPE.toString(), false);
        fastForwardMode = sharedPref.getInt(FAST_FORWARD_MODE.toString(), 0);
        fastForwardMultiplier = sharedPref.getInt(FAST_FORWARD_MULTIPLIER.toString(), 3);

        // Fetch the games directories :
        gamesFolderList = new ArrayList<>();
        // 1) The default directory (cannot be modified)
        gamesFolderList.add(easyRPGFolder + "/games");
        // 2) Others defined by users (separator : "*")
        String gameFolderListString = sharedPref.getString(GAMES_DIRECTORY.toString(), "");
        if (!gameFolderListString.isEmpty()) {
            for (String folder : gameFolderListString.split("\\*")) {
                if (!gamesFolderList.contains(folder)) {
                    gamesFolderList.add(folder);
                }
            }
        }
    }

    public static List<String> getGamesFolderList() {
        return gamesFolderList;
    }

    public static void addGameDirectory(String pathToAdd) {
        pathToAdd = pathToAdd.trim();

        // 1) The game folder must not be already in the list
        if (gamesFolderList.contains(pathToAdd)) {
            return;
        }

        // 2) Verify read permission
        File f = new File(pathToAdd);
        if (!f.canRead()) {
            Toast.makeText(context, context.getString(R.string.path_not_readable).replace("$PATH", pathToAdd), Toast.LENGTH_LONG).show();
            return;
        }

        // 3) When the user selects a directory containing a game, select automatically the folder above
        if (GameBrowserHelper.isRpg2kGame(f)) {
            pathToAdd = pathToAdd.substring(0, pathToAdd.lastIndexOf("/"));
        }

        // Update user's preferences
        gamesFolderList.add(pathToAdd);

        setGameFolderList(gamesFolderList);
    }

    public static void removeAGameFolder(String path) {
        gamesFolderList.remove(path);
        setGameFolderList(gamesFolderList);
    }

    private static void setGameFolderList(List<String> folderList) {
        gamesFolderList = folderList;

        StringBuilder sb = new StringBuilder();
        for (String folder : gamesFolderList) {
            sb.append(folder).append('*');
        }
        editor.putString(SettingsEnum.GAMES_DIRECTORY.toString(), sb.toString());
        editor.commit();
    }

    public static String getEasyRPGFolder() {
        return easyRPGFolder;
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
}

