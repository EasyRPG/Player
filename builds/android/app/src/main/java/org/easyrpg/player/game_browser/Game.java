package org.easyrpg.player.game_browser;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.util.Base64;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.documentfile.provider.DocumentFile;

import org.easyrpg.player.settings.SettingsManager;

import java.io.ByteArrayOutputStream;

public class Game implements Comparable<Game> {
    final static char escapeCode = '\u0001';
    /** The title shown in the Game Browser */
    private String title;
    /** Path to the game folder (forwarded via --project-path */
    private final String gameFolderPath;
    /** Relative path to the save directory, made absolute by launchGame (forwarded via --save-path) */
    private String savePath = "";
    /** Whether the game was tagged as a favourite */
    private boolean isFavorite;
    /** Title image shown in the Game Browser */
    private Bitmap titleScreen = null;
    /** Game is launched from the APK via standalone mode */
    private boolean standalone = false;

    public Game(String gameFolderPath, String saveFolder, String title, byte[] titleScreen) {
        this.gameFolderPath = gameFolderPath;

        // is only relative here, launchGame will put this in the "saves" directory
        if (!saveFolder.isEmpty()) {
            savePath = saveFolder;
        }

        this.title = title;

        if (titleScreen != null) {
            this.titleScreen = BitmapFactory.decodeByteArray(titleScreen, 0, titleScreen.length);
        };

        this.isFavorite = isFavoriteFromSettings();
    }

    public static Game fromCacheEntry(Context context, String cache) {
        String[] entries = cache.split(String.valueOf(escapeCode));

        if (entries.length != 4) {
            return null;
        }

        String savePath = entries[0];
        DocumentFile gameFolder = DocumentFile.fromTreeUri(context, Uri.parse(entries[1]));
        if (gameFolder == null) {
            return null;
        }

        String title = entries[2];

        byte[] titleScreen = null;
        if (!entries[3].equals("null")) {
            titleScreen = Base64.decode(entries[3], 0);
        }

        return new Game(entries[1], savePath, title, titleScreen);
    }

    public String getTitle() {
        return title;
    }

    public String getGameFolderPath() {
        return gameFolderPath;
    }

    public String getSavePath() {
        return savePath;
    }

    public void setSavePath(String path) {
        savePath = path;
    }

    public boolean isFavorite() {
        return isFavorite;
    }

    public void setFavorite(boolean isFavorite) {
        this.isFavorite = isFavorite;
        if(isFavorite){
            SettingsManager.addFavoriteGame(this);
        } else {
            SettingsManager.removeAFavoriteGame(this);
        }
    }

    private boolean isFavoriteFromSettings() {
        return SettingsManager.getFavoriteGamesList().contains(this.gameFolderPath);
    }

    @Override
    public int compareTo(Game game) {
        if (this.isFavorite() && !game.isFavorite()) {
            return -1;
        }
        if (!this.isFavorite() && game.isFavorite()) {
            return 1;
        }
        return this.title.compareTo(game.title);
    }

    public Encoding getEncoding() {
        return SettingsManager.getGameEncoding(this);
    }

    public void setEncoding(Encoding encoding) {
        SettingsManager.setGameEncoding(this, encoding);
    }

    @NonNull
    @Override
    public String toString() {
        return getTitle();
    }

    public String toCacheEntry() {
        StringBuilder sb = new StringBuilder();

        // Cache structure: savePath | gameFolderPath | title | titleScreen
        sb.append(savePath);
        sb.append(escapeCode);
        sb.append(gameFolderPath);
        sb.append(escapeCode);
        sb.append(title);
        sb.append(escapeCode);

        if (titleScreen != null) {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            titleScreen.compress(Bitmap.CompressFormat.PNG, 90, baos);
            byte[] b = baos.toByteArray();
            sb.append(Base64.encodeToString(b, Base64.NO_WRAP));
        } else {
            sb.append("null");
        }

        return sb.toString();
    }

    public Bitmap getTitleScreen() {
        return titleScreen;
    }

    public boolean isStandalone() {
        return standalone;
    }

    public void setStandalone(boolean standalone) {
        this.standalone = standalone;
    }
}
