package org.easyrpg.player.settings;

import static org.easyrpg.player.settings.SettingsEnum.*;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.net.Uri;
import android.preference.PreferenceManager;

import androidx.documentfile.provider.DocumentFile;

import org.easyrpg.player.Helper;
import org.easyrpg.player.button_mapping.InputLayout;
import org.easyrpg.player.game_browser.Encoding;
import org.easyrpg.player.game_browser.Game;

import java.io.File;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class SettingsManager {
    private final static long VIBRATION_DURATION = 20; // ms

    private static SharedPreferences pref;
    private static SharedPreferences.Editor editor;

    private static IniFile configIni;

    private static boolean vibrationEnabled, vibrateWhenSlidingDirectionEnabled;
    private static boolean ignoreLayoutSizePreferencesEnabled;
    private static boolean forcedLandscape;
    private static boolean stretch;
    private static boolean rtpScanningEnabled;
    private static int imageSize, gameResolution;
    private static int layoutTransparency, layoutSize, fastForwardMode, fastForwardMultiplier;
    private static int musicVolume, soundVolume;
    private static InputLayout inputLayoutHorizontal, inputLayoutVertical;
    // Note: don't store DocumentFile as they can be nullify with a change of context
    private static Uri easyRPGFolderURI, soundFountFileURI;
    private static Set<String> favoriteGamesList = new HashSet<>();
    private static int gamesCacheHash;
    private static Set<String> gamesCache = new HashSet<>();
    public static String RTP_FOLDER_NAME = "rtp", RTP_2000_FOLDER_NAME = "2000",
        RTP_2003_FOLDER_NAME = "2003", SOUND_FONTS_FOLDER_NAME = "soundfonts",
        GAMES_FOLDER_NAME = "games", SAVES_FOLDER_NAME = "saves";
    public static int FAST_FORWARD_MODE_HOLD = 0, FAST_FORWARD_MODE_TAP = 1;

    private static List<String> imageSizeOption = Arrays.asList("nearest", "integer", "bilinear");
    private static List<String> gameResolutionOption = Arrays.asList("original", "widescreen", "ultrawide");

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

        configIni = new IniFile(new File(context.getExternalFilesDir(null).getAbsoluteFile() + "/config.ini"));

        rtpScanningEnabled = sharedPref.getBoolean(ENABLE_RTP_SCANNING.toString(), true);
        vibrationEnabled = sharedPref.getBoolean(VIBRATION_ENABLED.toString(), true);
        layoutTransparency = sharedPref.getInt(LAYOUT_TRANSPARENCY.toString(), 100);
        vibrateWhenSlidingDirectionEnabled = sharedPref.getBoolean(VIBRATE_WHEN_SLIDING_DIRECTION.toString(), true);
        ignoreLayoutSizePreferencesEnabled = sharedPref.getBoolean(IGNORE_LAYOUT_SIZE_SETTINGS.toString(), false);
        layoutSize = sharedPref.getInt(LAYOUT_SIZE.toString(), 100);
        forcedLandscape = sharedPref.getBoolean(FORCED_LANDSCAPE.toString(), false);
        stretch = configIni.video.getBoolean(STRETCH.toString(), false);
        fastForwardMode = sharedPref.getInt(FAST_FORWARD_MODE.toString(), FAST_FORWARD_MODE_TAP);
        fastForwardMultiplier = sharedPref.getInt(FAST_FORWARD_MULTIPLIER.toString(), 3);

        musicVolume = configIni.audio.getInteger(MUSIC_VOLUME.toString(), 100);
        soundVolume = configIni.audio.getInteger(SOUND_VOLUME.toString(), 100);

        favoriteGamesList = new HashSet<>(sharedPref.getStringSet(FAVORITE_GAMES.toString(), new HashSet<>()));

        gamesCacheHash = sharedPref.getInt(CACHE_GAMES_HASH.toString(), 0);
        gamesCache = new HashSet<>(sharedPref.getStringSet(CACHE_GAMES.toString(), new HashSet<>()));

        imageSize = imageSizeOption.indexOf(configIni.video.getString(IMAGE_SIZE.toString(), ""));
        if (imageSize == -1) {
            imageSize = 2;
        }

        gameResolution = gameResolutionOption.indexOf(configIni.video.getString(GAME_RESOLUTION.toString(), ""));
        if (gameResolution == -1) {
            gameResolution = 0;
        }
    }

    public static Set<String> getFavoriteGamesList() {
        return favoriteGamesList;
    }

    public static void addFavoriteGame(Game game) {
        // Update user's preferences
        favoriteGamesList.add(game.getTitle());

        setFavoriteGamesList(favoriteGamesList);
    }

    public static void removeAFavoriteGame(Game game) {
        favoriteGamesList.remove(game.getTitle());
        setFavoriteGamesList(favoriteGamesList);
    }

    private static void setFavoriteGamesList(Set<String> folderList) {
        editor.putStringSet(FAVORITE_GAMES.toString(), folderList);
        editor.commit();
    }

    public static int getGamesCacheHash() {
        return gamesCacheHash;
    }

    public static Set<String> getGamesCache() {
        return gamesCache;
    }

    public static void clearGamesCache() {
        setGamesCache(0, new HashSet<>());
    }

    public static void setGamesCache(int hash, Set<String> cache) {
        if (hash == gamesCacheHash) {
            return;
        }

        gamesCache = cache;
        gamesCacheHash = hash;

        editor.putInt(CACHE_GAMES_HASH.toString(), hash);
        editor.putStringSet(CACHE_GAMES.toString(), cache);
        editor.commit();
    }

    public static int getImageSize() {
        return imageSize;
    }

    public static void setImageSize(int imageSize) {
        SettingsManager.imageSize = imageSize;
        configIni.video.set(IMAGE_SIZE.toString(), imageSizeOption.get(imageSize));
        configIni.save();
    }

    public static int getGameResolution() {
        return gameResolution;
    }

    public static void setGameResolution(int gameResolution) {
        SettingsManager.gameResolution = gameResolution;
        configIni.video.set(GAME_RESOLUTION.toString(), gameResolutionOption.get(gameResolution));
        configIni.save();
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

    public static int getMusicVolume() {
        return musicVolume;
    }

    public static void setMusicVolume(int i) {
        musicVolume = i;
        configIni.audio.set(MUSIC_VOLUME.toString(), i);
        configIni.save();
    }

    public static int getSoundVolume() {
        return soundVolume;
    }

    public static void setSoundVolume(int i) {
        soundVolume = i;
        configIni.audio.set(SOUND_VOLUME.toString(), i);
        configIni.save();
    }

    public static void setForcedLandscape(boolean b) {
        forcedLandscape = b;
        editor.putBoolean(SettingsEnum.FORCED_LANDSCAPE.toString(), b);
        editor.commit();
    }

    public static boolean isStretch() {
        return stretch;
    }

    public static void setStretch(boolean b) {
        stretch = b;
        configIni.video.set(STRETCH.toString(), b);
        configIni.save();
    }

    public static Uri getEasyRPGFolderURI(Context context) {
        if (easyRPGFolderURI == null) {
            SharedPreferences sharedPref = PreferenceManager.getDefaultSharedPreferences(context);
            String gamesFolderString = sharedPref.getString(EASYRPG_FOLDER_URI.toString(), "");
            if (gamesFolderString.isEmpty()) {
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
            if (soundfontURI.isEmpty()) {
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

    public static Encoding getGameEncoding(Game game) {
        return Encoding.regionCodeToEnum(pref.getString(game.getTitle() + "_Encoding", ""));
    }

    public static void setGameEncoding(Game game, Encoding encoding) {
        editor.putString(game.getTitle() + "_Encoding", encoding.getRegionCode());
        editor.commit();
    }
}
