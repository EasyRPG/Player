package org.easyrpg.player.game_browser;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.preference.PreferenceManager;
import android.util.Log;
import android.widget.Toast;

import androidx.documentfile.provider.DocumentFile;

import org.easyrpg.player.Helper;
import org.easyrpg.player.R;
import org.easyrpg.player.player.EasyRpgPlayerActivity;
import org.easyrpg.player.settings.SettingsMainActivity;
import org.easyrpg.player.settings.SettingsManager;

import java.util.ArrayList;

public class GameBrowserHelper {

    private final static String TAG_FIRST_LAUNCH = "FIRST_LAUNCH";
    public static int FOLDER_HAS_BEEN_CHOSEN = 1;

    public static void launchGame(Context context, Game game) {
        launchGame(context, game, false);
    }

    public static void launchGame(Context context, Game game, boolean debugMode) {
        String path = game.getGameFolderPath();

        // Test again in case somebody messed with the file system
        boolean valid = game.isStandalone() ||
            (game.isZipArchive() && game.getGameFolder().canRead()) ||
            (game.getGameFolder().isDirectory() && game.getGameFolder().canRead());

        if (valid) {
            Intent intent = new Intent(context, EasyRpgPlayerActivity.class);
            ArrayList<String> args = new ArrayList<>();

            // Command line passed via intent "command_line"
            String savePath;

            if (game.isZipArchive()) {
                // Create the redirected save folder
                DocumentFile saveFolder = Helper.createFolderInSave(context, game.getSavePath());

                args.add("--project-path");
                args.add(path + "/" + game.getZipInnerPath());

                // In error case the native code will try to put a save folder next to the zip
                if (saveFolder != null) {
                    savePath = saveFolder.getUri().toString();
                    args.add("--save-path");
                    args.add(savePath);
                } else {
                    savePath = path;
                }
            } else {
                args.add("--project-path");
                args.add(path);

                savePath = game.getSavePath();
                args.add("--save-path");
                args.add(savePath);
            }

            Encoding enc = game.getEncoding();
            if (enc.getIndex() > 0) {
                // 0 = Auto, in that case let the Player figure it out
                args.add("--encoding");
                args.add(enc.getRegionCode());
            }

            args.add("--config-path");
            args.add(context.getExternalFilesDir(null).getAbsolutePath());

            // Soundfont
            Uri soundfontUri = SettingsManager.getSoundFountFileURI(context);
            if (soundfontUri != null) {
                args.add("--soundfont");
                args.add(soundfontUri.toString());
            }

            if (debugMode) {
                args.add("--test-play");
            }

            intent.putExtra(EasyRpgPlayerActivity.TAG_SAVE_PATH, savePath);
            intent.putExtra(EasyRpgPlayerActivity.TAG_COMMAND_LINE, args.toArray(new String[0]));
            intent.putExtra(EasyRpgPlayerActivity.TAG_STANDALONE, game.isStandalone());

            Log.i("EasyRPG", "Start EasyRPG Player with following arguments : " + args);
            Log.i("EasyRPG", "The RTP folder is : " + SettingsManager.getRTPFolderURI(context));
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
            editor.apply();
        }
    }

    /**
     * Prepare and display the dialog box explaining how to use EasyRPG
     */
    public static void displayHowToUseEasyRpgDialog(Context context) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);

        // Dialog construction
        builder.setMessage(R.string.how_to_use_easy_rpg_explanation_android_30)
                .setTitle(R.string.how_to_use_easy_rpg)
                .setNeutralButton(R.string.ok, null);

        builder.create();
        builder.show();
    }

    /** Open the SAF to ask for a games folder */
    public static void pickAGamesFolder(Activity activity){
        // Choose a directory using the system's file picker.
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION
            | Intent.FLAG_GRANT_WRITE_URI_PERMISSION
            | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION
            | Intent.FLAG_GRANT_PREFIX_URI_PERMISSION);

        activity.startActivityForResult(intent, GameBrowserHelper.FOLDER_HAS_BEEN_CHOSEN);
    }

    /** Take into account the games folder chose by the user */
    public static void dealAfterFolderSelected(Activity activity, int requestCode, int resultCode, Intent resultData) {
        if (requestCode == GameBrowserHelper.FOLDER_HAS_BEEN_CHOSEN
            && resultCode == Activity.RESULT_OK
            && resultData != null) {

            // Extract the selected folder from the URI
            Uri uri = resultData.getData();
            Log.i("EasyRPG", "The selected EasyRPG folder is : " + uri.getPath());

            // Ask for permanent access to this folder
            final int takeFlags = resultData.getFlags()
                & (Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
            activity.getContentResolver().takePersistableUriPermission(uri, takeFlags);

            // Save the settings
            SettingsManager.setEasyRPGFolderURI(uri);

            // Create EasyRPG folders and the .nomedia file
            Uri easyRPGFolderURI = SettingsManager.getEasyRPGFolderURI(activity);
            Helper.createEasyRPGFolders(activity, easyRPGFolderURI);
        }
    }
}
