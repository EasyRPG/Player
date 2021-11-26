package org.easyrpg.player;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.net.Uri;
import android.provider.DocumentsContract;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.TypedValue;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.RelativeLayout.LayoutParams;
import android.widget.Toast;

import androidx.documentfile.provider.DocumentFile;

import org.easyrpg.player.settings.SettingsManager;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

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
		int px = (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, dValue, r.getDisplayMetrics());
		return px;
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

	/**
	 * Moves a view to a screen position. Position is from 0 to 1 and converted
	 * to screen pixel. Alignment is top right.
	 *
	 * @param view
	 *            View to move
	 * @param x
	 *            X position from 0 to 1
	 * @param y
	 *            Y position from 0 to 1
	 */
	public static void setLayoutPositionRight(Activity a, View view, double x, double y) {
		DisplayMetrics displayMetrics = a.getResources().getDisplayMetrics();
		float screenWidthDp = displayMetrics.widthPixels / displayMetrics.density;
		float screenHeightDp = displayMetrics.heightPixels / displayMetrics.density;

		RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(LayoutParams.WRAP_CONTENT,
				LayoutParams.WRAP_CONTENT);
		params.addRule(RelativeLayout.ALIGN_PARENT_RIGHT, 1);
		params.rightMargin = Helper.getPixels(a, screenWidthDp * x);
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

	public static void showWrongAPIVersion(Context context) {
		Toast.makeText(context, "Not avaible on this API", Toast.LENGTH_SHORT).show();
	}

	public static JSONObject readJSON(String contentFile) {
		try {
			// Parse the JSON
			JSONObject jso = new JSONObject(contentFile);
			return jso;
		} catch (JSONException e) {
			Log.e("JSO reading", "Error parsing a JSO file : " + e.getMessage());
		}

		return null;
	}

	public static JSONObject readJSONFile(String path) {
		String file = new String(), tmp;
		try {
			// Read the file
			BufferedReader bf = new BufferedReader(new FileReader(new File(path)));
			while ((tmp = bf.readLine()) != null) {
				file += tmp;
			}
			bf.close();

			// Parse the JSON
			JSONObject jso = new JSONObject(file);
			return jso;
		} catch (JSONException e) {
			Log.e("JSO reading", "Error parsing the JSO file " + path + "\n" + e.getMessage());
		} catch (IOException e) {
			Log.e("JSO reading", "Error reading the file " + path + "\n" + e.getMessage());
		}

		return null;
	}

	public static String readInternalFileContent(Context content, String fileName) {
		String file = new String(), tmp;
		try {
			// Read the file
			BufferedReader bf = new BufferedReader(new InputStreamReader(content.openFileInput(fileName)));
			while ((tmp = bf.readLine()) != null) {
				file += tmp;
			}
			bf.close();
		} catch (IOException e) {
			Log.e("JSO reading", "Error reading the file " + fileName + "\n" + e.getMessage());
		}
		return file;
	}

	/** Create RTP folders and .nomedia file in the games folder */
	public static void createEasyRPGDirectories(Context context, DocumentFile gamesFolder){
		// RTP folder
        DocumentFile RTPFolder = createFolder(context, gamesFolder, "RTP");
        createFolder(context, RTPFolder, "2000");
        createFolder(context, RTPFolder, "2003");

        // Save the RTP folder in Settings
        SettingsManager.setRtpFolder(RTPFolder);

        // The .nomedia file (avoid media app to scan games and RTP's folders)
        if (Helper.findFile(context, gamesFolder.getUri(), ".nomedia") == null) {
            gamesFolder.createFile("", ".nomedia");
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
        return DocumentFile.fromTreeUri(context, fileURI);
    }

    /** A DocumentID is obtained from ContentResolver, it is not a URI converted to String */
    public static Uri getURIFromDocumentID(Uri parentFileURI, String documentID) {
        return DocumentsContract.buildDocumentUriUsingTree(parentFileURI, documentID);
    }

    /** A DocumentID is obtained from ContentResolver, it is not a URI converted to String */
    public static DocumentFile getFileFromDocumentID(Context context, Uri parentFileURI, String documentID) {
        Uri uri = getURIFromDocumentID(parentFileURI, documentID);
        return DocumentFile.fromTreeUri(context, uri);
    }

    public static boolean isDirectoryFromMimeType(String mimeType) {
        if (mimeType != null) {
            return mimeType.equals(DocumentsContract.Document.MIME_TYPE_DIR);
        }
        return false;
    }
}
