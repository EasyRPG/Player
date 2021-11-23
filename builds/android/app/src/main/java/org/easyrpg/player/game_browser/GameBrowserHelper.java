package org.easyrpg.player.game_browser;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.preference.PreferenceManager;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.documentfile.provider.DocumentFile;

import android.util.Log;
import android.widget.Toast;

import org.easyrpg.player.Helper;
import org.easyrpg.player.R;
import org.easyrpg.player.player.EasyRpgPlayerActivity;
import org.easyrpg.player.settings.SettingsMainActivity;
import org.easyrpg.player.settings.SettingsManager;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.channels.FileChannel;
import java.util.ArrayList;

public class GameBrowserHelper {

    private final static String INI_FILE = "RPG_RT.ini";

    private final static String TAG_FIRST_LAUNCH = "FIRST_LAUNCH";
    private static int GRANTED_PERMISSION = 0;
    public static int FOLDER_HAS_BEEN_CHOSEN = 1;

    /**
     * Returns Ini File of game at index.
     * Optionally creates it.
     *
     * @param create create ini if not found
     * @return ini
     */
    public static File getIniOfGame(String path, boolean create) {
        File dir = new File(path);

        if (!dir.isDirectory() || !dir.canRead()) {
            return null;
        }

        for (File entry : dir.listFiles()) {
            if (entry.isFile() && entry.canRead()) {
                if (entry.getName().equalsIgnoreCase(INI_FILE)) {
                    return entry;
                }
            }
        }

        if (create) {
            File newIni = new File(dir.getAbsolutePath() + "/RPG_RT.ini");

            try {
                newIni.createNewFile();
            } catch (IOException e) {
                return null;
            }

            return newIni;
        }

        return null;
    }

    public static Boolean canWrite(File f) {
        if (f.isDirectory()) {
            FileWriter w = null;
            String testFilename = f.getPath() + "/.EASYRPG_WRITE_TEST";
            try {
                w = new FileWriter(testFilename);
                // Permissions are checked on open, but it is Android, better be save
                w.write("Android >.<");
            } catch (IOException e) {
                return false;
            } finally {
                try {
                    if (w != null) {
                        w.close();
                    }
                } catch (IOException e) {
                }
            }

            File testFile = new File(testFilename);
            if (testFile.exists()) {
                // Does not throw
                testFile.delete();
            }
        } else {
            boolean deleteAfter = f.exists();
            try {
                FileWriter w = new FileWriter(f, true);
                w.close();
            } catch (IOException e) {
                return false;
            }

            if (deleteAfter) {
                f.delete();
            }
        }

        return true;
    }

    // https://stackoverflow.com/q/106770/
    public static void copyFile(File sourceFile, File destFile) throws IOException {
        if (!destFile.exists()) {
            destFile.createNewFile();
        }

        FileChannel source = null;
        FileChannel destination = null;

        try {
            source = new FileInputStream(sourceFile).getChannel();
            destination = new FileOutputStream(destFile).getChannel();
            destination.transferFrom(source, 0, source.size());
        } finally {
            if (source != null) {
                source.close();
            }
            if (destination != null) {
                destination.close();
            }
        }
    }

    private static boolean saveDirectoryContainsSave(Game project) {
        if (project.getGameFolderPath().equals(project.getSavePath())) {
            // Doesn't matter because this is used for the copying logic to the save directory
            return true;
        }

        File[] files = getSavegames(new File(project.getSavePath()));
        return files.length > 0;
    }

    private static void copySavesFromGameDirectoryToSaveDirectory(Game project) {
        if (project.getGameFolderPath().equals(project.getSavePath())) {
            return;
        }

        File[] files = getSavegames(new File(project.getGameFolderPath()));
        for (final File fileEntry : files) {
            try {
                copyFile(fileEntry, new File(project.getSavePath() + "/" + fileEntry.getName()));
            } catch (IOException e) {
            }
        }
    }

    public static File[] getSavegames(File folder) {
        File[] files = folder.listFiles();
        ArrayList<File> saveFiles = new ArrayList<File>();
        if (files != null) {
            for (final File fileEntry : files) {
                if (fileEntry.isFile()) {
                    if (fileEntry.getName().toLowerCase().endsWith(".lsd")) {
                        saveFiles.add(fileEntry);
                    }
                }
            }
        }
        return saveFiles.toArray(new File[saveFiles.size()]);
    }

