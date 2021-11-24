package org.easyrpg.player.game_browser;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.DocumentsContract;
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

    //Files' names
    private final static String DATABASE_NAME = "RPG_RT.ldb", TREEMAP_NAME = "RPG_RT.lmt", INI_FILE = "RPG_RT.ini", EXE_FILE = "RPG_RT.exe";

    private List<Game> gameList;
    private List<String> errorList; // The list of errors that will be displayed in case of problems during the scan
    private Activity context;

    private static final int GAME_SCANNING_DEPTH = 1; // 1 = no recursive scanning

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

        //Scan the folder
        instance.scanGames();

        return instance;
    }

    private void scanGames(){
        gameList.clear();
        errorList.clear();

        // Retrieve the games folder
        DocumentFile gamesFolder = SettingsManager.getGameFolder();

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
        scanFolderRecursive(gamesFolder, GAME_SCANNING_DEPTH);

        // If the scan brings nothing in this folder : we notify the errorList
        if (gameList.size() <= 0) {
            String error = context.getString(R.string.no_games_found_and_explanation);
            errorList.add(error);
        }

        Log.i("EasyRPG", gameList.size() + " game(s) found.");
    }

    private void scanFolderRecursive(DocumentFile gamesFolder, int depth) {
        // TODO : Avoid to use listFile() for the root of the game folder
        if (depth > 0) {
            for (DocumentFile file : gamesFolder.listFiles()) {
                String name = file.getName();
                if (name == null) {
                    continue;
                }
                if (!name.startsWith(".")) {
                    // Is it a soundfont file ?
                    // We test this case here during the game scanning in order to avoid further syscalls
                    if (depth == GAME_SCANNING_DEPTH && name.toLowerCase().endsWith(".sf2")) {
                        Log.i("EasyRPG", "Soundfont found : " + file.getName());
                        SettingsManager.setSoundFountFile(file);
                    }
                    if (file.isDirectory() && file.canRead()) {
                        // Is the file/folder a RPG Maker game?
                        Game game = isAGame(context, file);
                        if (game != null) {
                            gameList.add(game);
                        }
                        else if (depth > 1) {
                            // Not a RPG2k Game but a directory -> recurse
                            // (We don't check if it's a directory or if its readable  because of slow
                            // Android SAF calls and scanFolder(...) already check that)
                            scanFolderRecursive(file, depth - 1);
                        }
                    }
                }
            }
        }
    }

    /** Return a game if "folder" is a game folder, or return null.
     *  This method is designed to reduce the number of syscalls */
    public static Game isAGame(Context context, DocumentFile folder) {
        Game game = null;
        Uri uri = folder.getUri();
        Uri titleFolderURI = null;

        // Create a lookup by extension as we go, in case we are dealing with non-standard extensions.
        int rpgrtCount = 0;
        boolean databaseFound = false;
        boolean treemapFound = false;
        boolean isARpgGame = false;

        for (String filePath : listFilesWithPathQuickVersion(context, uri)) {
            String fileName = filePath.substring(filePath.lastIndexOf('/') + 1);

            if (!databaseFound && fileName.equalsIgnoreCase(DATABASE_NAME)) {
                databaseFound = true;
            } else if (!treemapFound && fileName.equalsIgnoreCase(TREEMAP_NAME)) {
                treemapFound = true;
            }
            // Count non-standard files.
            // NOTE: Do not put this in the 'else' statement, since only 1 extension may be non-standard and we want to count both.
            // We might be dealing with a non-standard extension.
            // Show it, and let the C++ code sort out which file is which.
            if (fileName.toLowerCase().startsWith("rpg_rt.")) {
                if (!(fileName.equalsIgnoreCase(INI_FILE) || fileName.equalsIgnoreCase(EXE_FILE))) {
                    rpgrtCount += 1;
                }
            }

            if ((databaseFound && treemapFound) || rpgrtCount == 2) {
                isARpgGame = true;
            }

            // If we encounter a Title folder, we keep it for the title screen
            // We do that here in order to avoid syscalls
            if (fileName.equals("Title")) {
                titleFolderURI = DocumentsContract.buildDocumentUriUsingTree(uri, filePath);
            }
        }

        if (isARpgGame) {
            Bitmap titleScreen = GameScanner.extractTitleScreenImage(context, titleFolderURI);
            game = new Game(folder, titleScreen);
        }

        return game;
    }

    /** The VERY SLOW way of testing if a folder is a RPG2k Game. (contains DATABASE_NAME and TREEMAP_NAME)
     *  <!> It shouldn't be use unless Google forces the use of DocumentFile over ContentResolver
     * @param dir The directory to test
     * @return true if RPG2k game
     */
    @Deprecated
    public static boolean isAGame(DocumentFile dir) {
        if (!dir.isDirectory() || !dir.canRead()) {
            return false;
        }

        boolean databaseFound = false;
        boolean treemapFound = false;

        // Create a lookup by extension as we go, in case we are dealing with non-standard extensions.
        int rpgrtCount = 0;

        for (DocumentFile entry : dir.listFiles()) {
            if (entry.isFile() && entry.canRead()) {
                String entryName = entry.getName();
                if (entryName == null) {
                    continue;
                }

                if (!databaseFound && entryName.equalsIgnoreCase(DATABASE_NAME)) {
                    databaseFound = true;
                } else if (!treemapFound && entryName.equalsIgnoreCase(TREEMAP_NAME)) {
                    treemapFound = true;
                }

                // Count non-standard files.
                // NOTE: Do not put this in the 'else' statement, since only 1 extension may be non-standard and we want to count both.
                if (entryName.toLowerCase().startsWith("rpg_rt.")) {
                    if (!(entryName.equalsIgnoreCase(INI_FILE) || entryName.equalsIgnoreCase(EXE_FILE))) {
                        rpgrtCount += 1;
                    }
                }

                if (databaseFound && treemapFound) {
                    return true;
                }
            }
        }

        // We might be dealing with a non-standard extension.
        // Show it, and let the C++ code sort out which file is which.
        if (rpgrtCount == 2) {
            return true;
        }

        return false;
    }

    public static List<String> listFilesWithPathQuickVersion (Context context, Uri self){
        final ContentResolver resolver = context.getContentResolver();
        final Uri childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(self, DocumentsContract.getDocumentId(self));
        List<String> filesList = new ArrayList<>();
        try {
            Cursor c = resolver.query(childrenUri, new String[] { DocumentsContract.Document.COLUMN_DOCUMENT_ID }, null, null, null);
            while (c.moveToNext()) {
                String filePath = c.getString(0);
                filesList.add(filePath);
            }
            c.close();
        } catch (Exception e) {
            Log.e("EasyRPG", "Failed query: " + e);
        }
        return filesList;
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

    /** Return the game title screen, in a dumb way following last Enterbrain conventions */
    public static Bitmap extractTitleScreenImage(Context context, Uri titleScreenFolderURI) {
        try {
            // Retrieve the Title folder, containing titles screens
            DocumentFile titleFolder = DocumentFile.fromTreeUri(context, titleScreenFolderURI);

            // Display the first image found
            if (titleFolder != null && titleFolder.isDirectory()) {
                for (DocumentFile file : titleFolder.listFiles()) {
                    String fileName = file.getName().toLowerCase().trim();

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
                }
            }
        } catch (Exception e) {
            Log.e("EasyRPG", e.getMessage());
        }

        return null;
    }

    private static native byte[] decodeXYZ(int fd);
}
