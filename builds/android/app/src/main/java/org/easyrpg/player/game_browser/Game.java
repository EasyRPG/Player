package org.easyrpg.player.game_browser;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.util.Base64;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.documentfile.provider.DocumentFile;

import org.easyrpg.player.Helper;
import org.easyrpg.player.settings.SettingsManager;

import java.io.ByteArrayOutputStream;

public class Game implements Comparable<Game> {
    final static char escapeCode = '\u0001';
    /** The title shown in the Game Browser */
    private String title = "";
    /** Bytes of the title string in an unspecified encoding */
    private byte[] titleRaw = null;
    /** Human readable version of the game directory. Shown in the game browser
     *  when the specific setting is enabled.
     */
    private String gameFolderName = "";
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
    /** Associated project type. Used to differentiane between supported engines and known but unsupported engines */
    private ProjectType projectType = ProjectType.UNKNOWN;

    public Game(int projectTypeId) {
        this.projectType = ProjectType.getProjectType(projectTypeId);
        this.gameFolderPath = "";
    }

    public Game(String gameFolderPath, String saveFolder, byte[] titleScreen, int projectTypeId) {
        this.gameFolderPath = gameFolderPath;
        this.projectType = ProjectType.getProjectType(projectTypeId);

        // is only relative here, launchGame will put this in the "saves" directory
        if (!saveFolder.isEmpty()) {
            savePath = saveFolder;
        }

        if (titleScreen != null) {
            this.titleScreen = BitmapFactory.decodeByteArray(titleScreen, 0, titleScreen.length);
        };

        this.isFavorite = isFavoriteFromSettings();
    }

    public String getDisplayTitle() {
        String customTitle = getCustomTitle();
        if (!customTitle.isEmpty()) {
            return customTitle;
        }

        if (SettingsManager.getGameBrowserLabelMode() == 0 && !getTitle().isEmpty()) {
            return getTitle();
        } else {
            return gameFolderName;
        }
    }

    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public native void reencodeTitle();

    public String getCustomTitle() {
        return SettingsManager.getCustomGameTitle(this);
    }

    public void setCustomTitle(String customTitle) {
        SettingsManager.setCustomGameTitle(this, customTitle);
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

    public String getGameFolderName() {
        return gameFolderName;
    }

    public void setGameFolderName(String gameFolderName) {
        this.gameFolderName = gameFolderName;
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
        return SettingsManager.getFavoriteGamesList().contains(this.getKey());
    }

    @Override
    public int compareTo(Game game) {
        // Unsupported games last
        if (this.projectType == ProjectType.SUPPORTED && game.projectType.ordinal() > ProjectType.SUPPORTED.ordinal()) {
            return -1;
        }
        if (this.projectType.ordinal() > ProjectType.SUPPORTED.ordinal() && game.projectType == ProjectType.SUPPORTED) {
            return 1;
        }
        // Favorites first
        if (this.isFavorite() && !game.isFavorite()) {
            return -1;
        }
        if (!this.isFavorite() && game.isFavorite()) {
            return 1;
        }
        return this.getDisplayTitle().compareTo(game.getDisplayTitle());
    }

    /**
     * Returns a unique key to be used for storing settings related to the game.
     *
     * @return unique key
     */
    public String getKey() {
        return gameFolderPath.replaceAll("[/ ]", "_");
    }

    public Encoding getEncoding() {
        return SettingsManager.getGameEncoding(this);
    }

    public void setEncoding(Encoding encoding) {
        SettingsManager.setGameEncoding(this, encoding);
        reencodeTitle();
    }

    /**
     * @return The encoding number or "auto" when not configured (for use via JNI)
     */
    public String getEncodingCode() {
        return getEncoding().getRegionCode();
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

    @NonNull
    @Override
    public String toString() {
        return getDisplayTitle();
    }

    public Uri createSaveUri(Context context) {
        if (!getSavePath().isEmpty()) {
            DocumentFile saveFolder = Helper.createFolderInSave(context, getSavePath());

            if (saveFolder != null) {
                return saveFolder.getUri();
            }
        } else {
            return Uri.parse(getGameFolderPath());
        }

        return null;
    }

    public static Game fromCacheEntry(Context context, String cache) {
        String[] entries = cache.split(String.valueOf(escapeCode));

        if (entries.length != 7) {
            return null;
        }

        int parsedProjectType = Integer.parseInt(entries[6]);
        if (parsedProjectType > ProjectType.SUPPORTED.ordinal()) {
            // Unsupported game
            Game g = new Game(parsedProjectType);
            g.setGameFolderName(entries[2]);
            return g;
        }

        String savePath = entries[0];
        DocumentFile gameFolder = DocumentFile.fromTreeUri(context, Uri.parse(entries[1]));
        if (gameFolder == null) {
            return null;
        }

        String gameFolderName = entries[2];

        String title = entries[3];

        byte[] titleRaw = null;
        if (!entries[4].equals("null")) {
            titleRaw = Base64.decode(entries[4], 0);
        }

        byte[] titleScreen = null;
        if (!entries[5].equals("null")) {
            titleScreen = Base64.decode(entries[5], 0);
        }

        Game g = new Game(entries[1], savePath, titleScreen, parsedProjectType);
        g.setTitle(title);
        g.titleRaw = titleRaw;

        if (g.titleRaw != null) {
            g.reencodeTitle();
        }

        g.setGameFolderName(gameFolderName);

        return g;
    }

    public String toCacheEntry() {
        StringBuilder sb = new StringBuilder();

        // Cache structure: savePath | gameFolderPath | gameFolderName | title | titleRaw | titleScreen | projectType
        sb.append(savePath); // 0
        sb.append(escapeCode);
        sb.append(gameFolderPath); // 1
        sb.append(escapeCode);
        sb.append(gameFolderName); // 2
        sb.append(escapeCode);
        sb.append(title); // 3
        sb.append(escapeCode);
        if (titleRaw != null) { // 4
            sb.append(Base64.encodeToString(titleRaw, Base64.NO_WRAP));
        } else {
            sb.append("null");
        }
        sb.append(escapeCode);
        if (titleScreen != null) { // 5
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            titleScreen.compress(Bitmap.CompressFormat.PNG, 90, baos);
            byte[] b = baos.toByteArray();
            sb.append(Base64.encodeToString(b, Base64.NO_WRAP));
        } else {
            sb.append("null");
        }
        sb.append(escapeCode);
        sb.append(projectType.ordinal()); // 6

        return sb.toString();
    }

    public boolean isProjectTypeUnsupported() {
        return this.projectType.ordinal() > ProjectType.SUPPORTED.ordinal();
    }

    public String getProjectTypeLabel() {
        return this.projectType.getLabel();
    }
}