    public static void launchGame(Context context, Game game) {
        // Prepare savegames, copy them to the save directory on launch to prevent unwanted side effects
        // e.g. games copied from PC with savegames, or from internal storage.
        if (!saveDirectoryContainsSave(game)) {
            copySavesFromGameDirectoryToSaveDirectory(game);
        }

        String path = game.getGameFolderPath();

        // Test again in case somebody messed with the file system
        if (game.getGameFolder().isDirectory() && game.getGameFolder().canRead()) {
            Intent intent = new Intent(context, EasyRpgPlayerActivity.class);
            ArrayList<String> args = new ArrayList<String>();

            // Path of game passed to PlayerActivity via intent "project_path"
            // Command line passed via intent "command_line"
            args.add("--project-path");
            args.add(path);

            args.add("--save-path");
            args.add(game.getSavePath());

            args.add("--encoding");
            if (game.getEncoding() == null || game.getEncoding().length() == 0) {
                args.add("auto");
            } else {
                args.add(game.getEncoding());
            }

            // Soundfont
            DocumentFile soundfont = SettingsManager.getSoundFountFile();
            if (soundfont != null) {
                args.add("--soundfont");
                args.add(soundfont.getUri().toString());
            }

            // Disable audio depending on user preferences
            if (!SettingsManager.isAudioEnabled()) {
                args.add("--disable-audio");
            }

            intent.putExtra(EasyRpgPlayerActivity.TAG_SAVE_PATH, game.getSavePath());
            intent.putExtra(EasyRpgPlayerActivity.TAG_PROJECT_PATH, path);
            intent.putExtra(EasyRpgPlayerActivity.TAG_COMMAND_LINE, args.toArray(new String[args.size()]));

            Log.i("EasyRPG", "Start EasyRPG Player with following arguments : " + args.toString());
            context.startActivity(intent);
        } else {
            String msg = context.getString(R.string.not_valid_game).replace("$PATH", game.getTitle());
            Toast.makeText(context, msg, Toast.LENGTH_LONG).show();
        }
    }

    public static void openSettingsActivity(Context context) {
        Intent intent = new Intent(context, SettingsMainActivity.class);
        context.startActivity(intent);
    }

    public static void displayHowToMessageOnFirstStartup(Context context) {
        // First launch : display the "how to use" dialog box
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        boolean first_launch = preferences.getBoolean(TAG_FIRST_LAUNCH, true);
        if (first_launch) {
            // Displaying the "how to use" dialog box
            displayHowToUseEasyRpgDialog(context);

            // Set FIRST_LAUNCH to false
            SharedPreferences.Editor editor = preferences.edit();
            editor.putBoolean(TAG_FIRST_LAUNCH, false);
            editor.commit();
        }
    }

    /**
     * Prepare and display the dialog box explaining how to use EasyRPG
     */
    public static void displayHowToUseEasyRpgDialog(Context context) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);

        // Dialog construction
        builder.setMessage(R.string.how_to_use_easy_rpg_explanation)
                .setTitle(R.string.how_to_use_easy_rpg)
                .setNeutralButton(R.string.ok, null);

        builder.create();
        builder.show();
    }

    public static void askForStoragePermission(Activity context) {
        if (ContextCompat.checkSelfPermission(context, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(context,
                    new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, GRANTED_PERMISSION);
        }
    }

    /** Open the SAF to ask for a games folder */
    public static void pickAGamesFolder(Activity activity){
        // Choose a directory using the system's file picker.
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        // TODO : Understand why persistable permissions won't work with API 30
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION
            | Intent.FLAG_GRANT_WRITE_URI_PERMISSION
            | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION
            | Intent.FLAG_GRANT_PREFIX_URI_PERMISSION);

        // Optionally, specify a URI for the directory that should be opened in the system file picker when it loads.
        // TODO : Find a way to open the file picker in the user's root folder
        // intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, Uri.parse("/tree/primary"));

        activity.startActivityForResult(intent, GameBrowserHelper.FOLDER_HAS_BEEN_CHOSEN);
    }

    /** Take into account the games folder chose by the user */
    public static void dealAfterFolderSelected(Activity activity, int requestCode, int resultCode, Intent resultData) {
        if (requestCode == GameBrowserHelper.FOLDER_HAS_BEEN_CHOSEN
            && resultCode == Activity.RESULT_OK
            && resultData != null) {

            // Extract the selected folder from the URI
            Uri uri = resultData.getData();
            Log.i("EasyRPG", "The selected games folder is : " + uri.getPath());

            // Ask for permanent access to this folder
            final int takeFlags = resultData.getFlags()
                & (Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
            activity.getContentResolver().takePersistableUriPermission(uri, takeFlags);

            // Save the settings
            SettingsManager.setGameFolder(uri);

            // Create RTP folders and the .nomedia file
            DocumentFile gamesFolder = SettingsManager.getGameFolder();
            Helper.createEasyRPGDirectories(gamesFolder);
        }
    }
}
