package org.easyrpg.player.game_browser;

import android.content.Context;
import android.graphics.Bitmap;
import android.net.Uri;

import androidx.annotation.NonNull;
import androidx.documentfile.provider.DocumentFile;

import org.easyrpg.player.settings.SettingsManager;

public class Game implements Comparable<Game> {
	private final String title;
    private final String gameFolderPath;
    private String savePath;
	private boolean isFavorite;
    private final DocumentFile gameFolder;
    private Uri iniFile;
    private Bitmap titleScreen;
    private IniFileManager iniFileManager; // Always use initIniFileManager() before using iniFileManager
    /** Path to the game folder inside of the zip */
    private String zipInnerPath;

	private Game(DocumentFile gameFolder) {
		this.gameFolder = gameFolder;
	    this.title = gameFolder.getName();
        Uri folderURI = gameFolder.getUri();
	    this.gameFolderPath = folderURI.toString();

		// SavePath
        this.savePath = gameFolderPath;
        // TODO : Adapt this code in order to have savefile with .zip games
        // TODO : Save file should be stored in the EasyRPGFolder (there is no more default folder)
        /*
        if (GameBrowserHelper.canWrite(f)) {
			this.savePath = gameFolderPath;
		} else {
			// Not writable, redirect to a different path
			// Try preventing collisions by using the names of the two parent directories
			String savename = f.getParentFile().getName() + "/" + f.getName();
			savePath = SettingsManager.getEasyRPGFolder() + "/Save/" + savename;
			new File(savePath).mkdirs();
		}
        */

		this.isFavorite = isFavoriteFromSettings();
	}

	public Game(DocumentFile gameFolder, Uri iniFileUri, Bitmap titleScreen) {
	    this(gameFolder);
        this.iniFile = iniFileUri;
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

    private Game(DocumentFile gameFolder, String pathInZip, Uri iniFileUri, Bitmap titleScreen) {
        this(gameFolder, iniFileUri, titleScreen);
        zipInnerPath = pathInZip;
    }

    public static Game fromZip(DocumentFile gameFolder, String pathInZip, String saveFolder, Uri iniFileUri, Bitmap titleScreen) {
        Game game = new Game(gameFolder, pathInZip, iniFileUri, titleScreen);
        // is only relative here, launchGame will put this in the "saves" directory
        game.setSavePath(saveFolder);
        return game;
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
			SettingsManager.addFavoriteGame(this.title);
		} else {
			SettingsManager.removeAFavoriteGame(this.title);
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

    /** We initiate the IniFileManager only if necessary to prevent from unnecessary syscall */
    public void initIniFileManager(Context context) {
        if (iniFileManager == null) {
            this.iniFileManager = new IniFileManager(context, iniFile);
        }
    }

	public IniFileManager.Encoding getEncoding(Context context) {
        // We initiate the IniFileManager only if necessary to prevent from unnecessary syscalls */
        initIniFileManager(context);

        return this.iniFileManager.getEncoding();
	}

	public void setEncoding(Context context, IniFileManager.Encoding encoding) {
        // We initiate the IniFileManager only if necessary to prevent from unnecessary syscalls */
        initIniFileManager(context);

        this.iniFileManager.setEncoding(context, encoding);
	}

    @NonNull
    @Override
    public String toString() {
        return getTitle();
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
