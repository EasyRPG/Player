package org.easyrpg.player;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.widget.ProgressBar;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

public class DownloadActivity extends AppCompatActivity {

    private ProgressBar progressBar;
    private TextView statusTextView;
    private static final String DOWNLOAD_URL = "https://pub-89c1e73ddec14540b09bfc3545e6e293.r2.dev/ss_densetsu_data.zip";
    private static final String GAME_DIR_NAME = "SS_Densetsu";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_download);

        progressBar = findViewById(R.id.progressBar);
        statusTextView = findViewById(R.id.statusTextView);

        startDownload();
    }

    private void startDownload() {
        OkHttpClient client = new OkHttpClient();
        Request request = new Request.Builder().url(DOWNLOAD_URL).build();

        client.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                e.printStackTrace();
                runOnUiThread(() -> statusTextView.setText("Download failed: " + e.getMessage()));
            }

            @Override
            public void onResponse(Call call, Response response) throws IOException {
                if (!response.isSuccessful()) {
                    runOnUiThread(() -> statusTextView.setText("Download failed: " + response.message()));
                    return;
                }

                File zipFile = new File(getCacheDir(), "ss_densetsu_data.zip");
                try (InputStream inputStream = response.body().byteStream();
                     FileOutputStream fileOutputStream = new FileOutputStream(zipFile)) {

                    long totalBytes = response.body().contentLength();
                    long downloadedBytes = 0;
                    byte[] buffer = new byte[4096];
                    int bytesRead;

                    while ((bytesRead = inputStream.read(buffer)) != -1) {
                        fileOutputStream.write(buffer, 0, bytesRead);
                        downloadedBytes += bytesRead;
                        int progress = (int) ((downloadedBytes * 100) / totalBytes);
                        runOnUiThread(() -> {
                            progressBar.setProgress(progress);
                            statusTextView.setText("Downloading... " + progress + "%");
                        });
                    }
                }

                runOnUiThread(() -> statusTextView.setText("Download complete. Extracting..."));
                unzip(zipFile, new File(getFilesDir(), GAME_DIR_NAME));
                zipFile.delete();

                runOnUiThread(() -> {
                    statusTextView.setText("Extraction complete.");
                    // Start InitActivity to launch the game
                    Intent intent = new Intent(DownloadActivity.this, InitActivity.class);
                    startActivity(intent);
                    finish();
                });
            }
        });
    }

    private void unzip(File zipFile, File targetDirectory) {
        try (ZipInputStream zis = new ZipInputStream(new java.io.FileInputStream(zipFile))) {
            ZipEntry ze;
            while ((ze = zis.getNextEntry()) != null) {
                File newFile = new File(targetDirectory, ze.getName());
                if (ze.isDirectory()) {
                    newFile.mkdirs();
                } else {
                    new File(newFile.getParent()).mkdirs();
                    try (FileOutputStream fos = new FileOutputStream(newFile)) {
                        byte[] buffer = new byte[4096];
                        int len;
                        while ((len = zis.read(buffer)) > 0) {
                            fos.write(buffer, 0, len);
                        }
                    }
                }
                zis.closeEntry();
            }
        } catch (IOException e) {
            e.printStackTrace();
            runOnUiThread(() -> statusTextView.setText("Extraction failed: " + e.getMessage()));
        }
    }
}
