package org.easyrpg.player.player;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.DocumentsContract;
import android.util.Log;

import androidx.documentfile.provider.DocumentFile;

import org.easyrpg.player.Helper;

import java.io.IOException;
import java.util.ArrayList;

/**
 * A wrapper around SAF for use from JNI
 */
public class SafFile {
    private final Context context;
    private final Uri rootUri;

    private boolean metadataPopulated = false;
    private boolean metaIsFile = false;
    private boolean metaExists = false;
    private long metaFileSize = 0;

    private SafFile(Context context, Uri rootUri) {
        this.context = context;
        this.rootUri = rootUri;
    }

    public static SafFile fromPath(Context context, String path) {
        // Attempt to properly URL encode the path

        // Example:
        // The Player will pass in here as the game directory:
        // primary%3Aeasyrpg/document/primary%3Aeasyrpg%2Fgames%2FTestGame

        // When a file inside the game is accessed the path will be:
        // primary%3Aeasyrpg/document/primary%3Aeasyrpg%2Fgames%2FTestGame/Title/Title.png
        // The /Title/Title.png must be URL encoded, otherwise DocumentFile rejects it

        int encoded_slash_pos = path.lastIndexOf("%2F");
        if (encoded_slash_pos == -1) {
            // Should not happen because the game is in a subdirectory
            Log.e("EasyRPG", "Strange Uri " + path);
            return null;
        }
        int slash_pos = path.indexOf("/", encoded_slash_pos);

        // A file is provided when a / is after the encoded / (%2F)
        if (slash_pos > -1) {
            // Extract the filename and properly encode it
            String encoded = path.substring(0, slash_pos);
            String to_encode = path.substring(slash_pos);
            to_encode = Uri.encode(to_encode);
            path = encoded + to_encode;
        }

        if (!path.startsWith("content://")) {
            path = "content://" + path;
        }
        Uri uri = Uri.parse(path);

        return new SafFile(context, uri);
    }

    public boolean isFile() {
        populateMetadata();
        return metaExists && metaIsFile;
    }

    public boolean isDirectory() {
        populateMetadata();
        return metaExists && !metaIsFile;
    }

    public boolean exists() {
        populateMetadata();
        return metaExists;
    }

    public long getFilesize() {
        populateMetadata();
        return metaFileSize;
    }

    public int createInputFileDescriptor() {
        // No difference between read mode and binary read mode
        try (ParcelFileDescriptor fd = context.getContentResolver().openFileDescriptor(rootUri, "r")) {
            return fd.detachFd();
        } catch (IOException | IllegalArgumentException e) {
            return -1;
        }
    }

    public int createOutputFileDescriptor(boolean append) {
        Uri actualFile = rootUri;
        if (!exists()) {
            // The file must exist beforehand
            // To create it the parent directory must be obtained
            String full_path = rootUri.toString();
            String directory = full_path.substring(0, full_path.lastIndexOf("%2F"));
            String filename = full_path.substring(full_path.lastIndexOf("%2F") + 3);
            filename = Uri.decode(filename);
            DocumentFile df = DocumentFile.fromTreeUri(context, Uri.parse(directory));
            if (df == null || !df.exists()) {
                return -1;
            }
            df = df.createFile("application/octet-stream", filename);
            if (df == null || !df.exists()) {
                return -1;
            }
            // createFile can decide to not honor the filename, wtf?!
            actualFile = df.getUri();
        }

        if (append) {
            try (ParcelFileDescriptor fd = context.getContentResolver().openFileDescriptor(actualFile, "wa")) {
                return fd.detachFd();
            } catch (IOException e) {
                return -1;
            }
        } else {
            // Android 10+ does not truncate anymore when "w" is used (CVE-2023-21036)
            // See also https://issuetracker.google.com/issues/180526528
            try (ParcelFileDescriptor fd = context.getContentResolver().openFileDescriptor(actualFile, "wt")) {
                return fd.detachFd();
            } catch (IOException e) {
                // Some content providers do not support "wt" but will truncate when using "w"
                try (ParcelFileDescriptor fd = context.getContentResolver().openFileDescriptor(actualFile, "w")) {
                    return fd.detachFd();
                } catch (IOException e2) {
                    return -1;
                }
            }
        }
    }

    DirectoryTree getDirectoryContent() {
        ArrayList<String> files = new ArrayList<>();
        ArrayList<Boolean> is_dir = new ArrayList<>();

        final ContentResolver resolver = context.getContentResolver();
        String documentId = DocumentsContract.getTreeDocumentId(rootUri);
        if (DocumentsContract.isDocumentUri(context, rootUri)) {
            documentId = DocumentsContract.getDocumentId(rootUri);
        }

        final Uri childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(rootUri, documentId);

        Cursor c = resolver.query(childrenUri,new String[] {
                DocumentsContract.Document.COLUMN_DISPLAY_NAME,
                DocumentsContract.Document.COLUMN_MIME_TYPE },
                null, null, null);
        while (c.moveToNext()) {
            String file_path = c.getString(0);
            String mime_type = c.getString(1);

            files.add(file_path);
            is_dir.add(mime_type.equals(DocumentsContract.Document.MIME_TYPE_DIR));
        }
        c.close();

        return new DirectoryTree(files, is_dir);
    }

    private void populateMetadata() {
        if (metadataPopulated) {
            return;
        }

        metadataPopulated = true;

        final ContentResolver resolver = context.getContentResolver();

        Cursor c;

        try {
            c = resolver.query(rootUri, new String[]{
                    DocumentsContract.Document.COLUMN_MIME_TYPE,
                    DocumentsContract.Document.COLUMN_SIZE},
                null, null, null);
        } catch (IllegalArgumentException e) {
            metaExists = false;
            return;
        }

        if (c.moveToNext()) {
            metaExists = true;
            metaIsFile = !Helper.isDirectoryFromMimeType(c.getString(0));
            metaFileSize = c.getLong(1);
        }

        c.close();
    }
}
