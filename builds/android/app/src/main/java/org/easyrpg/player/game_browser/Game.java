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
	private String title;
    private final String gameFolderPath;
    private String savePath;
	private boolean isFavorite;
    private Bitmap titleScreen;
    private boolean standalone = false;

	private Game(String gameFolderPath) {
        int encoded_slash_pos = gameFolderPath.lastIndexOf("%2F");
        if (encoded_slash_pos == -1) {
            // Should not happen because the game is in a subdirectory
            Log.e("EasyRPG", "Strange Uri " + gameFolderPath);
        }
        int slash_pos = gameFolderPath.indexOf("/", encoded_slash_pos);

        // A file is provided when a / is after the encoded / (%2F)
        if (slash_pos > -1) {
            // Extract the filename and properly encode it
            this.title = gameFolderPath.substring(slash_pos + 1);
        } else {
            this.title = gameFolderPath.substring(encoded_slash_pos + 3);
        }

        int dot_pos = this.title.indexOf(".");
        if (dot_pos > -1) {
            this.title = this.title.substring(0, dot_pos);
        }

        this.gameFolderPath = gameFolderPath;
        this.savePath = gameFolderPath;

        this.isFavorite = isFavoriteFromSettings();
	}

	public Game(String gameFolderPath, byte[] titleScreen) {
	    this(gameFolderPath);
        this.titleScreen = BitmapFactory.decodeByteArray(titleScreen, 0, titleScreen.length);;
    }

    /**
     * Constructor for standalone mode
     *
     * @param gameFolder
     * @param saveFolder
     */
    public Game(String gameFolder, String saveFolder) {
        this.title = "Standalone";
        this.gameFolderPath = gameFolder;
        this.savePath = saveFolder;
        this.isFavorite = false;
        this.standalone = true;
    }

    private Game(String gameFolderPath, String saveFolder, byte[] titleScreen) {
        this(gameFolderPath, titleScreen);
        // is only relative here, launchGame will put this in the "saves" directory
        if (!saveFolder.isEmpty()) {
            savePath = saveFolder;
        }
    }

    public static Game fromCacheEntry(Context context, String cache) {
        String[] entries = cache.split(String.valueOf(escapeCode));

        if (entries.length != 3) {
            return null;
        }

        String savePath = entries[0];
        DocumentFile gameFolder = DocumentFile.fromTreeUri(context, Uri.parse(entries[1]));
        if (gameFolder == null) {
            return null;
        }

        byte[] titleScreen = null;
        if (!entries[2].equals("null")) {
            titleScreen = Base64.decode(entries[2], 0);
        }

        return new Game(entries[1], savePath, titleScreen);
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
		return SettingsManager.getFavoriteGamesList().contains(this.title);
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

        sb.append(savePath);
        sb.append(escapeCode);

        sb.append(gameFolderPath);
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

    public Boolean isStandalone() {
        return standalone;
    }
}
