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
import java.util.List;

public class GameBrowserHelper {
    public enum SafError {
        OK,
        ABORTED,
        DOWNLOAD_SELECTED,
        BAD_CONTENT_PROVIDER_CREATE,

        BAD_CONTENT_PROVIDER_READ,
        BAD_CONTENT_PROVIDER_WRITE,
        BAD_CONTENT_PROVIDER_DELETE,

        BAD_CONTENT_PROVIDER_FILENAME_IGNORED,
        BAD_CONTENT_PROVIDER_BASE_FOLDER_NOT_FOUND,
        BAD_CONTENT_PROVIDER_FILE_ACCESS,
        FOLDER_NOT_ALMOST_EMPTY
    }

    private final static String TAG_FIRST_LAUNCH = "FIRST_LAUNCH";
    public static int FOLDER_HAS_BEEN_CHOSEN = 1;

    public static String VIDEO_URL = "https://youtu.be/r9qU-6P3HOs";

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
        // show storage root on some systems by default
        intent.putExtra("android.content.extra.SHOW_ADVANCED", true);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION
            | Intent.FLAG_GRANT_WRITE_URI_PERMISSION
            | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION
            | Intent.FLAG_GRANT_PREFIX_URI_PERMISSION);

        activity.startActivityForResult(intent, GameBrowserHelper.FOLDER_HAS_BEEN_CHOSEN);
    }

    /** Take into account the games folder chose by the user */
    public static SafError dealAfterFolderSelected(Activity activity, int requestCode, int resultCode, Intent resultData) {
        if (requestCode == GameBrowserHelper.FOLDER_HAS_BEEN_CHOSEN
            && resultCode == Activity.RESULT_OK
            && resultData != null) {

            // Extract the selected folder from the URI
            Uri uri = resultData.getData();

            if (uri.toString().startsWith("content://com.android.providers.downloads")) {
                return SafError.DOWNLOAD_SELECTED;
            }

            // Ask for permanent access to this folder
            final int takeFlags = resultData.getFlags()
                & (Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
            activity.getContentResolver().takePersistableUriPermission(uri, takeFlags);

            // Check if write operations inside the folder work as expected
            SafError error = Helper.testContentProvider(activity, uri);
            if (error != SafError.OK) {
                return error;
            }

            // Check if the folder contains too many normal files already
            DocumentFile folder = Helper.getFileFromURI(activity, uri);
            if (folder == null) {
                return SafError.BAD_CONTENT_PROVIDER_BASE_FOLDER_NOT_FOUND;
            }

            List<String[]> items = Helper.listChildrenDocumentIDAndType(activity, folder.getUri());
            int item_count = 0;
            for (String[] item: items) {
                if (item[0] == null || Helper.isDirectoryFromMimeType(item[1]) || item[0].endsWith(".nomedia")) {
                    continue;
                }

                item_count += 1;

                if (item_count >= 3) {
                    return SafError.FOLDER_NOT_ALMOST_EMPTY;
                }
            }

            // Save the settings
            SettingsManager.setEasyRPGFolderURI(uri);

            // Create EasyRPG folders and the .nomedia file
            Uri easyRPGFolderURI = SettingsManager.getEasyRPGFolderURI(activity);
            Helper.createEasyRPGFolders(activity, easyRPGFolderURI);

            return SafError.OK;
        }

        return SafError.ABORTED;
    }

    public static void showErrorMessage(Context context, SafError error) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setTitle(R.string.error_saf_title)
            .setNeutralButton(R.string.ok, null);

        String errorMsg = "";

        switch (error) {
            case OK:
            case ABORTED:
                break;
            case DOWNLOAD_SELECTED:
                errorMsg = context.getString(R.string.error_saf_download_selected);
                break;
            case BAD_CONTENT_PROVIDER_CREATE:
                errorMsg = context.getString(R.string.error_saf_bad_content_provider);
                errorMsg += context.getString(R.string.error_saf_bad_content_provider_create);
                break;
            case BAD_CONTENT_PROVIDER_READ:
                errorMsg = context.getString(R.string.error_saf_bad_content_provider);
                errorMsg += context.getString(R.string.error_saf_bad_content_provider_read);
                break;
            case BAD_CONTENT_PROVIDER_WRITE:
                errorMsg = context.getString(R.string.error_saf_bad_content_provider);
                errorMsg += context.getString(R.string.error_saf_bad_content_provider_write);
                break;
            case BAD_CONTENT_PROVIDER_DELETE:
                errorMsg = context.getString(R.string.error_saf_bad_content_provider);
                errorMsg += context.getString(R.string.error_saf_bad_content_provider_delete);
                break;
            case BAD_CONTENT_PROVIDER_FILENAME_IGNORED:
                errorMsg = context.getString(R.string.error_saf_bad_content_provider);
                errorMsg += context.getString(R.string.error_saf_bad_content_provider_filename_ignored);
                break;
            case BAD_CONTENT_PROVIDER_BASE_FOLDER_NOT_FOUND:
                errorMsg = context.getString(R.string.error_saf_bad_content_provider);
                errorMsg += context.getString(R.string.error_saf_bad_content_provider_base_folder_not_found);
                break;
            case BAD_CONTENT_PROVIDER_FILE_ACCESS:
                errorMsg = context.getString(R.string.error_saf_bad_content_provider);
                errorMsg += context.getString(R.string.error_saf_bad_content_provider_file_access);
                break;
            case FOLDER_NOT_ALMOST_EMPTY:
                errorMsg = context.getString(R.string.error_saf_folder_not_empty);
                break;
        }

        errorMsg += "\n\n" + context.getString(R.string.error_saf_select_easyrpg);
        builder.setMessage(errorMsg);

        builder.create();
        builder.show();
    }
}
