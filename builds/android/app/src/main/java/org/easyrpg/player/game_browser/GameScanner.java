package org.easyrpg.player.game_browser;

import android.content.ContentResolver;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Build;
import android.os.ParcelFileDescriptor;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.util.Log;

import androidx.documentfile.provider.DocumentFile;

import org.easyrpg.player.Helper;
import org.easyrpg.player.R;
import org.easyrpg.player.settings.SettingsManager;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class GameScanner {
    // We use a singleton pattern to allow further optimizations
    // (Such as caching games' thumbnail, avoiding some sys call ...)
    private static volatile GameScanner instance = null;

    //Files' names
    private final static String DATABASE_NAME = "rpg_rt.ldb", TREEMAP_NAME = "rpg_rt.lmt", INI_FILE = "rpg_rt.ini", EXE_FILE = "rpg_rt.exe";

    private final List<Game> gameList;
    private final List<String> errorList; // The list of errors that will be displayed in case of problems during the scan

    private static final int GAME_SCANNING_DEPTH = 2; // 1 = no recursive scanning

    private GameScanner() {
        this.gameList = new ArrayList<>();
        this.errorList = new ArrayList<>();
    }

    public static GameScanner getInstance(Context context) {
        // Singleton pattern
        if (GameScanner.instance == null) {
            synchronized(GameScanner.class) {
                if (GameScanner.instance == null) {
                    GameScanner.instance = new GameScanner();
                }
            }
        }

        //Scan the folder
        instance.scanGames(context);

        return instance;
    }

    private void scanGames(Context context){
        gameList.clear();
        errorList.clear();

        // Retrieve the games folder
        Uri gamesFolderURI = SettingsManager.getGamesFolderURI(context);
        DocumentFile gamesFolder = Helper.getFileFromURI(context, gamesFolderURI);

        // 1) The folder must exist
        if (gamesFolder == null || !gamesFolder.isDirectory()) {
            String errMsg = context.getString(R.string.error_no_games_folder);
            Log.e("EasyRPG", errMsg);
            errorList.add(errMsg);

            return;
        }

        // 2) The folder must be readable and writable
        if (!gamesFolder.canRead() || !gamesFolder.canWrite()) {
            String errMsg = context.getString(R.string.error_games_no_rw);
            Log.e("EasyRPG", errMsg);
            errorList.add(errMsg);

            return;
        }

        // Scan the first level of the games folder and compute a hash from it
        int hash = scanFolderHash(context, gamesFolder.getUri());
        int cacheHash = SettingsManager.getGamesCacheHash();
        Log.i("EasyRPG", "Hash: " + hash + "/" + cacheHash);

        if (hash == cacheHash) {
            Set<String> gamesCache = SettingsManager.getGamesCache();
            for (String gameCache: gamesCache) {
                Game game = Game.fromCacheEntry(context, gameCache);
                if (game != null) {
                    gameList.add(game);
                }
            }

            if (gameList.size() > 0) {
                Log.i("EasyRPG", gameList.size() + " game(s) found in cache.");

                return;
            }

            // Bad cache, do a scan
            SettingsManager.clearGamesCache();
        }

        // Scan the games folder
        scanFolderRecursive(context, gamesFolder.getUri(), GAME_SCANNING_DEPTH);

        // If the scan brings nothing in this folder : we notify the errorList
        if (gameList.size() <= 0) {
            SettingsManager.clearGamesCache();
            String error = context.getString(R.string.no_games_found_and_explanation_android_30);
            errorList.add(error);
        } else {
            // Store the result in the cache
            Set<String> gamesCache = new HashSet<>();
            for (Game game: gameList) {
                gamesCache.add(game.toCacheEntry());
            }
            SettingsManager.setGamesCache(hash, gamesCache);
        }

        Log.i("EasyRPG", gameList.size() + " game(s) found.");
    }

    private int scanFolderHash(Context context, Uri folderURI) {
        StringBuilder sb = new StringBuilder();

        for (String[] array : Helper.listChildrenDocumentIDAndType(context, folderURI)) {
            sb.append(array[0]);
            sb.append(array[1]);
        }

        return sb.toString().hashCode();
    }

    private void scanFolderRecursive(Context context, Uri folderURI, int depth) {
        if (depth > 0) {
            for (String[] array : Helper.listChildrenDocumentIDAndType(context, folderURI)) {
                String fileDocumentID = array[0];
                String fileDocumentType = array[1];

                String name = Helper.getFileNameFromDocumentID(fileDocumentID);
                if (name.equals("")) {
                    continue;
                }
                if (!name.startsWith(".")) {
                    boolean isDirectory = Helper.isDirectoryFromMimeType(fileDocumentType);
                    if (isDirectory) {
                        // Is the file/folder a RPG Maker game?
                        Uri fileURI = Helper.getURIFromDocumentID(folderURI, fileDocumentID);
                        Game game = isAGame(context, fileURI);
                        if (game != null) {
                            gameList.add(game);
                        }
                        else if (depth > 1) {
                            // Not a RPG2k Game but a directory -> recurse
                            // (We don't check if it's a directory or if its readable  because of slow
                            // Android SAF calls and scanFolder(...) already check that)
                            scanFolderRecursive(context, fileURI, depth - 1);
                        }
                    } else {
                        String nameLower = name.toLowerCase(Locale.ROOT);
                        if (nameLower.endsWith(".zip") || nameLower.endsWith(".easyrpg")) {
                            Uri fileURI = Helper.getURIFromDocumentID(folderURI, fileDocumentID);
                            Game game = isAGameZipped(context, fileURI, true);
                            if (game != null) {
                                gameList.add(game);
                            }
                        }
                    }
                }
            }
        }
    }

    /** Return a game if "folder" is a game folder, or return null.
     *  This method is designed to reduce the number of sys calls */
    public static Game isAGame(Context context, Uri uri) {
        Game game = null;
        Uri titleFolderURI = null;

        // Create a lookup by extension as we go, in case we are dealing with non-standard extensions.
        int rpgRtCount = 0;
        boolean databaseFound = false;
        boolean treemapFound = false;
        boolean isARpgGame = false;

        for (String filePath : Helper.listChildrenDocumentID(context, uri)) {
            String fileName = Helper.getFileNameFromDocumentID(filePath);
            String fileNameLower = fileName.toLowerCase(Locale.ROOT);

            if (!databaseFound && fileName.equals(DATABASE_NAME)) {
                databaseFound = true;
            } else if (!treemapFound && fileName.equals(TREEMAP_NAME)) {
                treemapFound = true;
            }
            // Count non-standard files.
            // NOTE: Do not put this in the 'else' statement, since only 1 extension may be non-standard and we want to count both.
            // We might be dealing with a non-standard extension.
            // Show it, and let the C++ code sort out which file is which.
            if (fileNameLower.startsWith("rpg_rt.")) {
                if (!(fileName.equals(INI_FILE) || fileName.equals(EXE_FILE))) {
                    rpgRtCount += 1;
                }
            }

            if ((databaseFound && treemapFound) || rpgRtCount == 2) {
                isARpgGame = true;
            }

            // If we encounter a Title folder, we keep it for the title screen
            // We do that here in order to avoid syscalls
            if (fileNameLower.equals("title")) {
                titleFolderURI = DocumentsContract.buildDocumentUriUsingTree(uri, filePath);
            }
        }

        if (isARpgGame) {
            Bitmap titleScreen = GameScanner.extractTitleScreenImage(context, titleFolderURI);
            game = new Game(Helper.getFileFromURI(context, uri), titleScreen);
        }

        return game;
    }

    static class ZipFoundStats {
        int rpgRtCount = 0;
        boolean databaseFound = false;
        boolean treemapFound = false;
        boolean isARpgGame = false;
        byte[] titleImage = null;

        ZipFoundStats() {
        }
    }

    /** Return a game if "folder" is a game folder, or return null.
     *  This method is designed to reduce the number of sys calls */
    public static Game isAGameZipped(Context context, Uri zipUri, boolean unicode) {
        ContentResolver resolver = context.getContentResolver();

        // Create a lookup by extension as we go, in case we are dealing with non-standard extensions.
        Map<String, ZipFoundStats> games = new HashMap<>();

        try (InputStream zipIStream = resolver.openInputStream(zipUri)) {
            ZipInputStream zipStream;
            if (unicode) {
                zipStream = new ZipInputStream(zipIStream);
            } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                // arbitrary encoding that does not crash
                zipStream = new ZipInputStream(zipIStream, StandardCharsets.ISO_8859_1);
            } else {
                return null;
            }

            ZipEntry entry;

            while ((entry = zipStream.getNextEntry()) != null) {
                if (entry.isDirectory()) {
                    continue;
                }

                String fullPath = entry.getName();
                String fileName;

                if (fullPath.isEmpty()) {
                    continue;
                }

                int slash = fullPath.lastIndexOf('/');
                if (slash == -1) {
                    slash = fullPath.lastIndexOf('\\');
                }

                if (slash == -1) {
                    fileName = fullPath;
                } else if (slash == fullPath.length() - 1) {
                    continue;
                } else {
                    fileName = fullPath.substring(slash + 1);
                }

                String fileNameLower = fileName.toLowerCase(Locale.ROOT);

                String gameDirectory;
                if (slash <= 0) {
                    gameDirectory = "";
                } else {
                    gameDirectory = fullPath.substring(0, slash);
                }

                String gameDirectoryLower = gameDirectory.toLowerCase(Locale.ROOT);
                if (gameDirectoryLower.endsWith("/title") || gameDirectoryLower.endsWith("\\title")) {
                    // Check for a title image
                    ZipFoundStats stats = games.get(gameDirectory.substring(0, gameDirectory.length() - "/title".length()));

                    if (stats != null) {
                        if (fileNameLower.endsWith(".xyz") || fileNameLower.endsWith(".png") || fileNameLower.endsWith(".bmp")) {
                            int count;
                            try (ByteArrayOutputStream out = new ByteArrayOutputStream()) {
                                byte[] buffer = new byte[16 * 1024];
                                while ((count = zipStream.read(buffer)) != -1)
                                    out.write(buffer, 0, count);
                                stats.titleImage = out.toByteArray();
                                if (stats.isARpgGame) {
                                    break;
                                }
                            }
                        }
                    }

                    continue;
                }

                ZipFoundStats stats = games.get(gameDirectory);

                if (stats == null) {
                    stats = new ZipFoundStats();
                    games.put(gameDirectory, stats);
                }

                if (!stats.databaseFound && fileNameLower.equals(DATABASE_NAME)) {
                    stats.databaseFound = true;
                } else if (!stats.treemapFound && fileNameLower.equalsIgnoreCase(TREEMAP_NAME)) {
                    stats.treemapFound = true;
                }

                // Count non-standard files.
                // NOTE: Do not put this in the 'else' statement, since only 1 extension may be non-standard and we want to count both.
                // We might be dealing with a non-standard extension.
                // Show it, and let the C++ code sort out which file is which.
                if (fileNameLower.startsWith("rpg_rt.")) {
                    if (!(fileNameLower.equals(INI_FILE) || fileNameLower.equals(EXE_FILE))) {
                        stats.rpgRtCount += 1;
                    }
                }

                if ((stats.databaseFound && stats.treemapFound) || stats.rpgRtCount == 2) {
                    stats.isARpgGame = true;
                    if (stats.titleImage != null) {
                        break;
                    }
                }
            }
        } catch (IOException e) {
            return null;
        } catch (IllegalArgumentException e) {
            if (unicode && Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                return isAGameZipped(context, zipUri, false);
            }
            return null;
        }

        for (Map.Entry<String, ZipFoundStats> entry : games.entrySet()) {
            if (entry.getValue().isARpgGame) {
                String name = new File(zipUri.getPath()).getName();
                String saveFolder = name.substring(0, name.lastIndexOf("."));
                Bitmap titleScreen = extractTitleScreenImage(context, entry.getValue().titleImage);
                String key = entry.getKey();
                if (!unicode) {
                    // FIXME: This will launch the built-in Game Browser when the ZIP archive contains more than one folder in the root
                    // But the game can be at least launched this way.
                    key = "";
                }
                return Game.fromZip(Helper.getFileFromURI(context, zipUri), key, saveFolder, titleScreen);
            }
        }

        return null;
    }

    /** The VERY SLOW way of testing if a folder is a RPG2k Game. (contains DATABASE_NAME and TREEMAP_NAME)
     *  <!> It shouldn't be use unless Google forces the use of DocumentFile over ContentResolver
     * @param dir The directory to test
     * @return true if RPG2k game
     */
    @Deprecated
    public static boolean isAGameSlowWay(DocumentFile dir) {
        if (!dir.isDirectory() || !dir.canRead()) {
            return false;
        }

        boolean databaseFound = false;
        boolean treemapFound = false;

        // Create a lookup by extension as we go, in case we are dealing with non-standard extensions.
        int rpgRtCount = 0;

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
                        rpgRtCount += 1;
                    }
                }

                if (databaseFound && treemapFound) {
                    return true;
                }
            }
        }

        // We might be dealing with a non-standard extension.
        // Show it, and let the C++ code sort out which file is which.
        return rpgRtCount == 2;
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
    public static Bitmap extractTitleScreenImage(Context context, byte[] titleScreenBuffer) {
        if (titleScreenBuffer == null) {
            return null;
        }

        Bitmap bmp = BitmapFactory.decodeByteArray(titleScreenBuffer, 0, titleScreenBuffer.length);

        if (bmp == null) {
            byte[] xyz = decodeXYZbuffer(titleScreenBuffer);
            if (xyz == null) {
                return null;
            }
            return BitmapFactory.decodeByteArray(xyz, 0, xyz.length);
        }

        return bmp;
    }

    /** Return the game title screen, in a dumb way following last Enterbrain conventions */
    public static Bitmap extractTitleScreenImage(Context context, Uri titleScreenFolderURI) {
        try {
            // Retrieve the Title folder, containing titles screens
            DocumentFile titleFolder = Helper.getFileFromURI(context, titleScreenFolderURI);

            if (titleFolder != null && titleFolder.isDirectory()) {

                // Display the first image found in the Title folder
                for (String fileID : Helper.listChildrenDocumentID(context, titleScreenFolderURI)) {
                    String fileName = Helper.getFileNameFromDocumentID(fileID).toLowerCase().trim();

                    if (!fileName.startsWith(".")) {
                        if (fileName.endsWith("png") || fileName.endsWith("bmp")) {
                            Uri imageUri = Helper.getURIFromDocumentID(titleScreenFolderURI, fileID);
                            return MediaStore.Images.Media.getBitmap(context.getContentResolver(), imageUri);
                        }
                        else if (fileName.endsWith("xyz")) {
                            Uri imageUri = Helper.getURIFromDocumentID(titleScreenFolderURI, fileID);
                            Bitmap b = MediaStore.Images.Media.getBitmap(context.getContentResolver(), imageUri);
                            if (b == null && GameBrowserActivity.libraryLoaded) {
                                // Check for XYZ
                                try (ParcelFileDescriptor fd = context.getContentResolver().openFileDescriptor(imageUri, "r")) {
                                    byte[] xyz = decodeXYZfd(fd.detachFd());
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

    private static native byte[] decodeXYZfd(int fd);

    private static native byte[] decodeXYZbuffer(byte[] buffer);
}
