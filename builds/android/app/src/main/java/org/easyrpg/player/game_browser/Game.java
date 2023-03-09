package org.easyrpg.player.game_browser;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.util.Base64;

import androidx.annotation.NonNull;
import androidx.documentfile.provider.DocumentFile;

import org.easyrpg.player.settings.SettingsManager;

import java.io.ByteArrayOutputStream;

public class Game implements Comparable<Game> {
    final static char escapeCode = '\u0001';
	private final String title;
    private final String gameFolderPath;
    private String savePath;
	private boolean isFavorite;
    private final DocumentFile gameFolder;
    private Bitmap titleScreen;
    /** Path to the game folder inside of the zip */
    private String zipInnerPath;

	private Game(DocumentFile gameFolder) {
		this.gameFolder = gameFolder;
	    this.title = gameFolder.getName();
        Uri folderURI = gameFolder.getUri();
	    this.gameFolderPath = folderURI.toString();
        this.savePath = gameFolderPath;

        this.isFavorite = isFavoriteFromSettings();
	}

	public Game(DocumentFile gameFolder, Bitmap titleScreen) {
	    this(gameFolder);
        this.titleScreen = titleScreen;
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
        this.gameFolder = null;
        this.isFavorite = false;
    }

    private Game(DocumentFile gameFolder, String pathInZip, Bitmap titleScreen) {
        this(gameFolder, titleScreen);
        zipInnerPath = pathInZip;
    }

    public static Game fromZip(DocumentFile gameFolder, String pathInZip, String saveFolder, Bitmap titleScreen) {
        Game game = new Game(gameFolder, pathInZip, titleScreen);
        // is only relative here, launchGame will put this in the "saves" directory
        game.setSavePath(saveFolder);
        return game;
    }

    public static Game fromCacheEntry(Context context, String cache) {
        String[] entries = cache.split(String.valueOf(escapeCode));

        if (entries.length != 5) {
            return null;
        }

        String savePath = entries[0];
        DocumentFile gameFolder = DocumentFile.fromTreeUri(context, Uri.parse(entries[1]));
        if (gameFolder == null) {
            return null;
        }

        boolean isZip = Boolean.parseBoolean(entries[2]);
        String zipInnerPath = null;

        if (isZip) {
            zipInnerPath = entries[3];
        }

        Bitmap titleScreen = null;
        if (!entries[4].equals("null")) {
            byte[] decodedByte = Base64.decode(entries[4], 0);
            titleScreen = BitmapFactory.decodeByteArray(decodedByte, 0, decodedByte.length);
        }

        if (isZip) {
            return fromZip(gameFolder, zipInnerPath, savePath, titleScreen);
        } else {
            return new Game(gameFolder, titleScreen);
        }
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

        sb.append(gameFolder.getUri());
        sb.append(escapeCode);

        sb.append(isZipArchive());
        sb.append(escapeCode);

        sb.append(zipInnerPath);
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

    public DocumentFile getGameFolder() {
        return gameFolder;
    }

    public Bitmap getTitleScreen() {
        return titleScreen;
    }

    public Boolean isStandalone() {
        return gameFolder == null;
    }

    public Boolean isZipArchive() {
        return zipInnerPath != null;
    }

    public String getZipInnerPath() {
        return zipInnerPath;
    }
}
