package org.easyrpg.player.player;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.provider.DocumentsContract;
import android.util.Log;

import androidx.documentfile.provider.DocumentFile;

import java.io.IOException;
import java.util.ArrayList;

/**
 * A wrapper around SAF for use from JNI
 */
public class SafFile {
    private final Context context;
    private final DocumentFile root;

    private SafFile(Context context, DocumentFile root) {
        this.context = context;
        this.root = root;
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

        DocumentFile f = DocumentFile.fromTreeUri(context, uri);

        if (f == null || !f.getUri().toString().equals(uri.toString())) {
            // When providing a non-existent file fromTreeUri sometimes returns a tree to the root.
            // Prevent this nonsense.
            return null;
        }

        return new SafFile(context, f);
    }

    public boolean isFile() {
        return root != null && root.isFile();
    }

    public boolean isDirectory() {
        return root != null && root.isDirectory();
    }

    public boolean exists() {
        return root != null && root.exists();
    }

    public long getFilesize() {
        if (root == null || !root.exists()) {
            return -1L;
        }

        return root.length();
    }

    public int createInputFileDescriptor() {
        if (root == null) {
            return -1;
        }

        // No difference between read mode and binary read mode
        try (ParcelFileDescriptor fd = context.getContentResolver().openFileDescriptor(root.getUri(), "r")) {
            return fd.detachFd();
        } catch (IOException | IllegalArgumentException e) {
            return -1;
        }
    }

    public int createOutputFileDescriptor(boolean append) {
        if (root == null) {
            return -1;
        }

        Uri actualFile = root.getUri();
        if (!exists()) {
            // The file must exist beforehand
            // To create it the parent directory must be obtained
            String full_path = root.getUri().toString();
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
        if (root == null || !root.isDirectory()) {
            return null;
        }

        Uri root_uri = root.getUri();

        ArrayList<String> files = new ArrayList<>();
        ArrayList<Boolean> is_dir = new ArrayList<>();

        final ContentResolver resolver = context.getContentResolver();
        final Uri childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(root_uri, DocumentsContract.getDocumentId(root_uri));

        Cursor c = resolver.query(childrenUri,new String[] {
                DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                DocumentsContract.Document.COLUMN_MIME_TYPE },
                null, null, null);
        while (c.moveToNext()) {
            // The name can be something like ``primary:path/of/the/folder/the_file.txt
            // Get rid of all that junk
            String file_path = c.getString(0);
            int slash_pos = file_path.lastIndexOf('/');
            if (slash_pos != -1) {
                file_path = file_path.substring(slash_pos + 1);
            }

            String mime_type = c.getString(1);

            files.add(file_path);
            is_dir.add(mime_type.equals(DocumentsContract.Document.MIME_TYPE_DIR));
        }
        c.close();

        return new DirectoryTree(files, is_dir);
    }
}
