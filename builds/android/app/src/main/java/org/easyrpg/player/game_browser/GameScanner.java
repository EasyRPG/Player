package org.easyrpg.player.game_browser;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Environment;
import android.os.ParcelFileDescriptor;
import android.provider.MediaStore;
import android.util.Log;

import androidx.documentfile.provider.DocumentFile;

import org.easyrpg.player.R;
import org.easyrpg.player.settings.SettingsManager;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class GameScanner {
    // We use a singleton pattern to allow further optimizations
    // (Such as caching games' thumbnail, avoiding some syscall ...)
    private static volatile GameScanner instance = null;

    private List<Game> gameList;
    private List<String> errorList; // The list of errors that will be displayed in case of problems during the scan
    private Activity context;

    private static int GAME_SCANNING_DEPTH = 1; // 1 = no recursive scanning

    private GameScanner(Activity activity) {
        this.gameList = new ArrayList<Game>();
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
        // TODO : Do we need that ?
        // GameBrowserHelper.askForStoragePermission(activity);

        //Scan the folder
        instance.scanGames();

        return instance;
    }

    private void scanGames(){
        gameList.clear();
        errorList.clear();

        // Check that the storage is available
        // TODO : Is it still necessary?
        /*
        String state = Environment.getExternalStorageState();
        if (!Environment.MEDIA_MOUNTED.equals(state)) {
            errorList.add(context.getString(R.string.no_external_storage));
            return;
        }
        */

        // Retrieve the games folder
        DocumentFile gamesFolder = SettingsManager.getGameFolder();
        Log.e("EasyRPG", "Game folder : " + gamesFolder.getName());

        // 1) The folder must exist
        if (gamesFolder == null || !gamesFolder.isDirectory()) {
            // TODO Replace the text by a R.string
            //String msg = context.getString(R.string.creating_dir_failed).replace("$PATH", dir.getName());
            String msg = "The games folder doesn't exist or isn't a folder";
            Log.e("EasyRPG", msg);
            errorList.add(msg);

            return;
        }

        // 2) The folder must be readable and writable
        if (!gamesFolder.canRead() || !gamesFolder.canWrite()) {
            // TODO Replace the text by a R.string
            // String msg = context.getString(R.string.path_not_readable).replace("$PATH", path);
            String msg = "The app doesn't have read or write access to the games folder";
            Log.e("EasyRPG", msg);
            errorList.add(msg);

            return;
        }

        // Scan the games folder
        // TODO : Bring back depth (2) when the performance hit will be solved, the problem is linked with slow SAF calls
        scanFolder(gamesFolder, GAME_SCANNING_DEPTH);

        // Sort the games list : alphabetically ordered, favorite in first
        Collections.sort(gameList);

        // If the scan brings nothing in this folder : we notify the errorList
        if (gameList.size() <= 0) {
            String error = context.getString(R.string.no_games_found_and_explanation);
            errorList.add(error);
        }

        Log.i("EasyRPG", gameList.size() + " games found : " + gameList);
    }

    private void scanFolder(DocumentFile gamesFolder, int depth) {
        if (depth > 0) {
            for (DocumentFile file : gamesFolder.listFiles()) {
                String name = file.getName();
                if (!name.startsWith(".")) {
                    // Is it a soundfont file ?
                    // We test this case here during the game scanning in order to avoid further syscalls
                    if (depth == GAME_SCANNING_DEPTH && name.toLowerCase().endsWith(".sf2")) {
                        SettingsManager.setSoundFountFile(file);
                    }
                    // Is the file/folder a RPG Maker game?
                    else if (GameBrowserHelper.isRpg2kGame(file)) {
                        gameList.add(new Game(file));
                    } else if (depth > 1) {
                        // Not a RPG2k Game but a directory -> recurse
                        // (We don't check if it's a directory or if its readable  because of slow
                        // Android SAF calls and scanFolder(...) already check that)
                        scanFolder(file, depth - 1);
                    }
                }
            }
        }
    }

    public List<Game> getGameList() {
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
    public static Bitmap getGameTitleScreen(Game game, Activity context) {
        // Retrieve the Title folder, containing titles screens
        DocumentFile titleFolder = game.getGameFolder().findFile("Title");

        // Display the first image found
        if (titleFolder != null && titleFolder.isDirectory()) {
            for (DocumentFile file : titleFolder.listFiles()) {
                String fileName = file.getName().toLowerCase().trim();
                try {
                    if (!fileName.startsWith(".")) {
                        if (fileName.endsWith("png") || fileName.endsWith("bmp")) {
                            Uri imageUri = file.getUri();
                            return MediaStore.Images.Media.getBitmap(context.getContentResolver(), imageUri);
                        } else if (fileName.endsWith("xyz")) {
                            Uri imageUri = file.getUri();
                            Bitmap b = MediaStore.Images.Media.getBitmap(context.getContentResolver(), imageUri);
                            if (b == null && GameBrowserActivity.libraryLoaded) {
                                // Check for XYZ
                                try (ParcelFileDescriptor fd = context.getContentResolver().openFile(imageUri, "r", null)) {
                                    byte[] xyz = decodeXYZ(fd.detachFd());
                                    if (xyz == null) {
                                        return null;
                                    }
                                    return BitmapFactory.decodeByteArray(xyz, 0, xyz.length);
                                } catch (IOException e) {
                                    return null;
                                }
                            }
                            return b;
                        }
                    }
                } catch (Exception e) {
                    continue;
                }
            }
        }

        return null;
    }

    private static native byte[] decodeXYZ(int fd);
}
