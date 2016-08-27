package org.easyrpg.player.game_browser;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.widget.Toast;

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
import java.util.LinkedList;

public class GameBrowserHelper {
    //Files' names
    private final static String DATABASE_NAME = "RPG_RT.ldb", TREEMAP_NAME = "RPG_RT.lmt", INI_FILE = "RPG_RT.ini";

    private final static String TAG_FIRST_LAUNCH = "FIRST_LAUNCH";
    private static int GRANTED_PERMISSION = 0;

    /**
     * Tests if a folder is a RPG2k Game.
     * (contains DATABASE_NAME and TREEMAP_NAME)
     *
     * @param dir Directory to test
     * @return true if RPG2k game
     */
    public static boolean isRpg2kGame(File dir) {
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

    private static boolean saveDirectoryContainsSave(GameInformation project) {
        if (project.getGameFolderPath().equals(project.getSavePath())) {
            // Doesn't matter because this is used for the copying logic to the save directory
            return true;
        }

        File[] files = getSavegames(new File(project.getSavePath()));
        return files.length > 0;
    }

    private static void copySavesFromGameDirectoryToSaveDirectory(GameInformation project) {
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

    public static void launchGame(Context context, GameInformation project) {
        // Prepare savegames, copy them to the save directory on launch to prevent unwanted side effects
        // e.g. games copied from PC with savegames, or from internal storage.
        if (!saveDirectoryContainsSave(project)) {
            copySavesFromGameDirectoryToSaveDirectory(project);
        }

        String path = project.getGameFolderPath();

        // Test again in case somebody messed with the file system
        if (GameBrowserHelper.isRpg2kGame(new File(path))) {
            Intent intent = new Intent(context, EasyRpgPlayerActivity.class);
            ArrayList<String> args = new ArrayList<String>();

            // Path of game passed to PlayerActivity via intent "project_path"
            // Command line passed via intent "command_line"
            args.add("--project-path");
            args.add(path);

            args.add("--save-path");
            args.add(project.getSavePath());

            args.add("--encoding");
            if (project.getEncoding() == null || project.getEncoding().length() == 0) {
                args.add("auto");
            } else {
                args.add(project.getEncoding());
            }

            // Disable audio depending on user preferences
            if (!SettingsManager.isAudioEnabled()) {
                args.add("--disable-audio");
            }

            intent.putExtra(EasyRpgPlayerActivity.TAG_SAVE_PATH, project.getSavePath());
            intent.putExtra(EasyRpgPlayerActivity.TAG_PROJECT_PATH, path);
            intent.putExtra(EasyRpgPlayerActivity.TAG_COMMAND_LINE, args.toArray(new String[args.size()]));
            context.startActivity(intent);
        } else {
            String msg = context.getString(R.string.not_valid_game).replace("$PATH", project.getTitle());
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
}
