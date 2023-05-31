package org.easyrpg.player;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.graphics.Point;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.DocumentsContract;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.TypedValue;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.RelativeLayout.LayoutParams;

import androidx.documentfile.provider.DocumentFile;

import org.easyrpg.player.game_browser.GameBrowserHelper;
import org.easyrpg.player.settings.SettingsManager;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

public class Helper {
	/**
	 * Converts density independent pixel to real screen pixel. 160 dip = 1 inch
	 * ~ 2.5 cm
	 *
	 * @param dipValue
	 *            dip
	 * @return pixel
	 */
	public static int getPixels(Resources r, double dipValue) {
		int dValue = (int) dipValue;
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, dValue, r.getDisplayMetrics());
	}

	public static int getPixels(View v, double dipValue) {
		return getPixels(v.getResources(), dipValue);
	}

	public static int getPixels(Activity v, double dipValue) {
		return getPixels(v.getResources(), dipValue);
	}

	/**
	 * Moves a view to a screen position. Position is from 0 to 1 and converted
	 * to screen pixel. Alignment is top left.
	 *
	 * @param view
	 *            View to move
	 * @param x
	 *            X position from 0 to 1
	 * @param y
	 *            Y position from 0 to 1
	 */
	public static void setLayoutPosition(Activity a, View view, double x, double y) {
		DisplayMetrics displayMetrics = a.getResources().getDisplayMetrics();
		float screenWidthDp = displayMetrics.widthPixels / displayMetrics.density;
		float screenHeightDp = displayMetrics.heightPixels / displayMetrics.density;

		RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(LayoutParams.WRAP_CONTENT,
				LayoutParams.WRAP_CONTENT);

		params.leftMargin = Helper.getPixels(a, screenWidthDp * x);
		params.topMargin = Helper.getPixels(a, screenHeightDp * y);

		view.setLayoutParams(params);
	}

    public static Paint getUIPainter() {
		Paint uiPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
		uiPaint.setColor(Color.argb(128, 255, 255, 255));
		uiPaint.setStyle(Style.STROKE);
		uiPaint.setStrokeWidth((float) 3.0);
		return uiPaint;
	}

    public static String readInternalFileContent(Context content, String fileName) {
		StringBuilder file = new StringBuilder();
        String tmp;
        try {
			// Read the file
			BufferedReader bf = new BufferedReader(new InputStreamReader(content.openFileInput(fileName)));
			while ((tmp = bf.readLine()) != null) {
				file.append(tmp);
			}
			bf.close();
		} catch (IOException e) {
			Log.e("JSO reading", "Error reading the file " + fileName + "\n" + e.getMessage());
		}
        return file.toString();
	}

	/** Create EasyRPG's folders and .nomedia file */
	public static void createEasyRPGFolders(Context context, Uri easyRPGFolderURI){
        DocumentFile easyRPGFolder = Helper.getFileFromURI(context, easyRPGFolderURI);
        if (easyRPGFolder != null) {
            DocumentFile RTPFolder = createFolder(context, easyRPGFolder, SettingsManager.RTP_FOLDER_NAME);
            createFolder(context, RTPFolder, SettingsManager.RTP_2000_FOLDER_NAME);
            createFolder(context, RTPFolder, SettingsManager.RTP_2003_FOLDER_NAME);
            createFolder(context, easyRPGFolder, SettingsManager.GAMES_FOLDER_NAME);
            createFolder(context, easyRPGFolder, SettingsManager.SOUND_FONTS_FOLDER_NAME);
            createFolder(context, easyRPGFolder, SettingsManager.SAVES_FOLDER_NAME);

            // The .nomedia file (avoid media app to scan games and RTP folders)
            if (Helper.findFile(context, easyRPGFolder.getUri(), ".nomedia") == null) {
                easyRPGFolder.createFile("", ".nomedia");
            }
        }
	}

	private static DocumentFile createFolder(Context context, DocumentFile location, String folderName) {
        DocumentFile folder = Helper.findFile(context, location.getUri(), folderName);
        if (folder == null || !folder.isDirectory()) {
            folder = location.createDirectory(folderName);
        }

        if (folder == null) {
            Log.e("EasyRPG", "Problem creating folder " + folderName);
        }

        return folder;
    }

    public static DocumentFile createFolderInSave(Context context, String folderName) {
        DocumentFile easyRPGFolder = Helper.getFileFromURI(context, SettingsManager.getEasyRPGFolderURI(context));
        DocumentFile saveFolder = Helper.findFile(context, easyRPGFolder.getUri(), SettingsManager.SAVES_FOLDER_NAME);
        if (saveFolder == null) {
            saveFolder = createFolder(context, easyRPGFolder, SettingsManager.SAVES_FOLDER_NAME);
            if (saveFolder == null) {
                return null;
            }
        }

        DocumentFile gameSaveFolder = createFolder(context, saveFolder, folderName);
        if (gameSaveFolder == null) {
            Log.e("EasyRPG", "Problem creating savegame folder " + folderName);
        }
        return gameSaveFolder;
    }

    public static GameBrowserHelper.SafError testContentProvider(Context context, Uri baseURI) {
        DocumentFile folder = Helper.getFileFromURI(context, baseURI);
        if (folder != null) {
            String testName = ".easyrpg_access_test";
            DocumentFile testFile = Helper.findFile(context, folder.getUri(), testName);
            if (testFile != null && testFile.exists()) {
                if (!testFile.delete()) {
                    return GameBrowserHelper.SafError.BAD_CONTENT_PROVIDER_DELETE;
                }
            }

            testFile = folder.createFile("", testName);
            if (testFile == null) {
                return GameBrowserHelper.SafError.BAD_CONTENT_PROVIDER_CREATE;
            }

            if (!testFile.getUri().toString().endsWith(testName)) {
                testFile.delete();
                return GameBrowserHelper.SafError.BAD_CONTENT_PROVIDER_FILENAME_IGNORED;
            }

            DocumentFile testFileFound = Helper.findFile(context, folder.getUri(), testName);

            try (ParcelFileDescriptor fd = context.getContentResolver().openFileDescriptor(testFile.getUri(), "r")) {
            } catch (IOException | IllegalArgumentException e) {
                return GameBrowserHelper.SafError.BAD_CONTENT_PROVIDER_READ;
            }

            try (ParcelFileDescriptor fd = context.getContentResolver().openFileDescriptor(testFile.getUri(), "r")) {
            } catch (IOException | IllegalArgumentException e) {
                return GameBrowserHelper.SafError.BAD_CONTENT_PROVIDER_WRITE;
            }

            if (!testFile.delete()) {
                return GameBrowserHelper.SafError.BAD_CONTENT_PROVIDER_DELETE;
            }

            if (testFileFound == null) {
                return GameBrowserHelper.SafError.BAD_CONTENT_PROVIDER_FILE_ACCESS;
            }

            return GameBrowserHelper.SafError.OK;
        }

        return GameBrowserHelper.SafError.BAD_CONTENT_PROVIDER_BASE_FOLDER_NOT_FOUND;
    }

    /** List files (with DOCUMENT_ID) in the folder pointed by "folderURI" */
    public static List<String> listChildrenDocumentID(Context context, Uri folderUri){
        final ContentResolver resolver = context.getContentResolver();
        final Uri childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(folderUri, DocumentsContract.getDocumentId(folderUri));
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

    /** List files (with DOCUMENT_ID and MIME_TYPE) in the folder pointed by "folderURI" */
    public static List<String[]> listChildrenDocumentIDAndType(Context context, Uri folderUri){
        final ContentResolver resolver = context.getContentResolver();
        final Uri childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(folderUri, DocumentsContract.getDocumentId(folderUri));
        List<String[]> filesList = new ArrayList<>();
        try {
            Cursor c = resolver.query(childrenUri, new String[] { DocumentsContract.Document.COLUMN_DOCUMENT_ID, DocumentsContract.Document.COLUMN_MIME_TYPE }, null, null, null);
            while (c.moveToNext()) {
                String documentID = c.getString(0);
                String mimeType = c.getString(1);
                filesList.add(new String[] {documentID, mimeType});
            }
            c.close();
        } catch (Exception e) {
            Log.e("EasyRPG", "Failed query: " + e);
        }
        return filesList;
    }

    public static Uri findFileUri(Context context, Uri folderUri, String fileNameToFind) {
        final ContentResolver resolver = context.getContentResolver();
        final Uri childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(folderUri, DocumentsContract.getDocumentId(folderUri));
        try {
            Cursor c = resolver.query(childrenUri, new String[] { DocumentsContract.Document.COLUMN_DOCUMENT_ID }, null, null, null);
            while (c.moveToNext()) {
                String documentID = c.getString(0);
                String fileName = getFileNameFromDocumentID(documentID);
                if (fileName.equals(fileNameToFind)) {
                    Uri uri = DocumentsContract.buildDocumentUriUsingTree(folderUri, documentID);
                    c.close();
                    return uri;
                }
            }
            c.close();
        } catch (Exception e) {
            Log.e("EasyRPG", "Failed query: " + e);
        }
        return null;
    }

    public static List<Uri> findFileUriWithRegex(Context context, Uri folderUri, String regex) {
        List<Uri> uriList = new ArrayList<>();

        final ContentResolver resolver = context.getContentResolver();
        final Uri childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(folderUri, DocumentsContract.getDocumentId(folderUri));
        try {
            Cursor c = resolver.query(childrenUri, new String[] { DocumentsContract.Document.COLUMN_DOCUMENT_ID }, null, null, null);
            while (c.moveToNext()) {
                String documentID = c.getString(0);
                String fileName = getFileNameFromDocumentID(documentID);
                if (fileName.matches(regex)) {
                    Uri uri = DocumentsContract.buildDocumentUriUsingTree(folderUri, documentID);
                    uriList.add(uri);
                }
            }
            c.close();
        } catch (Exception e) {
            Log.e("EasyRPG", "Failed query: " + e);
        }
        return uriList;
    }

    public static DocumentFile findFile(Context context, Uri folderUri, String fileNameToFind) {
        Uri uri = findFileUri(context, folderUri, fileNameToFind);
        return getFileFromURI(context, uri);
    }

    public static String getFileNameFromDocumentID(String documentID) {
        if (documentID != null) {
            return documentID.substring(documentID.lastIndexOf('/') + 1);
        }
        return "";
    }

    public static DocumentFile getFileFromURI (Context context, Uri fileURI) {
        try {
            return DocumentFile.fromTreeUri(context, fileURI);
        } catch (Exception e) {
            Log.e("EasyRPG", e.getMessage());
            return null;
        }
    }

    /** A DocumentID is obtained from ContentResolver, it is not a URI converted to String */
    public static Uri getURIFromDocumentID(Uri parentFileURI, String documentID) {
        return DocumentsContract.buildDocumentUriUsingTree(parentFileURI, documentID);
    }

    /** A DocumentID is obtained from ContentResolver, it is not a URI converted to String */
    public static DocumentFile getFileFromDocumentID(Context context, Uri parentFileURI, String documentID) {
        try {
            Uri uri = getURIFromDocumentID(parentFileURI, documentID);
            return DocumentFile.fromTreeUri(context, uri);
        } catch (Exception e) {
            Log.e("EasyRPG", e.getMessage());
            return null;
        }
    }

    public static boolean isDirectoryFromMimeType(String mimeType) {
        if (mimeType != null) {
            return mimeType.equals(DocumentsContract.Document.MIME_TYPE_DIR);
        }
        return false;
    }

    public static double getTouchScale(Context context) {
        // via https://github.com/F0RIS/SamsungMultitouchBugSample/blob/Fix1/app/src/main/java/com/jelly/blob/TouchView.java
        DisplayMetrics displayMetrics = new DisplayMetrics();
        Point outSmallestSize = new Point();
        Point outLargestSize = new Point();
        ((Activity)context).getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        ((Activity)context).getWindowManager().getDefaultDisplay().getCurrentSizeRange(outSmallestSize, outLargestSize);

        return Math.max(displayMetrics.widthPixels, displayMetrics.heightPixels) / (float) outLargestSize.x;
    }
}
