package org.easyrpg.player.game_browser;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Environment;
import android.util.Log;

import org.easyrpg.player.R;
import org.easyrpg.player.settings.SettingsManager;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class GameScanner {
    // We use a singleton pattern to allow further optimizations
    // (Such as caching games' thumbnail, avoiding some syscall ...)
    private static volatile GameScanner instance = null;

    //Files' names
    private final static String DATABASE_NAME = "RPG_RT.ldb",
            TREEMAP_NAME = "RPG_RT.lmt",
            INI_FILE = "RPG_RT.ini";

    private List<GameInformation> gameList;
    private List<String> errorList;
    private Activity context;

    private GameScanner(Activity activity) {
        this.gameList = new ArrayList<GameInformation>();
        this.errorList = new ArrayList<String>();
        this.context = activity;
    }

    public static GameScanner getInstance(Activity activity) {
        // Singleton pattern
        if (GameScanner.instance == null) {
            synchronized(GameScanner.class) {
                if (GameScanner.instance == null) {
                    GameScanner.instance = new GameScanner(activity);
                }
            }
        }
        GameScanner.instance.context = activity;

        // Verify and Ask for permissions
        GameBrowserHelper.askForStoragePermission(activity);

        //Scan the folder
        instance.scanGames();

        return instance;
    }

    private void scanGames(){
        gameList.clear();
        errorList.clear();

        String state = Environment.getExternalStorageState();
        if (!Environment.MEDIA_MOUNTED.equals(state)) {
            errorList.add(context.getString(R.string.no_external_storage));
            return;
        }

        // Scanning all the games folders
        boolean first_directory = true;
        for(String path : SettingsManager.getGamesFolderList()){
            File dir = new File(path);
            // Verification
            // 1) The folder must exist
            if (!dir.exists() && !dir.mkdirs()) {
                String msg = context.getString(R.string.creating_dir_failed).replace("$PATH", path);
                Log.e("refreshScanGames( )", msg);
                errorList.add(msg);

                continue;
            }

            // 2) The folder must be readable
            if (!dir.canRead() || !dir.isDirectory()) {
                String msg = context.getString(R.string.path_not_readable).replace("$PATH", path);
                Log.e("refreshScanGames( )", msg);
                errorList.add(msg);

                continue;
            }

            // Scan the folder
            File[] list = dir.listFiles();
            // Go 2 directores deep to find games in /easyrpg/games, otherwise only 1
            scanFolder(list, first_directory ? 2 : 1);
            first_directory = false;
        }

        // If the scan bring nothing in this folder : we notifiate the errorList
        if (gameList.size() == 0) {
            String error = context.getString(R.string.no_games_found_and_explanation);
            errorList.add(error);
        }

        Log.i("Browser", gameList.size() + " games found : " + gameList);
    }

    /**
     * Tests if a folder is a RPG2k Game.
     * (contains DATABASE_NAME and TREEMAP_NAME)
     * @param dir Directory to test
     * @return true if RPG2k game
     */
    private static boolean isRpg2kGame(File dir) {
        if (!dir.isDirectory() || !dir.canRead()) {
            return false;
        }

        boolean databaseFound = false;
        boolean treemapFound = false;

        for (File entry : dir.listFiles()) {
            if (entry.isFile() && entry.canRead()) {
                if (!databaseFound && entry.getName().equalsIgnoreCase(DATABASE_NAME)) {
                    databaseFound = true;
                } else if (!treemapFound && entry.getName().equalsIgnoreCase(TREEMAP_NAME)) {
                    treemapFound = true;
                }

                if (databaseFound && treemapFound) {
                    return true;
                }
            }
        }

        return false;
    }

    private void scanFolder(File[] list, int depth) {
        if (list != null) {
            for (File file : list) {
                if (!file.getName().startsWith(".")) {
                    if (isRpg2kGame(file)) {
                        gameList.add(new GameInformation(file.getName(), file.getAbsolutePath()));
                    } else if (file.isDirectory() && file.canRead() && depth > 0) {
                        // Not a RPG2k Game but a directory -> recurse
                        scanFolder(file.listFiles(), depth - 1);
                    }
                }
            }
        }
    }

    public List<GameInformation> getGameList() {
        return gameList;
    }

    public List<String> getErrorList() {
        return errorList;
    }

    public boolean hasError() {
        return !errorList.isEmpty();
    }

    /** Return the game title screen, in a dumb way following last Enterbrain conventions to avoid
     *  numerous syscalls.
     */
    public static Bitmap getGameTitleScreen(GameInformation gameInformation) {
        // Search the title folder : the dumb way
        File dir = new File(gameInformation.getGameFolderPath() + "/Title");
        if(dir.isDirectory() && dir.canRead()) {
            File[] files = dir.listFiles();
            for (File f : files) {
                String name = f.getName().toLowerCase().trim();
                if (!f.getName().startsWith(".") && (name.endsWith("png") || name.endsWith("bmp") || name.endsWith("xyz"))) {
                    Bitmap b = BitmapFactory.decodeFile(f.getAbsolutePath());
                    if (b == null && GameBrowserActivity.libraryLoaded) {
                        // Check for XYZ
                        byte[] xyz = decodeXYZ(f.getAbsolutePath());
                        if (xyz == null) {
                            return null;
                        }
                        return BitmapFactory.decodeByteArray(xyz, 0, xyz.length);
                    }
                    return b;
                }
            }
        }
        return null;
    }
    
    private static native byte[] decodeXYZ(String path);
}
