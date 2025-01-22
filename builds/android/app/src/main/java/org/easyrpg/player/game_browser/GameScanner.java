package org.easyrpg.player.game_browser;

import android.app.Activity;
import android.content.Context;
import android.net.Uri;
import android.util.Log;
import android.widget.TextView;

import androidx.documentfile.provider.DocumentFile;

import org.easyrpg.player.Helper;
import org.easyrpg.player.R;
import org.easyrpg.player.settings.SettingsManager;
import org.libsdl.app.SDL;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

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

    public static GameScanner getInstance(Activity activity) {
        // Singleton pattern
        if (GameScanner.instance == null) {
            synchronized(GameScanner.class) {
                if (GameScanner.instance == null) {
                    GameScanner.instance = new GameScanner();
                }
            }
        }

        //Scan the folder
        instance.scanGames(activity);

        return instance;
    }

    private void scanGames(Activity activity){
        Context context = activity.getApplicationContext();

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

            if (!gameList.isEmpty()) {
                Log.i("EasyRPG", gameList.size() + " game(s) found in cache.");

                return;
            }

            // Bad cache, do a scan
            SettingsManager.clearGamesCache();
        }

        // Scan the games folder
        scanRootFolder(activity, gamesFolder.getUri());

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

        sb.append("3"); // Bump this when the cache layout changes
        for (String[] array : Helper.listChildrenDocuments(context, folderURI)) {
            sb.append(array[0]);
            sb.append(array[1]);
        }

        return sb.toString().hashCode();
    }

    private void scanRootFolder(Activity activity, Uri folderURI) {
        Context context = activity.getApplicationContext();
        SDL.setContext(context);

        final ArrayList<String> names = new ArrayList<>();
        final ArrayList<Uri> fileURIs = new ArrayList<>();

        // Precalculate how many folders are to be scanned
        for (String[] array : Helper.listChildrenDocuments(context, folderURI)) {
            String fileDocumentID = array[0];
            String name = array[2];

            if (name.isEmpty()) {
                continue;
            }

            if (!name.startsWith(".")) {
                Uri fileURI = Helper.getURIFromDocumentID(folderURI, fileDocumentID);
                names.add(name);
                fileURIs.add(fileURI);
            }
        }

        // Scan all the folders and show the current scanning progress
        for (int i = 0; i < names.size(); ++i) {
            final String name = names.get(i); // only "final" variables can be passed to lambdas
            final int j = i;
            activity.runOnUiThread(() -> {
                // Update Ui progress
                TextView myTextView = activity.findViewById(R.id.progressText);
                myTextView.setText(String.format("%s (%d/%d)", name, j + 1, names.size()));
            });

            Game[] candidates = findGames(fileURIs.get(i).toString(), names.get(i));

            if (candidates == null) {
                continue;
            }

            for (Game candidate: candidates) {
                if (candidate != null) {
                    gameList.add(candidate);
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

    private static native Game[] findGames(String path, String mainFolderName);
}
